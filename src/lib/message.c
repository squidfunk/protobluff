/*
 * Copyright (c) 2013-2015 Martin Donath <martin.donath@squidfunk.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <alloca.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lib/binary.h"
#include "lib/common.h"
#include "lib/cursor.h"
#include "lib/field.h"
#include "lib/field/descriptor.h"
#include "lib/message.h"
#include "lib/message/descriptor.h"
#include "lib/part.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a message from a descriptor and a binary.
 *
 * \param[in]     descriptor Message descriptor
 * \param[in,out] binary     Binary
 * \return                   Message
 */
extern pb_message_t
pb_message_create(
    const pb_message_descriptor_t *descriptor,
    pb_binary_t *binary) {
  assert(descriptor && binary);
  pb_message_t message = {
    .descriptor = descriptor,
    .part       = pb_part_create_from_binary(binary)
  };
  return message;
}

/*!
 * Create a submessage within a message for a specific tag.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Submessage
 */
extern pb_message_t
pb_message_create_within(pb_message_t *message, pb_tag_t tag) {
  assert(message && tag);
  if (pb_message_valid(message)) {
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_field_by_tag(message->descriptor, tag);
    assert(descriptor &&
      pb_field_descriptor_type(descriptor) == PB_TYPE_MESSAGE &&
      pb_field_descriptor_reference(descriptor));
    pb_message_t submessage = {
      .descriptor = pb_field_descriptor_reference(descriptor),
      .part       = pb_part_create(message, tag)
    };
    return submessage;
  }
  return pb_message_create_invalid();
}

/*!
 * Create a submessage within a nested message for a branch of tags.
 *
 * \warning All messages on the branch of tags except the leaf may not occur
 * repeatedly, but must be defined as optional or required.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \return                Submessage
 */
extern pb_message_t
pb_message_create_nested(
    pb_message_t *message, const pb_tag_t tags[], size_t size) {
  assert(message && tags && size);
  pb_message_t prev = pb_message_create_invalid(),
               this = pb_message_copy(message);
  for (size_t t = 0; t < size && pb_message_valid(&this); ++t) {
    pb_message_destroy(&prev);

#ifndef NDEBUG

    /* Assert non-repeated non-leaf messages */
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_field_by_tag(this.descriptor, tags[t]);
    assert(descriptor && (
      pb_field_descriptor_type(descriptor)  == PB_TYPE_MESSAGE && (
      pb_field_descriptor_label(descriptor) != PB_LABEL_REPEATED ||
        t == size - 1)));

#endif /* NDEBUG */

    /* Swap messages and create a new one within */
    prev = this; this = pb_message_create_within(&prev, tags[t]);
  }
  pb_message_destroy(&prev);
  return this;
}

/*!
 * Create a message at the current position of a cursor.
 *
 * \param[in,out] cursor Cursor
 * \return               Message
 */
extern pb_message_t
pb_message_create_from_cursor(pb_cursor_t *cursor) {
  assert(cursor);
  if (pb_cursor_valid(cursor)) {
    const pb_message_t *message = pb_cursor_message(cursor);
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_field_by_tag(message->descriptor,
        pb_cursor_tag(cursor));
    if (descriptor && pb_field_descriptor_type(descriptor) == PB_TYPE_MESSAGE) {
      assert(pb_field_descriptor_reference(descriptor));
      pb_message_t submessage = {
        .descriptor = pb_field_descriptor_reference(descriptor),
        .part       = pb_part_create_from_cursor(cursor)
      };
      return submessage;
    }
  }
  return pb_message_create_invalid();
}

/*!
 * Create a message from a descriptor and a bytes field.
 *
 * \param[in]     descriptor Message descriptor
 * \param[in,out] field      Bytes field
 * \return                   Message
 */
extern pb_message_t
pb_message_create_from_field(
    const pb_message_descriptor_t *descriptor,
    pb_field_t *field) {
  assert(descriptor && field);
  if (pb_field_valid(field) && !pb_field_align(field)) {
    assert(pb_field_descriptor_type(
      pb_field_descriptor(field)) == PB_TYPE_BYTES);
    pb_message_t message = {
      .descriptor = descriptor,
      .part       = pb_part_copy(pb_field_part(field))
    };
    return message;
  }
  return pb_message_create_invalid();
}

/*!
 * Destroy a message.
 *
 * \param[in,out] message Message
 */
extern void
pb_message_destroy(pb_message_t *message) {
  assert(message);
  pb_part_destroy(&(message->part));
}

/*!
 * Check a message.
 *
 * Fields do not necessarily occur in ascending order, so they are checked in
 * order of their appearance. Repeated fields may occur multiple times, required
 * and optional fields can occur only once. Fields unknown by the descriptor are
 * skipped, which is necessary for backward compatibility after updates.
 *
 * \param[in] message Message
 * \return            Error code
 */
extern pb_error_t
pb_message_check(const pb_message_t *message) {
  assert(message);
  if (unlikely_(!pb_message_valid(message)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Copy message and initialize temporary struct to count occurrences */
  pb_message_t copy = pb_message_copy(message);
  struct {
    pb_tag_t *data;
    size_t size;
  } field = {
    .data = alloca(sizeof(pb_tag_t)
          * pb_message_descriptor_size(copy.descriptor)),
    .size = 0
  };

  /* Walk through message and record field occurrences */
  pb_cursor_t cursor = pb_cursor_create_without_tag(&copy);
  if (pb_cursor_valid(&cursor)) {
    do {
      pb_tag_t tag = pb_cursor_tag(&cursor);
      const pb_field_descriptor_t *descriptor =
        pb_message_descriptor_field_by_tag(message->descriptor, tag);
      if (!descriptor)
        continue;

      /* Check for multiple occurrences of non-repeated fields */
      uint8_t miss = 1;
      for (size_t f = 0; miss && f < field.size; ++f)
        if (field.data[f] == tag)
          miss = 0;
      if (miss)
        field.data[field.size++] = tag;
      else if (pb_field_descriptor_label(descriptor) != PB_LABEL_REPEATED)
        error = PB_ERROR_INVALID;

      /* Recurse, if submessage */
      if (pb_field_descriptor_type(descriptor) == PB_TYPE_MESSAGE) {
        pb_message_t submessage = pb_message_create_from_cursor(&cursor);
        error = pb_message_check(&submessage);
        pb_message_destroy(&submessage);
      }
    } while (!error && pb_cursor_next(&cursor));
  }

  /* Don't indicate an error, if the cursor just reached the end */
  if (!error && (error = pb_cursor_error(&cursor)) == PB_ERROR_OFFSET)
    error = PB_ERROR_NONE;
  pb_cursor_destroy(&cursor);

  /* Check for absent required fields */
  if (!error) {
    pb_message_descriptor_iter_t it =
      pb_message_descriptor_iter_create(copy.descriptor);
    if (pb_message_descriptor_iter_begin(&it)) {
      do {
        const pb_field_descriptor_t *descriptor =
          pb_message_descriptor_iter_current(&it);
        if (pb_field_descriptor_label(descriptor) == PB_LABEL_REQUIRED) {
          uint8_t miss = 1;
          for (size_t f = 0; miss && f < field.size; ++f)
            if (field.data[f] == pb_field_descriptor_tag(descriptor))
              miss = 0;
          if (miss)
            error = PB_ERROR_ABSENT;
        }
      } while (!error && pb_message_descriptor_iter_next(&it));
    }
    pb_message_descriptor_iter_destroy(&it);
  }
  pb_message_destroy(&copy);
  return error;
}

/*!
 * Test whether a message contains at least one occurrence for a given tag.
 *
 * Whether the message is valid or not is checked by the cursor, so there is
 * no need to perform this check before creating the cursor.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Test result
 */
extern int
pb_message_has(pb_message_t *message, pb_tag_t tag) {
  assert(message && tag);
  pb_cursor_t cursor = pb_cursor_create_internal(message, tag);
  int result = pb_cursor_valid(&cursor);
  pb_cursor_destroy(&cursor);
  return result;
}

/*!
 * Compare the value for a given tag from a message with the given value.
 *
 * Whether the message is valid or not is checked by the cursor, so there is
 * no need to perform this check before creating the cursor.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \param[in]     value   Pointer holding value
 * \return                Test result
 */
extern int
pb_message_match(pb_message_t *message, pb_tag_t tag, const void *value) {
  assert(message && tag && value);

#ifndef NDEBUG

  /* Check is only necessary in debug mode */
  if (unlikely_(!pb_message_valid(message)))
    return 0;

  /* Assert non-message field */
  const pb_field_descriptor_t *descriptor =
    pb_message_descriptor_field_by_tag(message->descriptor, tag);
  assert(descriptor &&
    pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE);

#endif /* NDEBUG */

  /* Use cursor to seek for matching value */
  pb_cursor_t cursor = pb_cursor_create_internal(message, tag);
  int result = pb_cursor_match(&cursor, value) ||
               pb_cursor_seek(&cursor, value);
  pb_cursor_destroy(&cursor);
  return result;
}

/*!
 * Read the value for a given tag from a message, or return its default.
 *
 * For reasons of concistency, repeated fields must be read using a cursor.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \param[out]    value   Pointer receiving value
 * \return                Error code
 */
extern pb_error_t
pb_message_get(pb_message_t *message, pb_tag_t tag, void *value) {
  assert(message && tag && value);
  if (unlikely_(!pb_message_valid(message)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Assert non-repeated non-message field */
  const pb_field_descriptor_t *descriptor =
    pb_message_descriptor_field_by_tag(message->descriptor, tag);
  assert(descriptor &&
    pb_field_descriptor_type(descriptor)  != PB_TYPE_MESSAGE &&
    pb_field_descriptor_label(descriptor) != PB_LABEL_REPEATED);

  /* Use cursor to omit field creation */
  pb_cursor_t cursor = pb_cursor_create_internal(message, tag);
  if (pb_cursor_valid(&cursor)) {
    error = pb_cursor_get(&cursor, value);
  } else if ((error = pb_cursor_error(&cursor)) == PB_ERROR_OFFSET) {

    /* Extract default value, if present */
    if (unlikely_(!pb_field_descriptor_default(descriptor))) {
      error = PB_ERROR_ABSENT;
    } else {
      memcpy(value, pb_field_descriptor_default(descriptor),
        pb_field_descriptor_type_size(descriptor));
      error = PB_ERROR_NONE;
    }
  }
  pb_cursor_destroy(&cursor);
  return error;
}

/*!
 * Write a value or submessage for a given tag to a message.
 *
 * Writing a repeated field or submessage will always create a new one.
 *
 * \warning In order to avoid any side-effects when writing a submessage to a
 * message, both messages are not allowed to share a common binary. Otherwise
 * the submessage may contain empty messages initialized by the other message
 * before writing the raw message data.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \param[in]     value   Pointer holding value
 * \return                Error code
 */
extern pb_error_t
pb_message_put(pb_message_t *message, pb_tag_t tag, const void *value) {
  assert(message && tag && value);
  if (unlikely_(!pb_message_valid(message)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Assert descriptor */
  const pb_field_descriptor_t *descriptor =
    pb_message_descriptor_field_by_tag(message->descriptor, tag);
  assert(descriptor);

  /* Create field and write value */
  if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
    pb_field_t field = pb_field_create_without_default(message, tag);
    error = pb_field_put(&field, value);
    pb_field_destroy(&field);

  /* Write submessage to message */
  } else {
    pb_message_t submessage = pb_message_copy(value);
    assert(pb_message_binary(message) != pb_message_binary(&submessage));
    if (unlikely_(!pb_message_valid(&submessage) ||
                    pb_message_align(&submessage))) {
      error = PB_ERROR_INVALID;

    /* Write raw binary data */
    } else {
      pb_part_t part = pb_part_create(message, tag);
      error = pb_part_write(&part, pb_binary_data_from(
        pb_message_binary(&submessage), pb_message_start(&submessage)),
          pb_message_size(&submessage));
      pb_part_destroy(&part);
    }
    pb_message_destroy(&submessage);
  }
  return error;
}

/*!
 * Erase a field or submessage for a given tag from a message.
 *
 * Erasing a repeated field or submessage will always erase all occurrences.
 *
 * The cursor may either be invalid or at an invalid offset after trying to
 * erase all instances of a field or submessage inside a message. These errors
 * are intercepted and the function returns with success. However, if any other
 * error is encountered by the cursor, it is returned.
 *
 * \warning Erasing a field or submessage from a message is an idempotent
 * operation, regardless of whether the field existed or not.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Error code
 */
extern pb_error_t
pb_message_erase(pb_message_t *message, pb_tag_t tag) {
  assert(message && tag);
  if (unlikely_(!pb_message_valid(message)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Use cursor to omit field/message creation */
  pb_cursor_t cursor = pb_cursor_create_internal(message, tag);
  if (pb_cursor_valid(&cursor))
    do {
      if ((error = pb_cursor_erase(&cursor)))
        break;                                             /* LCOV_EXCL_LINE */
    } while (pb_cursor_next(&cursor));
  if (!error && (error = pb_cursor_error(&cursor)))
    if (error == PB_ERROR_INVALID || error == PB_ERROR_OFFSET)
      error = PB_ERROR_NONE;
  pb_cursor_destroy(&cursor);
  return error;
}

/*!
 * Clear a message entirely.
 *
 * \warning Clearing a message will also invalidate it, as the underlying part
 * cannot restore the associated header (tag and length prefix).
 *
 * \param[in,out] message Message
 * \return                Error code
 */
extern pb_error_t
pb_message_clear(pb_message_t *message) {
  assert(message);
  return pb_part_clear(&(message->part));
}

/*!
 * Retrieve a pointer to the raw data for a given tag from a message.
 *
 * For reasons of concistency, repeated fields must be accessed using a cursor.
 *
 * Whether the message is valid or not is checked by the cursor, so there is
 * no need to perform this check before creating the cursor.
 *
 * \warning Operating on the raw data of a binary is insanely dangerous and
 * only recommended for 32- and 64-bit fixed-size fields on zero-copy binaries.
 * If the underlying binary is altered, the pointer silently loses its
 * validity. All following operations will quietly corrupt the binary.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Raw data
 */
extern void *
pb_message_raw(pb_message_t *message, pb_tag_t tag) {
  assert(message && tag);
  void *value = NULL;

#ifndef NDEBUG

  /* Check is only necessary in debug mode */
  if (unlikely_(!pb_message_valid(message)))
    return NULL;

  /* Assert non-repeated non-message field */
  const pb_field_descriptor_t *descriptor =
    pb_message_descriptor_field_by_tag(message->descriptor, tag);
  assert(descriptor &&
    pb_field_descriptor_type(descriptor)  != PB_TYPE_MESSAGE &&
    pb_field_descriptor_label(descriptor) != PB_LABEL_REPEATED);

#endif /* NDEBUG */

  /* Use cursor to omit field creation */
  pb_cursor_t cursor = pb_cursor_create_internal(message, tag);
  if (pb_cursor_valid(&cursor))
    value = pb_cursor_raw(&cursor);
  pb_cursor_destroy(&cursor);
  return value;
}

/* LCOV_EXCL_START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*!
 * Dump a message.
 *
 * Though this is just a wrapper for pb_part_dump(), it is implemented as a
 * proper function so it can be easily called from a debugger.
 *
 * \warning Don't use this in production, only for debugging.
 *
 * \param[in] message Message
 */
extern void
pb_message_dump(const pb_message_t *message) {
  assert(message);
  pb_part_dump(&(message->part));
}

/* LCOV_EXCL_STOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
/*
 * Copyright (c) 2013-2020 Martin Donath <martin.donath@squidfunk.com>
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

#include "core/descriptor.h"
#include "core/stream.h"
#include "core/varint.h"
#include "message/common.h"
#include "message/cursor.h"
#include "message/field.h"
#include "message/journal.h"
#include "message/message.h"
#include "message/part.h"

/* ----------------------------------------------------------------------------
 * Defaults
 * ------------------------------------------------------------------------- */

/* 8-bit default value */
static const uint8_t
default_8bit = 0;

/* 32-bit default value */
static const uint32_t
default_32bit = 0;

/* 64-bit default value */
static const uint64_t
default_64bit = 0;

/* ----------------------------------------------------------------------------
 * Mappings
 * ------------------------------------------------------------------------- */

/* Mapping: type ==> default value */
static const void *
default_map[] = {
  [PB_TYPE_INT32]    = &default_32bit,
  [PB_TYPE_INT64]    = &default_64bit,
  [PB_TYPE_UINT32]   = &default_32bit,
  [PB_TYPE_UINT64]   = &default_64bit,
  [PB_TYPE_SINT32]   = &default_32bit,
  [PB_TYPE_SINT64]   = &default_64bit,
  [PB_TYPE_FIXED32]  = &default_32bit,
  [PB_TYPE_FIXED64]  = &default_64bit,
  [PB_TYPE_SFIXED32] = &default_32bit,
  [PB_TYPE_SFIXED64] = &default_64bit,
  [PB_TYPE_BOOL]     = &default_8bit,
  [PB_TYPE_ENUM]     = &default_32bit,
  [PB_TYPE_FLOAT]    = &default_32bit,
  [PB_TYPE_DOUBLE]   = &default_64bit,
  [PB_TYPE_STRING]   = NULL,
  [PB_TYPE_BYTES]    = NULL,
  [PB_TYPE_MESSAGE]  = NULL
};

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a field within a message for a specific tag.
 *
 * \warning The lines excluded from code coverage cannot be triggered within
 * the tests, as they are masked through pb_field_create_without_default().
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Field
 */
extern pb_field_t
pb_field_create(pb_message_t *message, pb_tag_t tag) {
  pb_field_t field = pb_field_create_without_default(message, tag);
  if (pb_field_valid(&field) && pb_field_empty(&field)) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(pb_message_descriptor(message), tag);

    /* Write explicit or implicit default value to field */
    const void *value = pb_field_descriptor_default(descriptor)
      ? pb_field_descriptor_default(descriptor)
      : default_map[pb_field_descriptor_type(descriptor)];
    if (value && pb_field_put(&field, value)) {
      pb_field_destroy(&field);                            /* LCOV_EXCL_LINE */
      return pb_field_create_invalid();                    /* LCOV_EXCL_LINE */
    }
  }
  return field;
};

/*!
 * Create a field within a message for a specific tag without setting defaults.
 *
 * \warning This function returns a field in an inconsistent state, with a tag
 * set but no value, in a way which is not recognized by the parser. Immediate
 * writes should always be performed through pb_message_put(), which calls
 * this function internally.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Field
 */
extern pb_field_t
pb_field_create_without_default(pb_message_t *message, pb_tag_t tag) {
  assert(message && tag);
  if (pb_message_valid(message)) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(pb_message_descriptor(message), tag);
    assert(descriptor &&
      pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE);
    pb_field_t field = {
      .descriptor = descriptor,
      .part       = pb_part_create(message, tag)
    };
    return field;
  }
  return pb_field_create_invalid();
};

/*!
 * Create a field within a nested message for a branch of tags.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \return                Field
 */
extern pb_field_t
pb_field_create_nested(
    pb_message_t *message, const pb_tag_t tags[], size_t size) {
  assert(message && tags && size > 1);
  pb_message_t submessage =
    pb_message_create_nested(message, tags, --size);
  pb_field_t field = pb_field_create(&submessage, tags[size]);
  pb_message_destroy(&submessage);
  return field;
}

/*!
 * Create a field at the current position of a cursor.
 *
 * \param[in,out] cursor Cursor
 * \return               Field
 */
extern pb_field_t
pb_field_create_from_cursor(pb_cursor_t *cursor) {
  assert(cursor);
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor = pb_cursor_descriptor(cursor);
    if (descriptor &&
        pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
      pb_field_t field = {
        .descriptor = descriptor,
        .part       = pb_part_create_from_cursor(cursor)
      };
      return field;
    }
  }
  return pb_field_create_invalid();
}

/*!
 * Destroy a field.
 *
 * \param[in,out] field Field
 */
extern void
pb_field_destroy(pb_field_t *field) {
  assert(field);
  pb_part_destroy(&(field->part));
}

/*!
 * Compare the value of a field with the given value.
 *
 * 32-bit and 64-bit fixed-size fields can be compared by raw pointer, all      // TODO: fix
 * other types must be read explicitly.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] field Field
 * \param[in]     value Pointer receiving value
 * \return              Test result
 */
extern int
pb_field_match(pb_field_t *field, const void *value) {
  assert(field && value);
  if (unlikely_(!pb_field_valid(field) || pb_field_align(field)))
    return 0;

  /* Directly compare fixed-size fields */
  size_t size = pb_field_descriptor_type_size(field->descriptor);
  // if (pb_field_descriptor_wiretype(field->descriptor) & PB_WIRETYPE_64BIT)   // TODO: fix
  //   return !memcmp(value, pb_field_raw(field), size);

  /* Allocate temporary space for type-agnostic comparison */
  void *temp = alloca(size);
  if (unlikely_(pb_field_get(field, temp)))
    return 0;

  /* Compare field value according to type */
  return pb_field_descriptor_wiretype(field->descriptor) != PB_WIRETYPE_LENGTH
    ? !memcmp(value, temp, size)
    : pb_string_equals(value, temp);
}

/*!
 * Read the value from a field.
 *
 * This function has to ensure the alignment of the part, since it directly
 * creates the stream from the underlying part.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] field Field
 * \param[out]    value Pointer receiving value
 * \return              Error code
 */
extern pb_error_t
pb_field_get(pb_field_t *field, void *value) {
  assert(field && value);
  if (unlikely_(!pb_field_valid(field) || pb_field_align(field)))
    return PB_ERROR_INVALID;

  /* Create a stream to read the field's value */
  pb_stream_t stream =
    pb_stream_create_from_part(&(field->part));
  pb_type_t  type  = pb_field_descriptor_type(field->descriptor);
  pb_error_t error = pb_stream_read(&stream, type, value);
  pb_stream_destroy(&stream);
  return error;
}

/*!
 * Write a value to a field.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] field Field
 * \param[in]     value Pointer holding value
 * \return              Error code
 */
extern pb_error_t
pb_field_put(pb_field_t *field, const void *value) {
  assert(field && value);
  if (unlikely_(!pb_field_valid(field)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Write new value according to wiretype */
  pb_type_t type = pb_field_descriptor_type(field->descriptor);
  switch (pb_field_descriptor_wiretype(field->descriptor)) {

    /* Write a variable-sized integer */
    case PB_WIRETYPE_VARINT: {

#ifndef NDEBUG

      /* Assert valid value for enum field */
      if (type == PB_TYPE_ENUM)
        assert(pb_enum_descriptor_value_by_number(
          pb_field_descriptor_enum(field->descriptor),
            *(const pb_enum_t *)value));

#endif /* NDEBUG */

      /* Pack variable-sized integer */
      uint8_t data[10];
      error = pb_part_write(&(field->part), data,
        pb_varint_pack(type, data, value));
      break;
    }

    /* Write a fixed-sized 64-bit value */
    case PB_WIRETYPE_64BIT:
      error = pb_part_write(&(field->part), value, 8);
      break;

    /* Write a length-prefixed value */
    case PB_WIRETYPE_LENGTH:
      error = pb_part_write(&(field->part),
        pb_string_data(value), pb_string_size(value));
      break;

    /* Write a fixed-sized 32-bit value */
    case PB_WIRETYPE_32BIT:
      error = pb_part_write(&(field->part), value, 4);
      break;
  }
  return error;
}

/*!
 * Clear a field entirely.
 *
 * When a field is cleared and the underlying message was previously merged
 * with another message, the tag/field of the former message may re-appear,
 * as the current field masked the former. In order to clear all occurrences
 * of a tag/field, use pb_message_erase() on the underlying message. This is
 * coherent with the way merged messages are handled by the default Protocol
 * Buffers implementations.
 *
 * \warning Clearing a field will also invalidate it, as the underlying part
 * cannot restore the associated header (tag and length prefix).
 *
 * \param[in,out] field Field
 * \return              Error code
 */
extern pb_error_t
pb_field_clear(pb_field_t *field) {
  assert(field);
  return pb_part_clear(&(field->part));
}

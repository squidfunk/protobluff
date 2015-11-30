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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/descriptor.h"
#include "core/stream.h"
#include "message/common.h"
#include "message/cursor.h"
#include "message/field.h"
#include "message/journal.h"
#include "message/message.h"
#include "message/part.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a cursor over a message for a specific tag.
 *
 * \warning After creating a cursor, it is mandatory to check its validity
 * with the macro pb_cursor_valid().
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Cursor
 */
extern pb_cursor_t
pb_cursor_create(pb_message_t *message, pb_tag_t tag) {
  assert(message && tag);
  return pb_cursor_create_internal(message, tag);
}

/*!
 * Create a cursor over a message.
 *
 * Internally, the tag may also be initialized to zero, in which case the
 * cursor will halt on every field. The cursor position is initialized with
 * the maximum offset for the subsequent call to pb_cursor_next(), so that
 * the offset will simply wrap around.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Cursor
 */
extern pb_cursor_t
pb_cursor_create_internal(pb_message_t *message, pb_tag_t tag) {
  assert(message);
  if (pb_message_valid(message) && !pb_message_align(message)) {
    pb_cursor_t cursor = {
      .message = pb_message_copy(message),
      .tag     = tag,
      .current = {
        .tag    = 0,
        .offset = {
          .start = pb_message_start(message),
          .end   = pb_message_start(message),
          .diff  = {
            .origin = 0,
            .tag    = 0,
            .length = 0
          }
        }
      },
      .pos   = SIZE_MAX, /* = uninitialized */
      .error = PB_ERROR_NONE
    };
    if (!pb_cursor_next(&cursor))
      cursor.pos = 0;
    return cursor;
  }
  return pb_cursor_create_invalid();
}

/*!
 * Create a cursor over a nested message for a set of tags.
 *
 * Whether the message is valid or not is checked by the cursor, so there is
 * no need to perform this check before creating the cursor.
 *
 * \param[in,out] message Message
 * \param[in]     tags    Tags
 * \param[in]     size    Tag count
 * \return                Cursor
 */
extern pb_cursor_t
pb_cursor_create_nested(
    pb_message_t *message, const pb_tag_t tags[], size_t size) {
  assert(message && tags && size > 1);
  pb_message_t submessage =
    pb_message_create_nested(message, tags, --size);
  pb_cursor_t cursor = pb_cursor_create(&submessage, tags[size]);
  pb_message_destroy(&submessage);
  return cursor;
}

/*!
 * Destroy a cursor.
 *
 * \param[in,out] cursor Cursor
 */
extern void
pb_cursor_destroy(pb_cursor_t *cursor) {
  assert(cursor);
  pb_message_destroy(&(cursor->message));
  cursor->error = PB_ERROR_INVALID;
}

/*!
 * Move a cursor to the next field.
 *
 * \param[in,out] cursor Cursor
 * \return               Test result
 */
extern int
pb_cursor_next(pb_cursor_t *cursor) {
  assert(cursor);
  if (unlikely_(!pb_cursor_valid(cursor)))
    return 0;

  /* If alignment yields an invalid result the current part was most probably
     deleted, but the cursor must not necessarily be invalid */
  cursor->error = pb_cursor_align(cursor);

  /* Continue at end offset until end of message */
  pb_stream_t stream = pb_stream_create_at(
    pb_journal_buffer(pb_cursor_journal(cursor)), cursor->current.offset.end);
  while (pb_stream_offset(&stream) < pb_message_end(&(cursor->message))) {
    pb_offset_t offset = {
      .start = 0,
      .end   = 0,
      .diff  = {
        .origin = pb_stream_offset(&stream),
        .tag    = pb_stream_offset(&stream),
        .length = 0
      }
    };

    /* Read tag from stream */
    pb_tag_t tag = 0; uint32_t length;
    if ((cursor->error = pb_stream_read(&stream, PB_TYPE_UINT32, &tag)))
      break;

    /* Skip field contents to determine length */
    offset.diff.length = pb_stream_offset(&stream);
    if ((tag & 7) == PB_WIRETYPE_LENGTH) {
      if ((cursor->error = pb_stream_read(&stream, PB_TYPE_UINT32, &length)))
        break;
      offset.start = pb_stream_offset(&stream);
      if ((cursor->error = pb_stream_advance(&stream, length)))
        break;
    } else {
      offset.start = pb_stream_offset(&stream);
      if ((cursor->error = pb_stream_skip(&stream, tag & 7)))
        break;
    }

    /* If tags match or we don't care for the tag, we're fine */
    if (!cursor->tag || cursor->tag == (tag >> 3)) {
      offset.end          = pb_stream_offset(&stream);
      offset.diff.origin -= offset.start;
      offset.diff.tag    -= offset.start;
      offset.diff.length -= offset.start;

      /* Update cursor */
      cursor->current.tag    = tag >> 3;
      cursor->current.offset = offset;
      cursor->pos++;

      /* Cleanup and exit */
      pb_stream_destroy(&stream);
      return 1;
    }
  }

  /* Cleanup and invalidate cursor */
  pb_stream_destroy(&stream);
  if (!cursor->error)
    cursor->error = PB_ERROR_EOM;
  return 0;
}

/*!
 * Rewind a cursor to its initial position.
 *
 * \param[in,out] cursor Cursor
 * \return               Test result
 */
extern int
pb_cursor_rewind(pb_cursor_t *cursor) {
  assert(cursor);
  pb_message_t *message = &(cursor->message);
  if (pb_message_valid(message) && !pb_message_align(message)) {
    cursor->current.tag    = 0;
    cursor->current.offset = (pb_offset_t){
      .start = pb_message_start(message),
      .end   = pb_message_start(message),
      .diff  = {
        .origin = 0,
        .tag    = 0,
        .length = 0
      }
    };
    cursor->pos   = SIZE_MAX; /* = uninitialized */
    cursor->error = PB_ERROR_NONE;
  }
  return pb_cursor_next(cursor);
}

/*!
 * Seek a cursor from its current position to a field containing the value.
 *
 * \warning The seek operation is not allowed on cursors created without tags,
 * as the cursor would assume the field type to match the value type.
 *
 * \param[in,out] cursor Cursor
 * \param[in]     value  Pointer holding value
 * \return               Test result
 */
extern int
pb_cursor_seek(pb_cursor_t *cursor, const void *value) {
  assert(cursor && value);
  int result = 0;

  /* Retrieve and check descriptor for current tag */
  if (pb_cursor_valid(cursor) && cursor->tag) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(
        pb_message_descriptor(&(cursor->message)), cursor->current.tag);

    /* Check type and seek for value */
    if (descriptor && pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
      while (!result && pb_cursor_next(cursor)) {
        pb_field_t field = pb_field_create_from_cursor(cursor);
        result = pb_field_match(&field, value);
        pb_field_destroy(&field);
      }
    }
  }
  return result;
}

/*!
 * Compare the value of the current field of a cursor with the given value.
 *
 * \warning If a cursor is created without a tag, the caller is obliged to
 * check the current tag before reading or altering the value in any way.
 *
 * \param[in,out] cursor Cursor
 * \param[in]     value  Pointer holding value
 * \return               Test result
 */
extern int
pb_cursor_match(pb_cursor_t *cursor, const void *value) {
  assert(cursor && value);
  int result = 0;

  /* Retrieve and check descriptor for current tag */
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(
        pb_message_descriptor(&(cursor->message)), cursor->current.tag);

    /* Check type and compare value */
    if (descriptor && pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
      pb_field_t field = pb_field_create_from_cursor(cursor);
      result = pb_field_match(&field, value);
      pb_field_destroy(&field);
    }
  }
  return result;
}

/*!
 * Read the value of the current field from a cursor.
 *
 * \warning If a cursor is created without a tag, the caller is obliged to
 * check the current tag before reading or altering the value in any way.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] cursor Cursor
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_cursor_get(pb_cursor_t *cursor, void *value) {
  assert(cursor && value);
  pb_error_t error = PB_ERROR_INVALID;

  /* Retrieve and check descriptor for current tag */
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(
        pb_message_descriptor(&(cursor->message)), cursor->current.tag);
    if (unlikely_(!descriptor)) {
      error = PB_ERROR_DESCRIPTOR;

    /* Check type and read value */
    } else if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
      pb_field_t field = pb_field_create_from_cursor(cursor);
      error = pb_field_get(&field, value);
      pb_field_destroy(&field);
    }
  }
  return error;
}

/*!
 * Write a value or submessage to the current field of a cursor.
 *
 * \warning If a cursor is created without a tag, the caller is obliged to
 * check the current tag before reading or altering the value in any way.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] cursor Cursor
 * \param[in]     value  Pointer holding value
 * \return               Error code
 */
extern pb_error_t
pb_cursor_put(pb_cursor_t *cursor, const void *value) {
  assert(cursor && value);
  pb_error_t error = PB_ERROR_INVALID;

  /* Retrieve and check descriptor for current tag */
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(
        pb_message_descriptor(&(cursor->message)), cursor->current.tag);
    if (unlikely_(!descriptor)) {
      error = PB_ERROR_DESCRIPTOR;

    /* Create field and write value */
    } else if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
      pb_field_t field = pb_field_create_from_cursor(cursor);
      error = pb_field_put(&field, value);
      pb_field_destroy(&field);

    /* Write submessage to current cursor position */
    } else {
      pb_message_t submessage = pb_message_copy(value);
      assert(pb_cursor_journal(cursor) != pb_message_journal(&submessage));
      if (unlikely_(!pb_message_valid(&submessage) ||
                     pb_message_align(&submessage)))
        return PB_ERROR_INVALID;

      /* Write raw data */
      pb_part_t part = pb_part_create_from_cursor(cursor);
      error = pb_part_write(&part, pb_journal_data_from(
        pb_message_journal(&submessage), pb_message_start(&submessage)),
          pb_message_size(&submessage));

      /* Cleanup before exit */
      pb_part_destroy(&part);
      pb_message_destroy(&submessage);
    }
  }
  return error;
}

/*!
 * Erase the current field or submessage from a cursor.
 *
 * The cursor is reset to the previous part's end offset, so advancing the
 * cursor will set the position to the actual next field.
 *
 * \param[in,out] cursor Cursor
 * \return               Error code
 */
extern pb_error_t
pb_cursor_erase(pb_cursor_t *cursor) {
  assert(cursor);
  pb_error_t error = PB_ERROR_INVALID;

  /* Retrieve and check descriptor for current tag */
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(
        pb_message_descriptor(&(cursor->message)), cursor->current.tag);
    if (unlikely_(!descriptor)) {
      error = PB_ERROR_DESCRIPTOR;

    /* Clear field */
    } else if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
      pb_field_t field = pb_field_create_from_cursor(cursor);
      error = pb_field_clear(&field);
      pb_field_destroy(&field);

    /* Clear submessage */
    } else {
      pb_message_t submessage = pb_message_create_from_cursor(cursor);
      error = pb_message_clear(&submessage);
      pb_message_destroy(&submessage);
    }
  }
  return error;
}

/*!
 * Retrieve a pointer to the raw data of the current field from a cursor.
 *
 * \warning Operating on the raw data of a journal is insanely dangerous and
 * only recommended for 32- and 64-bit fixed-size fields on zero-copy journals.
 * If the underlying journal is altered, the pointer silently loses its
 * validity. All following operations will quietly corrupt the journal.
 *
 * \warning If a cursor is created without a tag, the caller is obliged to
 * check the current tag before reading or altering the value in any way.
 *
 * \param[in,out] cursor Cursor
 * \return               Raw data
 */
extern void *
pb_cursor_raw(pb_cursor_t *cursor) {
  assert(cursor);
  void *value = NULL;

  /* Retrieve and check descriptor for current tag */
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(
        pb_message_descriptor(&(cursor->message)), cursor->current.tag);
    if (descriptor && pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
      pb_field_t field = pb_field_create_from_cursor(cursor);
      value = pb_field_raw(&field);
      pb_field_destroy(&field);
    }
  }
  return value;
}

/*!
 * Ensure that a cursor is properly aligned.
 *
 * This is less of a trivial issue than one might think at first, since the
 * current cursor part, as well as its underlying message need to be aligned.
 *
 * \param[in,out] cursor Cursor
 * \return               Error code
 */
extern pb_error_t
pb_cursor_align(pb_cursor_t *cursor) {
  assert(cursor);
  assert(pb_cursor_valid(cursor));
  pb_error_t error = PB_ERROR_NONE;

  /* Align cursor and current part, if currently unaligned */
  const pb_part_t *part = pb_message_part(&(cursor->message));
  if (unlikely_(!pb_part_aligned(part))) {
    pb_version_t version = pb_cursor_version(cursor);
    if (!(error = pb_message_align(&(cursor->message)))) {
      error = pb_journal_align(pb_cursor_journal(cursor),
        &version, &(cursor->current.offset));
    }
  }
  return error;
}
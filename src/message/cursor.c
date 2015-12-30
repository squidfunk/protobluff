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
#include "message/buffer.h"
#include "message/common.h"
#include "message/cursor.h"
#include "message/field.h"
#include "message/journal.h"
#include "message/message.h"
#include "message/part.h"

/* ----------------------------------------------------------------------------
 * Internal functions
 * ------------------------------------------------------------------------- */

/*!
 * Move a cursor to the next value of a packed field.
 *
 * \param[in,out] cursor Cursor
 * \return               Test result
 */
static int
next_packed(pb_cursor_t *cursor) {
  assert(cursor);
  pb_offset_t *offset = &(cursor->current.offset),
              *packed = &(cursor->current.packed);

  /* Create temporary buffer to read the next value of the packed field */
  pb_buffer_t buffer = pb_buffer_create_zero_copy_internal(
    pb_journal_data_from(pb_cursor_journal(cursor), offset->end),
      packed->end - offset->end);

  /* Create stream over temporary buffer */
  pb_stream_t stream = pb_stream_create(&buffer);
  while (pb_stream_left(&stream)) {

    /* Skip field contents to determine length */
    pb_wiretype_t wiretype =
      pb_field_descriptor_wiretype(cursor->current.descriptor);
    if (unlikely_((cursor->error = pb_stream_skip(&stream, wiretype))))
      break;

    /* Adjust offsets */
    offset->diff.origin -= offset->end - offset->start;
    offset->start        = offset->end;
    offset->end         += pb_stream_offset(&stream);

    /* Cleanup and return with success */
    pb_stream_destroy(&stream);
    pb_buffer_destroy(&buffer);
    return 1;
  }

  /* Cleanup to move on to next field */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);

  /* Switch back to non-packed context, as end is reached */
  *offset = *packed;
  return packed->end = 0;
}

/*!
 * Move a cursor to the next field.
 *
 * \param[in,out] cursor Cursor
 * \return               Test result
 */
static int
next(pb_cursor_t *cursor) {
  assert(cursor);
  pb_offset_t *offset = &(cursor->current.offset),
              *packed = &(cursor->current.packed);

  /* Create temporary buffer to read the next value */
  pb_buffer_t buffer = pb_buffer_create_zero_copy_internal(
    pb_journal_data_from(pb_cursor_journal(cursor), 0),
      pb_message_end(&(cursor->message)));

  /* Create stream over temporary buffer */
  pb_stream_t stream = pb_stream_create_at(&buffer, offset->end);
  while (pb_stream_left(&stream)) {

    /* Adjust offsets */
    offset->start       = offset->end;
    offset->diff.origin = pb_message_start(&(cursor->message));
    offset->diff.tag    = pb_stream_offset(&stream);

    /* Read tag from stream */
    pb_tag_t tag; uint32_t length;
    if ((cursor->error = pb_stream_read(&stream, PB_TYPE_UINT32, &tag)))
      break;

    /* Extract wiretype and tag */
    pb_wiretype_t wiretype = tag & 7;
    tag >>= 3;

    /* Skip field contents to determine length */
    offset->diff.length = pb_stream_offset(&stream);
    if (wiretype == PB_WIRETYPE_LENGTH) {
      if ((cursor->error = pb_stream_read(&stream, PB_TYPE_UINT32, &length)))
        break;
      offset->start = pb_stream_offset(&stream);
      if ((cursor->error = pb_stream_advance(&stream, length)))
        break;
    } else {
      offset->start = pb_stream_offset(&stream);
      if ((cursor->error = pb_stream_skip(&stream, wiretype)))
        break;
    }

    /* Adjust offsets */
    offset->end          = pb_stream_offset(&stream);
    offset->diff.origin -= offset->start;
    offset->diff.tag    -= offset->start;
    offset->diff.length -= offset->start;

    /* If a tag is set check if the tags match or continue */
    if (cursor->tag && cursor->tag != tag) {
      continue;

    /* Otherwise try to load descriptor for current tag */
    } else if (!cursor->current.descriptor ||
        pb_field_descriptor_tag(cursor->current.descriptor) != tag) {
      if (!(cursor->current.descriptor = pb_descriptor_field_by_tag(
          pb_message_descriptor(&(cursor->message)), tag)))
        continue;
    }

    /* Switch to packed context in case of packed field */
    if (wiretype != pb_field_descriptor_wiretype(cursor->current.descriptor) &&
        wiretype == PB_WIRETYPE_LENGTH) {
      *packed = *offset;

      /* Prepare offsets for packed field members */
      offset->end         = offset->start;
      offset->diff.tag    = 0;
      offset->diff.length = 0;
    }

    /* Cleanup and return */
    pb_stream_destroy(&stream);
    pb_buffer_destroy(&buffer);
    return !packed->end;
  }

  /* Invalidate cursor if at end */
  if (!(pb_stream_left(&stream) && cursor->error))
    cursor->error = PB_ERROR_EOM;

  /* Cleanup and return */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
  return 0;
}

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
    const pb_field_descriptor_t *descriptor = tag
      ? pb_descriptor_field_by_tag(pb_message_descriptor(message), tag)
      : NULL;
    pb_cursor_t cursor = {
      .message = pb_message_copy(message),
      .tag     = tag,
      .current = {
        .descriptor = descriptor,
        .offset     = {
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
 * Move a cursor to the next occurrence of a field.
 *
 * If alignment yields an invalid result, the current part was most probably
 * deleted, but the cursor must not necessarily be invalid.
 *
 * \param[in,out] cursor Cursor
 * \return               Test result
 */
extern int
pb_cursor_next(pb_cursor_t *cursor) {
  assert(cursor);
  int result = 0;
  if (pb_cursor_valid(cursor)) {
    pb_cursor_align(cursor);
    do {
      result = cursor->current.packed.end
        ? next_packed(cursor)
        : next(cursor);
      if (result)
        cursor->pos++;
    } while (!cursor->error && !result);
  };
  return result;
}

/*!
 * Move a cursor to the first occurrence of a field.
 *
 * \param[in,out] cursor Cursor
 * \return               Test result
 */
extern int
pb_cursor_first(pb_cursor_t *cursor) {
  assert(cursor);
  pb_cursor_t temp = pb_cursor_create_internal(
    &(cursor->message), cursor->tag);
  pb_cursor_destroy(cursor);
  *cursor = temp;
  return pb_cursor_valid(cursor);
}

/*!
 * Move a cursor to the last occurrence of a field.
 *
 * \param[in,out] cursor Cursor
 * \return               Test result
 */
extern int
pb_cursor_last(pb_cursor_t *cursor) {
  assert(cursor);
  int result = 0;
  if (cursor->tag) {
    pb_cursor_t temp = pb_cursor_create_internal(
      &(cursor->message), cursor->tag);
    if (pb_cursor_valid(&temp)) {
      do {
        if (cursor->tag == pb_cursor_tag(&temp)) {
          pb_cursor_destroy(cursor);
          *cursor = temp;
        }
      } while (pb_cursor_next(&temp));
    }
    if (temp.error == PB_ERROR_EOM)
      result = 1;
    pb_cursor_destroy(&temp);
  }
  return result;
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
  if (pb_cursor_valid(cursor) && cursor->tag) {
    const pb_field_descriptor_t *descriptor = cursor->current.descriptor;

    /* Create field and seek for value */
    if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
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
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor = cursor->current.descriptor;

    /* Create field and compare value */
    if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
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
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor = cursor->current.descriptor;

    /* Create field and read value */
    if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
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
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor = cursor->current.descriptor;

    /* Create field and write value */
    if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
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
  if (pb_cursor_valid(cursor)) {
    const pb_field_descriptor_t *descriptor = cursor->current.descriptor;

    /* Clear field */
    if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
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
    const pb_field_descriptor_t *descriptor = cursor->current.descriptor;

    /* Create field and return pointer to raw data */
    if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
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

  /* Check if cursor is already aligned */
  if (unlikely_(!pb_cursor_aligned(cursor))) {
    pb_version_t version = pb_cursor_version(cursor);

    /* Align current packed context offset, if given */
    const pb_field_descriptor_t *descriptor = cursor->current.descriptor;
    if (pb_field_descriptor_packed(descriptor)) {
      pb_version_t version = pb_cursor_version(cursor);
      error = pb_journal_align(pb_cursor_journal(cursor),
        &version, &(cursor->current.packed));
    }

    /* Align current cursor offset */
    if (!(error = pb_message_align(&(cursor->message)))) {
      error = pb_journal_align(pb_cursor_journal(cursor),
        &version, &(cursor->current.offset));
    }
  }
  return error;
}
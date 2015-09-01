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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/binary.h"
#include "lib/binary/buffer.h"
#include "lib/binary/stream.h"
#include "lib/common.h"
#include "lib/cursor.h"
#include "lib/field/descriptor.h"
#include "lib/journal.h"
#include "lib/message.h"
#include "lib/message/descriptor.h"
#include "lib/part.h"

/* ----------------------------------------------------------------------------
 * Jump tables
 * ------------------------------------------------------------------------- */

/* Jump table: wiretype ==> stream skip method */
static const pb_binary_stream_skip_f skip_jump[7] = {
  [PB_WIRETYPE_VARINT] = pb_binary_stream_skip_varint,
  [PB_WIRETYPE_64BIT]  = pb_binary_stream_skip_fixed64,
  [PB_WIRETYPE_32BIT]  = pb_binary_stream_skip_fixed32
};

/* ----------------------------------------------------------------------------
 * Internal functions
 * ------------------------------------------------------------------------- */

/*!
 * Adjust the length prefix of the part by the provided delta to reflect the
 * current length of the part, and return the (possibly) adjusted delta.
 *
 * \param[in,out] part  Part
 * \param[in,out] delta Delta
 * \return              Error code
 */
static pb_error_t
adjust_prefix(pb_part_t *part, ptrdiff_t *delta) {
  assert(part && part->offset.diff.length);
  assert(pb_part_valid(part) && pb_part_aligned(part));
  pb_error_t error = PB_ERROR_NONE;

  /* Write new length prefix to binary */
  uint32_t bytes = pb_part_size(part);
  pb_binary_buffer_t buffer = pb_binary_buffer_create();
  if (!(error = pb_binary_buffer_write_varint32(&buffer, &bytes))) {
    ptrdiff_t adjust = pb_binary_buffer_size(&buffer)
                     + part->offset.diff.length;
    if (likely_(!adjust)) {
      error = pb_binary_write(part->binary,
        part->offset.start + part->offset.diff.length, part->offset.start,
          pb_binary_buffer_data(&buffer), pb_binary_buffer_size(&buffer));

    /* Length of length prefix changed */
    } else {
      pb_journal_t *journal = pb_binary_journal(part->binary);
      assert(journal);

      /* Write new length prefix to binary and perform manual alignment */
      pb_error_t error = pb_journal_log(journal, part->offset.start
        + part->offset.diff.length, part->offset.start, adjust);
      if (!error) {
        error = pb_binary_write(part->binary,
          part->offset.start + part->offset.diff.length, part->offset.start,
            pb_binary_buffer_data(&buffer), pb_binary_buffer_size(&buffer));
        if (unlikely_(error)) {
          pb_journal_revert(journal);                        /* LCOV_EXCL_LINE */

        /* Update offsets */
        } else {                                             /* LCOV_EXCL_LINE */
          part->version++;
          part->offset.start       += adjust;
          part->offset.end         += adjust;
          part->offset.diff.origin -= adjust;
          part->offset.diff.tag    -= adjust;
          part->offset.diff.length -= adjust;

          /* Adjust delta for subsequent updates */
          *delta += adjust;
        }
      }
    }
  }
  pb_binary_buffer_destroy(&buffer);
  return error;
}

/*!
 * Perform a recursive length prefix update on all messages containing the
 * given part and possibly the part as such up to its origin.
 *
 * This is where all the magic happens. This function takes a binary stream and
 * reads data from it, until it reaches a length-prefixed field. Messages are
 * always length-prefixed, so binary parts that are not can be safely skipped.
 * If the binary stream's current offset matches the tag offset of the part
 * exactly, and the part is empty, we ran into an initialization update, so we
 * can directly abort here.
 *
 * Otherwise, if the binary part contains the part, and thus does not match
 * exactly, we must have found a submessage and need to recurse. The length
 * prefixes must be updated from within, since they are encoded as
 * variable-sized integers and may also affect the size of parent messages.
 *
 * After recursing, it is mandatory to check if the part needs to be
 * realigned, since there may have been updates on the length prefixes of
 * submessages that affect the offsets of the part, and perform such a
 * realignment if necessary.
 *
 * Finally, we write the new length prefix and adjust the delta value if the
 * new length prefix exceeds the length of the current one. That easy.
 *
 * \warning The lines excluded from code coverage are impossible to occur,
 * but they are necessary to ensure application-wide proper error handling.
 *
 * \param[in,out] part   Part
 * \param[in,out] stream Binary stream
 * \param[in,out] delta  Delta
 * \return               Error code
 */
static pb_error_t
adjust_recursive(
    pb_part_t *part, pb_binary_stream_t *stream, ptrdiff_t *delta) {
  assert(part && stream && delta && *delta);
  assert(pb_part_aligned(part));
  pb_error_t error = PB_ERROR_NONE;

  /* Read from the binary stream until the end of the part */
  while (pb_binary_stream_offset(stream) < part->offset.end) {
    if (pb_binary_stream_offset(stream) == part->offset.start - *delta &&
        pb_part_empty(part))
      break;

    /* Skip non-length-prefixed fields */
    pb_tag_t tag = 0; uint32_t bytes;
    size_t offset = pb_binary_stream_offset(stream);
    if ((error = pb_binary_stream_read_varint32(stream, &tag)))
      break;                                               /* LCOV_EXCL_LINE */
    if ((tag & 7) != PB_WIRETYPE_LENGTH) {
      if (!skip_jump[tag & 7] || (error = skip_jump[tag & 7](stream)))
        break;                                             /* LCOV_EXCL_LINE */
      continue;
    }

    /* Create a temporary part, so we can use adjust_prefix() */
    pb_part_t temp = {
      .binary  = pb_part_binary(part),
      .version = pb_part_version(part),
      .offset  = {
        .start = 0,
        .end   = 0,
        .diff  = {
          .origin = offset,
          .tag    = offset,
          .length = pb_binary_stream_offset(stream)
        }
      }
    };

    /* Read length from length-prefixed field */
    if ((error = pb_binary_stream_read_varint32(stream, &bytes)))
      break;                                               /* LCOV_EXCL_LINE */

    /* Update offsets */
    temp.offset.start        = pb_binary_stream_offset(stream);
    temp.offset.end          = temp.offset.start + bytes;
    temp.offset.diff.origin -= temp.offset.start;
    temp.offset.diff.tag    -= temp.offset.start;
    temp.offset.diff.length -= temp.offset.start;

    /* Abort, if we're past the origin */
    if (temp.offset.start > part->offset.start + part->offset.diff.origin)
      break;

    /* The temporary part lies within the part */
    if (temp.offset.start <= part->offset.start &&
        temp.offset.end   >= part->offset.end - *delta) {
      temp.offset.end += *delta;

      /* The parts don't match, so recurse */
      if (temp.offset.start != part->offset.start ||
          temp.offset.end   != part->offset.end) {
        pb_binary_stream_t substream = pb_binary_stream_create_at(
          pb_binary_stream_binary(stream), temp.offset.start);
        error = adjust_recursive(part, &substream, delta);
        pb_binary_stream_destroy(&substream);
        if (unlikely_(error))
          break;                                           /* LCOV_EXCL_LINE */

        /* Parts may be unaligned due to length prefix update */
        if ((!pb_part_aligned(part)  && (error = pb_part_align(part))) ||
            (!pb_part_aligned(&temp) && (error = pb_part_align(&temp))))
          break;                                           /* LCOV_EXCL_LINE */
      }

      /* Adjust length prefix */
      error = adjust_prefix(&temp, delta);
      break;

    /* Otherwise just skip binary part */
    } else if ((error = pb_binary_stream_skip(stream, bytes))) {
      break;                                               /* LCOV_EXCL_LINE */
    }
  }

  /* Invalidate part on error */
  if (unlikely_(error))
    pb_part_invalidate(part);                              /* LCOV_EXCL_LINE */
  return error;
}

/*!
 * Perform a length prefix update on all containing messages.
 *
 * \param[in,out] part  Part
 * \param[in]     delta Delta
 * \return              Error code
 */
static pb_error_t
adjust(pb_part_t *part, ptrdiff_t delta) {
  assert(part && delta);
  assert(pb_part_valid(part));

  /* Create binary stream and perform length prefix update */
  pb_binary_stream_t stream = pb_binary_stream_create(part->binary);
  pb_error_t error = adjust_recursive(part, &stream, &delta);
  pb_binary_stream_destroy(&stream);

  /* Align part, if unaligned */
  if (!error && !pb_part_aligned(part))
    return pb_part_align(part);
  return error;
}

/*!
 * Initialize a part.
 *
 * The journal must be tested for writability before writing to the binary, as
 * the binary write is irreversible, while the journal could be reverted.
 *
 * \warning Some lines excluded from code coverage are impossible to occur,
 * but they are necessary to ensure application-wide proper error handling.
 * Others, especially those concerning the journal and binary, are not
 * testable within the context of a part.
 *
 * \param[in,out] part       Part
 * \param[in]     descriptor Field descriptor
 */
static void
init(pb_part_t *part, const pb_field_descriptor_t *descriptor) {
  assert(part && descriptor);
  assert(pb_part_aligned(part));

  /* Initialize field for writing */
  pb_binary_buffer_t buffer = pb_binary_buffer_create();
  do {
    pb_wiretype_t wiretype = pb_field_descriptor_wiretype(descriptor);
    pb_tag_t tag = pb_field_descriptor_tag(descriptor);

    /* Write tag to buffer and backup offsets */
    uint32_t value = wiretype | (tag << 3);
    if (pb_binary_buffer_write_varint32(&buffer, &value))
      break;                                               /* LCOV_EXCL_LINE */
    part->offset.diff.tag    = part->offset.start;
    part->offset.diff.length = part->offset.start
                             + pb_binary_buffer_size(&buffer);

    /* Write default length of zero for length-prefixed fields */
    if (wiretype == PB_WIRETYPE_LENGTH) {
      uint32_t bytes = 0;
      if (pb_binary_buffer_write_varint32(&buffer, &bytes))
        break;                                             /* LCOV_EXCL_LINE */
    }

    /* New data must be written to part, so obtain journal */
    pb_journal_t *journal = pb_binary_journal(part->binary);
    assert(journal);

    /* Write buffer to binary and perform manual alignment */
    pb_error_t error = pb_journal_log(journal,
      part->offset.start + part->offset.diff.origin,
      part->offset.end, pb_binary_buffer_size(&buffer));
    if (!error) {
      size_t offset = pb_binary_buffer_size(&buffer);
      error = pb_binary_write(part->binary,
        part->offset.start, part->offset.end,
          pb_binary_buffer_data(&buffer), pb_binary_buffer_size(&buffer));
      if (unlikely_(error)) {
        pb_journal_revert(journal);                        /* LCOV_EXCL_LINE */

      /* Update offsets */
      } else {                                             /* LCOV_EXCL_LINE */
        part->version++;
        part->offset.start       += offset;
        part->offset.end         += pb_binary_buffer_size(&buffer);
        part->offset.diff.origin -= offset;
        part->offset.diff.tag    -= part->offset.start;
        part->offset.diff.length -= part->offset.start;

        /* Recursive length prefix update of parent messages */
        if (!adjust(part, pb_binary_buffer_size(&buffer))) {
          pb_binary_buffer_destroy(&buffer);
          return;
        }
      }
    }                                                      /* LCOV_EXCL_LINE */
  } while (0);                                             /* LCOV_EXCL_LINE */

  /* Yes. You pulled a Pobert */
  pb_binary_buffer_destroy(&buffer);                       /* LCOV_EXCL_LINE */
  pb_part_invalidate(part);                                /* LCOV_EXCL_LINE */
}

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a part from a message for a specific tag.
 *
 * This method returns a part to read or alter a certain field or submessage of
 * a message. However, how and when a field or submessage is created in the
 * underlying binary depends on the label of the part, which is defined in the
 * message's descriptor. There are two cases:
 *
 * -# Required and optional parts are directly returned when they are already
 *    contained in the message. Otherwise, they are created and returned empty.
 *
 * -# Repeated parts are always appended to the end of the list of already
 *    contained parts with the same tag when created. Dedicated instances of
 *    repeated parts can be accessed through cursors.
 *
 * At first we try to find the exact offset of the specified part. If there is
 * no exact offset, meaning the part does currently not exist, we record the
 * best matching offset, at which we would expect the part to reside. This is
 * just after the part with the previously smaller (or same) tag.
 *
 * There are a few possible scenarios to consider:
 *
 * -# The cursor is valid, the tag matches exactly and the message is not
 *    repeated. In this case we found an existing part and can return it.
 *
 * -# The cursor is valid but the tag does not match exactly or the part is
 *    repeated, so we create a new part.
 *
 * -# The cursor is not valid, in which case we're at the beginning of the
 *    message, so we create a new part.
 *
 * When the second case is true, we know that the message is either empty, or
 * the part we're looking for does not exist. Therefore we create an empty part
 * and return it.
 *
 * \warning New parts always come without values, so only the tag (and length
 * prefix, if applicable) are present. However, empty fields are not valid and
 * not recognized by the parser (except empty strings), so the caller must
 * ensure that a value is set directly after creating a new part.
 *
 * \warning Merged messages are not implemented as demanded in the Protocol
 * Buffers standard. This would make it necessary to scan the entire binary
 * every time, as there may be overrides for required or optional fields. This
 * is orthogonal to the philosophy of this library, and is thus omitted.
 *
 * \warning The existence of the descriptor field is checked with an assertion,
 * as the tag is assumed to be specified at compile time.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Part
 */
extern pb_part_t
pb_part_create(pb_message_t *message, pb_tag_t tag) {
  assert(message && tag);
  if (pb_message_valid(message) && !pb_message_align(message)) {
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_field_by_tag(pb_message_descriptor(message), tag);
    assert(descriptor);

    /* Determine exact or best matching field offset */
    pb_cursor_t cursor = pb_cursor_create_without_tag(message);
    pb_cursor_t temp   = pb_cursor_copy(&cursor);
    if (pb_cursor_valid(&temp)) {
      do {
        if (pb_cursor_tag(&temp) <= tag) {
          pb_cursor_destroy(&cursor);
          cursor = pb_cursor_copy(&temp);
        }
      } while (pb_cursor_next(&temp));
    }

    /* Don't indicate an error, if the cursor just reached the end */
    if (pb_cursor_valid(&temp) ||
        pb_cursor_error(&temp) == PB_ERROR_OFFSET) {

      /* If the tag matches and the field is non-repeated, we're done */
      size_t start = pb_message_start(message);
      if (pb_cursor_valid(&cursor)) {
        if (pb_cursor_tag(&cursor) == tag &&
            pb_field_descriptor_label(descriptor) != PB_LABEL_REPEATED) {
          pb_part_t part = pb_part_create_from_cursor(&cursor);
          pb_cursor_destroy(&cursor);
          return part;
        }

        /* Otherwise correct insert vector in documented cases */
        const pb_offset_t *offset = pb_cursor_current(&cursor);
        if (pb_cursor_pos(&cursor) || pb_cursor_tag(&cursor) <= tag)
          start = offset->end;
      }
      pb_cursor_destroy(&cursor);

      /* Create and return an empty part */
      pb_part_t part = {
        .binary  = pb_message_binary(message),
        .version = pb_message_version(message),
        .offset  = {
          .start = start,
          .end   = start,
          .diff  = {
            .origin = pb_message_start(message) - start,
            .tag    = 0,
            .length = 0
          }
        }
      };
      init(&part, descriptor);
      return part;
    }
    pb_cursor_destroy(&temp);
  }
  return pb_part_create_invalid();
}

/*!
 * Create a part from a binary.
 *
 * \warning A part does not take ownership of the provided binary, so the
 * caller must ensure that the binary is not freed during operations.
 *
 * \param[in,out] binary Binary
 * \return               Part
 */
extern pb_part_t
pb_part_create_from_binary(pb_binary_t *binary) {
  assert(binary);
  if (pb_binary_valid(binary)) {
    pb_part_t part = {
      .binary  = binary,
      .version = pb_binary_version(binary),
      .offset  = {
        .start = 0,
        .end   = pb_binary_size(binary),
        .diff  = {
          .origin = 0,
          .tag    = 0,
          .length = 0
        }
      }
    };
    return part;
  }
  return pb_part_create_invalid();
}

/*!
 * Create a part at the current position of a cursor.
 *
 * \param[in,out] cursor Cursor
 * \return               Part
 */
extern pb_part_t
pb_part_create_from_cursor(pb_cursor_t *cursor) {
  assert(cursor);
  if (pb_cursor_valid(cursor) && !pb_cursor_align(cursor)) {
    const pb_offset_t *offset = pb_cursor_current(cursor);
    pb_part_t part = {
      .binary  = pb_cursor_binary(cursor),
      .version = pb_cursor_version(cursor),
      .offset  = *offset
    };
    return part;
  }
  return pb_part_create_invalid();
}

/*!
 * Destroy a part.
 *
 * \param[in,out] part Part
 */
extern void
pb_part_destroy(pb_part_t *part) {
  assert(part);
  pb_part_invalidate(part);
}

/*!
 * Retrieve the internal error state of a part.
 *
 * \param[in] part Part
 * \return         Error code
 */
extern pb_error_t
pb_part_error(const pb_part_t *part) {
  assert(part);
  return part->version & PB_PART_INVALID
    ? PB_ERROR_INVALID
    : PB_ERROR_NONE;
}

/*!
 * Write data to a part.
 *
 * \param[in,out] part   Part
 * \param[in]     data[] Binary data
 * \param[in]     size   Binary size
 * \return               Error code
 */
extern pb_error_t
pb_part_write(pb_part_t *part, const uint8_t data[], size_t size) {
  assert(part && data && size);
  if (!pb_part_valid(part) || (!pb_part_aligned(part) && pb_part_align(part)))
    return PB_ERROR_INVALID;

  /* Perform immediate write, if binary size delta is zero */
  ptrdiff_t delta = size - pb_part_size(part);
  if (!delta)
    return pb_binary_write(part->binary,
      part->offset.start, part->offset.end, data, size);

  /* Binary size delta is non-zero, so obtain journal */
  pb_journal_t *journal = pb_binary_journal(part->binary);
  assert(journal);

  /* Write data to binary and perform manual alignment */
  pb_error_t error = pb_journal_log(journal,
    part->offset.start, part->offset.end, delta);
  if (!error) {
    error = pb_binary_write(part->binary,
      part->offset.start, part->offset.end, data, size);
    if (unlikely_(error)) {
      pb_journal_revert(journal);

    /* Update offsets */
    } else {
      part->version++;
      part->offset.end += delta;

      /* Recursive length prefix update of parent messages */
      if (part->offset.diff.length)
        error = adjust_prefix(part, &delta);
      if (!error)
        error = adjust(part, delta);
    }
  }
  return error;
}

/*!
 * Clear data from a part and invalidate it.
 *
 * \warning The lines excluded from code coverage are impossible to occur,
 * but they are necessary to ensure application-wide proper error handling.
 *
 * \param[in,out] part Part
 * \return             Error code
 */
extern pb_error_t
pb_part_clear(pb_part_t *part) {
  assert(part);
  if (!pb_part_valid(part) || (!pb_part_aligned(part) && pb_part_align(part)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* If the binary size delta is zero, nothing has to be done */
  ptrdiff_t delta = -(pb_part_size(part)) + part->offset.diff.tag;
  if (delta) {
    pb_journal_t *journal = pb_binary_journal(part->binary);
    if (unlikely_(!pb_journal_valid(journal)))
      return PB_ERROR_INVALID;

    /* Clear data from binary and perform manual alignment */
    error = pb_journal_log(journal, part->offset.start
      + part->offset.diff.origin, part->offset.end, delta);
    if (!error) {
      error = pb_binary_clear(part->binary, part->offset.start
        + part->offset.diff.tag, part->offset.end);
      if (unlikely_(error)) {
        pb_journal_revert(journal);                        /* LCOV_EXCL_LINE */
        return error;                                      /* LCOV_EXCL_LINE */

      /* Update offsets */
      } else {
        part->version++;
        part->offset.start        = delta + part->offset.end;
        part->offset.end         += delta;
        part->offset.diff.origin  = 0;
        part->offset.diff.tag     = 0;
        part->offset.diff.length  = 0;

        /* Recursive length prefix update of parent messages */
        error = adjust(part, delta);
      }
    }
  }
  pb_part_invalidate(part);
  return error;
}

/*!
 * Ensure that a part is properly aligned.
 *
 * Since alignment is an entirely internal issue, unnecessary alignments should
 * be avoided wherever possible. Therefore we statically assert if the part is
 * already aligned, in order to trace all calls back to their origins.
 *
 * \param[in,out] part Part
 * \return             Error code
 */
extern pb_error_t
pb_part_align(pb_part_t *part) {
  assert(part);
  assert(pb_part_valid(part) && !pb_part_aligned(part));
  return pb_journal_align(pb_binary_journal(part->binary),
    &(part->version), &(part->offset));
}

/* LCOV_EXCL_START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*!
 * Dump a part.
 *
 * To perform a non-destructive alignment before printing, a copy is created to
 * be used for all subsequent operations.
 *
 * \warning Don't use this in production, only for debugging.
 *
 * \param[in] part Part
 */
extern void
pb_part_dump(const pb_part_t *part) {
  assert(part);
  assert(pb_part_valid(part));

  /* Non-destructive alignment for printing */
  pb_part_t copy = pb_part_copy(part);
  if (!pb_part_aligned(&copy) && pb_part_align(&copy)) {
    fprintf(stderr, "ERROR - libprotobluff: invalid part\n");
  } else {
    pb_binary_dump_range(part->binary, copy.offset.start, copy.offset.end);
  }
  pb_part_destroy(&copy);
}

/* LCOV_EXCL_STOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
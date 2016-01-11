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

#include "core/descriptor.h"
#include "core/stream.h"
#include "core/varint.h"
#include "message/common.h"
#include "message/cursor.h"
#include "message/journal.h"
#include "message/message.h"
#include "message/part.h"

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

  /* Write length prefix to buffer */
  uint32_t length = pb_part_size(part);
  uint8_t data[5]; size_t size = pb_varint_pack_uint32(data, &length);

  /* Write data to journal */
  pb_error_t error = pb_journal_write(part->journal,
    part->offset.start + part->offset.diff.length,
    part->offset.start + part->offset.diff.length,
    part->offset.start, data, size);
  if (likely_(!error)) {

    /* Update offsets if necessary */
    ptrdiff_t adjust = size + part->offset.diff.length;
    if (adjust) {
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
  return error;
}

/*!
 * Perform a recursive length prefix update on all messages containing the
 * given part and possibly the part as such up to its origin.
 *
 * This is where all the magic happens. This function takes a stream and reads
 * data from it, until it reaches a length-prefixed field. Messages are always
 * length-prefixed, so stream parts that are not can be safely skipped. If the
 * stream's current offset matches the tag offset of the part exactly, and the
 * part is empty, we ran into an initialization update, so we can abort here.
 *
 * Otherwise, if the stream part contains the part, and thus does not match
 * exactly, we must have found a submessage and need to recurse. The length
 * prefixes must be updated from within, since they are encoded as
 * variable-sized integers and may also affect the size of parent messages.
 * However, packed field are also length-prefixed, so it's important to
 * distinguish them from submessages. This is done by comparing the start
 * offsets and origins of the part and the stream part.
 *
 * After recursing, it is mandatory to check if the part needs to be
 * realigned, since there may have been updates on the length prefixes of
 * submessages that affect the offsets of the part, and perform such a
 * realignment if necessary.
 *
 * Finally, we write the new length prefix and adjust the delta value if the
 * new length prefix exceeds the length of the current one. That easy.
 *
 * \warning The lines excluded from code coverage cannot be triggered within
 * the tests, as they are masked through the previous function calls.
 *
 * \param[in,out] part   Part
 * \param[in,out] stream Stream
 * \param[in,out] delta  Delta
 * \return               Error code
 */
static pb_error_t
adjust_recursive(pb_part_t *part, pb_stream_t *stream, ptrdiff_t *delta) {
  assert(part && stream && delta && *delta);
  assert(pb_part_aligned(part));
  pb_error_t error = PB_ERROR_NONE;

  /* Read from the stream until the end of the part */
  while (pb_stream_offset(stream) < part->offset.end) {
    if (pb_stream_offset(stream) == part->offset.start - *delta &&
        pb_part_empty(part))
      break;

    /* Skip non-length-prefixed fields */
    pb_tag_t tag = 0;
    size_t offset = pb_stream_offset(stream);
    if ((error = pb_stream_read(stream, PB_TYPE_UINT32, &tag)))
      break;                                               /* LCOV_EXCL_LINE */
    if ((tag & 7) != PB_WIRETYPE_LENGTH) {
      if ((error = pb_stream_skip(stream, tag & 7)))
        break;                                             /* LCOV_EXCL_LINE */
      continue;
    }

    /* Create a temporary part, so we can use adjust_prefix() */
    pb_part_t temp = {
      .journal = pb_part_journal(part),
      .version = pb_part_version(part),
      .offset  = {
        .start = 0,
        .end   = 0,
        .diff  = {
          .origin = offset,
          .tag    = offset,
          .length = pb_stream_offset(stream)
        }
      }
    };

    /* Read length from length-prefixed field */
    uint32_t length;
    if ((error = pb_stream_read(stream, PB_TYPE_UINT32, &length)))
      break;                                               /* LCOV_EXCL_LINE */

    /* Update offsets */
    temp.offset.start        = pb_stream_offset(stream);
    temp.offset.end          = temp.offset.start + length;
    temp.offset.diff.origin -= temp.offset.start;
    temp.offset.diff.tag    -= temp.offset.start;
    temp.offset.diff.length -= temp.offset.start;

    /* Abort, if we're not inside a packed field and past the origin */
    size_t origin = part->offset.start + part->offset.diff.origin;
    if (part->offset.diff.tag && temp.offset.start > origin)
      break;

    /* The part lies within the temporary part */
    if (temp.offset.start <= part->offset.start &&
        temp.offset.end   >= part->offset.end - *delta) {
      temp.offset.end += *delta;

      /* Check if we might be at the edge of a packed field */
      if (temp.offset.start + temp.offset.diff.origin < origin) {

        /* The parts don't match, so recurse */
        if (temp.offset.start != part->offset.start ||
            temp.offset.end   != part->offset.end) {
          pb_stream_t substream = pb_stream_create_at(
            pb_stream_buffer(stream), temp.offset.start);
          error = adjust_recursive(part, &substream, delta);
          pb_stream_destroy(&substream);
          if (unlikely_(error))
            break;                                         /* LCOV_EXCL_LINE */

          /* Parts may be unaligned due to length prefix update */
          if (!pb_part_aligned(&temp) && (error = pb_part_align(&temp)))
            break;                                         /* LCOV_EXCL_LINE */
        }

      /* We're inside a packed field, so clear it, if the part is empty */
      } else if (*delta < 0 && pb_part_empty(&temp)) {
        error = pb_journal_clear(part->journal,
          temp.offset.start + temp.offset.diff.origin,
          temp.offset.start + temp.offset.diff.origin,
          temp.offset.end);
        break;
      }

      /* Adjust length prefix */
      error = adjust_prefix(&temp, delta);
      break;

    /* Otherwise just skip stream part */
    } else if ((error = pb_stream_advance(stream, length))) {
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

  /* Create stream and perform length prefix update */
  pb_stream_t stream = pb_stream_create(pb_journal_buffer(part->journal));
  pb_error_t  error  = adjust_recursive(part, &stream, &delta);
  pb_stream_destroy(&stream);
  return error;
}

/*!
 * Initialize a part.
 *
 * \warning The lines excluded from code coverage cannot be triggered within
 * the tests, as they are masked through the previous function calls.
 *
 * \param[in,out] part     Part
 * \param[in]     wiretype Wiretype
 * \param[in]     tag      Tag
 */
static void
init(pb_part_t *part, pb_wiretype_t wiretype, pb_tag_t tag) {
  assert(part && tag);
  assert(pb_part_aligned(part));
  do {

    /* Write tag to temporary buffer */
    uint32_t value = wiretype | (tag << 3);
    uint8_t data[10]; size_t size = pb_varint_pack_uint32(data, &value);

    /* Backup offsets for later update */
    part->offset.diff.tag    = part->offset.start;
    part->offset.diff.length = part->offset.start
                             + size;

    /* Write default length-prefix of zero for length-prefixed fields */
    if (wiretype == PB_WIRETYPE_LENGTH) {
      uint32_t length = 0;
      size += pb_varint_pack_uint32(&(data[size]), &length);
    }

    /* Write data to journal and update offsets */
    pb_error_t error = pb_journal_write(part->journal,
      part->offset.start + part->offset.diff.origin,
      part->offset.start,  part->offset.end, data, size);
    if (likely_(!error)) {
      part->version++;
      part->offset.start       += size;
      part->offset.end         += size;
      part->offset.diff.origin -= size;
      part->offset.diff.tag    -= part->offset.start;
      part->offset.diff.length -= part->offset.start;

      /* Recursive length prefix update of parent messages */
      if (!adjust(part, size))
        return;
    }                                                      /* LCOV_EXCL_LINE */
  } while (0);                                             /* LCOV_EXCL_LINE */

  /* Yes. You pulled a Pobert */
  pb_part_invalidate(part);                                /* LCOV_EXCL_LINE */
}

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a part from a message for a specific tag.
 *
 * This function returns a part to read or alter a certain field or submessage
 * of a message. However, how and when a field or submessage is created in the
 * underlying buffer depends on the label of the part, which is defined in the
 * message's descriptor. There are three cases:
 *
 * -# Required and optional parts are directly returned when they are already
 *    contained in the message. If a message contains multiple occurrences of
 *    such a part, the last occurrence is returned. Otherwise, they are created
 *    and returned empty.
 *
 * -# Additionally to the process described in 1, if a part is member of a
 *    oneof, all succeeding members of the oneof are erased, so the part
 *    corresponding to the given tag is ensured to be active/visible.
 *    Preceeding members of the oneof are not erased from the message.
 *
 * -# Repeated parts are always appended to the end of the list of already
 *    contained parts with the same tag (or as a value to the last packed
 *    field, if applicable) when created. Dedicated instances of repeated
 *    parts can be accessed through cursors.
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
  do {
    if (!pb_message_valid(message) || pb_message_align(message))
      break;

    /* Assert descriptor */
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(pb_message_descriptor(message), tag);
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
    if (pb_cursor_error(&temp) == PB_ERROR_EOM) {
      pb_cursor_destroy(&temp);

      /* Record start offset and handle different cases */
      size_t start = pb_message_start(message);
      if (pb_cursor_valid(&cursor)) {

        /* If the tag is part of a oneof ensure it is the active tag */
        if (pb_field_descriptor_label(descriptor) == PB_LABEL_ONEOF) {
          temp = pb_cursor_copy(&cursor);
          do {
            int member = pb_field_descriptor_oneof(descriptor) ==
              pb_field_descriptor_oneof(pb_cursor_descriptor(&temp));
            if (member && tag != pb_cursor_tag(&temp))
              if (pb_cursor_erase(&temp))
                break;                                     /* LCOV_EXCL_LINE */
          } while (pb_cursor_next(&temp));
          pb_cursor_destroy(&temp);

          /* Ensure alignment of cursor and message */
          pb_cursor_align(&cursor);
          if (pb_message_align(message))
            break;                                         /* LCOV_EXCL_LINE */
        }

        /* If the tag matches and the field is non-repeated, we're done */
        if (pb_cursor_tag(&cursor) == tag &&
            pb_field_descriptor_label(descriptor) != PB_LABEL_REPEATED) {
          pb_part_t part = pb_part_create_from_cursor(&cursor);
          pb_cursor_destroy(&cursor);
          return part;
        }

        /* Otherwise correct insert vector in documented cases */
        const pb_offset_t *offset = pb_cursor_offset(&cursor);
        if (pb_cursor_pos(&cursor) || pb_cursor_tag(&cursor) <= tag)            // TODO: pb_cursor_pos check seems unnecessary - find edge test cacses
          start = offset->end;
      }

      /* Create an empty part */
      pb_part_t part = {
        .journal = pb_message_journal(message),
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

      /* Initialize length prefix of part underlying a packed field */
      if (pb_field_descriptor_packed(descriptor)) {
        if (tag != pb_cursor_tag(&cursor)) {
          init(&part, PB_WIRETYPE_LENGTH, tag);

          /* Correct offsets for packed field */
          part.offset.diff.tag = 0;
          part.offset.diff.length = 0;
        }

      /* Initialize all other cases */
      } else {
        init(&part, pb_field_descriptor_wiretype(descriptor), tag);
      }

      /* Cleanup and return part */
      pb_cursor_destroy(&cursor);
      return part;
    }
    pb_cursor_destroy(&temp);
    pb_cursor_destroy(&cursor);
  } while (0);
  return pb_part_create_invalid();
}

/*!
 * Create a part from a journal.
 *
 * \warning A part does not take ownership of the provided journal, so the
 * caller must ensure that the journal is not freed during operations.
 *
 * \param[in,out] journal Journal
 * \return                Part
 */
extern pb_part_t
pb_part_create_from_journal(pb_journal_t *journal) {
  assert(journal);
  if (pb_journal_valid(journal)) {
    pb_part_t part = {
      .journal = journal,
      .version = pb_journal_version(journal),
      .offset  = {
        .start = 0,
        .end   = pb_journal_size(journal),
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
    const pb_offset_t *offset = pb_cursor_offset(cursor);
    pb_part_t part = {
      .journal = pb_cursor_journal(cursor),
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
 * \param[in]     data[] Raw data
 * \param[in]     size   Raw data size
 * \return               Error code
 */
extern pb_error_t
pb_part_write(pb_part_t *part, const uint8_t data[], size_t size) {
  assert(part && data && size);
  if (!pb_part_valid(part) || (!pb_part_aligned(part) && pb_part_align(part)))
    return PB_ERROR_INVALID;

  /* Write data to journal */
  ptrdiff_t  delta = size - pb_part_size(part);
  pb_error_t error = pb_journal_write(part->journal,
    part->offset.start, part->offset.start,
    part->offset.end, data, size);
  if (likely_(!error)) {

    /* Update offsets if necessary */
    if (delta) {
      part->version++;
      part->offset.end += delta;

      /* Recursive length prefix update of parent messages */
      if (part->offset.diff.length)
        error = adjust_prefix(part, &delta);
      if (!error)
        error = adjust(part, delta);

      /* Ensure aligned part */
      if (!error && !pb_part_aligned(part))
        error = pb_part_align(part);
    }
  }
  return error;
}

/*!
 * Clear data from a part and invalidate it.
 *
 * \param[in,out] part Part
 * \return             Error code
 */
extern pb_error_t
pb_part_clear(pb_part_t *part) {
  assert(part);
  if (!pb_part_valid(part) || (!pb_part_aligned(part) && pb_part_align(part)))
    return PB_ERROR_INVALID;

  /* Adjust origin for correct journaling of packed field */
  ptrdiff_t origin = part->offset.diff.tag
    ? part->offset.diff.origin
    : 0;

  /* Clear data from journal */
  ptrdiff_t  delta = -(pb_part_size(part)) + part->offset.diff.tag;
  pb_error_t error = pb_journal_clear(part->journal,
    part->offset.start + origin,
    part->offset.start + part->offset.diff.tag,
    part->offset.end);
  if (likely_(!error)) {

    /* Update offsets if necessary */
    if (delta) {
      part->version++;
      part->offset.start = delta + part->offset.end;
      part->offset.end  += delta;

      /* Reset origin for non-packed field */
      if (part->offset.diff.tag)
        part->offset.diff.origin = 0;

      /* Recursive length prefix update of parent messages */
      error = adjust(part, delta);
    }
    pb_part_invalidate(part);
  }
  return error;
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
  pb_part_t temp = pb_part_copy(part);
  if (!pb_part_aligned(&temp) && pb_part_align(&temp)) {
    fprintf(stderr, "ERROR - libprotobluff: invalid part\n");
  } else {
    pb_buffer_dump_range(pb_journal_buffer(part->journal),
      temp.offset.start, temp.offset.end);
  }
  pb_part_destroy(&temp);
}

/* LCOV_EXCL_STOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
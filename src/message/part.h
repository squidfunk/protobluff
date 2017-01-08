/*
 * Copyright (c) 2013-2017 Martin Donath <martin.donath@squidfunk.com>
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

#ifndef PB_MESSAGE_PART_H
#define PB_MESSAGE_PART_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <protobluff/message/part.h>

#include "core/stream.h"
#include "message/common.h"
#include "message/journal.h"

/* ----------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------- */

struct pb_cursor_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_part_t
pb_part_create(
  pb_message_t *message,               /* Message */
  pb_tag_t tag);                       /* Tag */

PB_WARN_UNUSED_RESULT
extern pb_part_t
pb_part_create_from_journal(
  pb_journal_t *journal);              /* Journal */

PB_WARN_UNUSED_RESULT
extern pb_part_t
pb_part_create_from_cursor(
  struct pb_cursor_t *cursor);         /* Cursor */

extern void
pb_part_destroy(
  pb_part_t *part);                    /* Part */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_part_write(
  pb_part_t *part,                     /* Part */
  const uint8_t data[],                /* Raw data */
  size_t size);                        /* Raw data size */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_part_clear(
  pb_part_t *part);                    /* Part */

/* ----------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------- */

#define PB_PART_INVALID (1ULL << 63)   /*!< Invalidation marker */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Create an invalid part.
 *
 * \return Part
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_part_t
pb_part_create_invalid(void) {
  pb_part_t part = {
    .version = PB_PART_INVALID
  };
  return part;
}

/*!
 * Create a shallow copy of a part.
 *
 * \param[in] part Part
 * \return         Part copy
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_part_t
pb_part_copy(const pb_part_t *part) {
  assert(part);
  return *part;
}

/*!
 * Retrieve the underlying journal of a part.
 *
 * \param[in] part Part
 * \return         Journal
 */
PB_INLINE pb_journal_t *
pb_part_journal(pb_part_t *part) {
  assert(part);
  return part->journal;
}

/*!
 * Retrieve the version of a part.
 *
 * \param[in] part Part
 * \return         Version
 */
PB_INLINE pb_version_t
pb_part_version(const pb_part_t *part) {
  assert(part);
  return part->version & ~PB_PART_INVALID;
}

/*!
 * Test whether a part is properly aligned.
 *
 * \warning This function does no runtime check for a valid part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Test result
 */
PB_INLINE int
pb_part_aligned(const pb_part_t *part) {
  assert(part);
  assert(pb_part_valid(part));
  return part->version == pb_journal_version(part->journal);
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
PB_WARN_UNUSED_RESULT
PB_INLINE pb_error_t
pb_part_align(pb_part_t *part) {
  assert(part);
  assert(pb_part_valid(part) && !pb_part_aligned(part));
  return pb_journal_align(part->journal,
    &(part->version), &(part->offset));
}

/*!
 * Invalidate a part.
 *
 * \param[in,out] part Part
 */
PB_INLINE void
pb_part_invalidate(pb_part_t *part) {
  assert(part);
  part->version |= PB_PART_INVALID;
}

/*!
 * Retrieve the start offset of a part within its underlying journal.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Start offset
 */
PB_INLINE size_t
pb_part_start(const pb_part_t *part) {
  assert(part);
  assert(pb_part_aligned(part));
  return part->offset.start;
}

/*!
 * Retrieve the end offset of a part within its underlying journal.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         End offset
 */
PB_INLINE size_t
pb_part_end(const pb_part_t *part) {
  assert(part);
  assert(pb_part_aligned(part));
  return part->offset.end;
}

/*!
 * Retrieve the size of a part.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Part size
 */
PB_INLINE size_t
pb_part_size(const pb_part_t *part) {
  assert(part);
  assert(pb_part_aligned(part));
  return part->offset.end - part->offset.start;
}

/*!
 * Test whether a part is empty.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Test result
 */
PB_INLINE int
pb_part_empty(const pb_part_t *part) {
  assert(part);
  assert(pb_part_aligned(part));
  return !pb_part_size(part);
}

/* ------------------------------------------------------------------------- */

/*!
 * Create a stream from a part.
 *
 * This function is located here, since it is highly dependent on the part
 * implementation while using no stream internals.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Stream
 */
PB_INLINE pb_stream_t
pb_stream_create_from_part(const pb_part_t *part) {
  assert(part);
  assert(pb_part_aligned(part));
  return pb_stream_create_at(pb_journal_buffer(part->journal),
    part->offset.start + part->offset.diff.length);
}

#endif /* PB_MESSAGE_PART_H */

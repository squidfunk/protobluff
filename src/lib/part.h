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

#ifndef PB_PART_H
#define PB_PART_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <protobluff/part.h>

#include "lib/binary.h"
#include "lib/binary/stream.h"
#include "lib/common.h"
#include "lib/cursor.h"
#include "lib/journal.h"
#include "lib/message.h"

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
pb_part_create_from_binary(
  pb_binary_t *binary);                /* Binary */

PB_WARN_UNUSED_RESULT
extern pb_part_t
pb_part_create_from_cursor(
  pb_cursor_t *cursor);                /* Cursor */

extern void
pb_part_destroy(
  pb_part_t *part);                    /* Part */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_part_write(
  pb_part_t *part,                     /* Part */
  const uint8_t data[],                /* Binary data */
  size_t size);                        /* Binary size */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_part_clear(
  pb_part_t *part);                    /* Part */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_part_align(
  pb_part_t *part);                    /* Part */

extern void
pb_part_dump(
  const pb_part_t *part);              /* Part */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

#define PB_PART_INVALID (1UL << 63)    /*!< Invalidation marker */

/* ------------------------------------------------------------------------- */

/*!
 * Create an invalid part.
 *
 * \return Part
 */
#define pb_part_create_invalid() \
  ((pb_part_t){ .version = PB_PART_INVALID })

/*!
 * Create a copy of a part.
 *
 * \param[in] part Part
 * \return         Part copy
 */
#define pb_part_copy(part) \
  (assert(part), *(const pb_part_t *)(part))

/*!
 * Retrieve the underlying binary of a part.
 *
 * \param[in] part Part
 * \return         Binary
 */
#define pb_part_binary(part) \
  (assert(part), (part)->binary)

/*!
 * Retrieve the version of a part.
 *
 * \param[in] part Part
 * \return         Version
 */
#define pb_part_version(part) \
  (assert(part), (const pb_version_t)(part)->version & ~PB_PART_INVALID)

/*!
 * Retrieve the start offset of a part within its underlying binary.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Start offset
 */
#define pb_part_start(part) \
  (assert((part) && pb_part_aligned(part)), \
    (const size_t)(part)->offset.start)

/*!
 * Retrieve the end offset of a part within its underlying binary.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         End offset
 */
#define pb_part_end(part) \
  (assert((part) && pb_part_aligned(part)), \
    (const size_t)(part)->offset.end)

/*!
 * Retrieve the size of a part.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Part size
 */
#define pb_part_size(part) \
  (assert((part) && pb_part_aligned(part)), \
    (pb_part_end(part) - pb_part_start(part)))

/*!
 * Test whether a part is empty.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Test result
 */
#define pb_part_empty(part) \
  (assert((part) && pb_part_aligned(part)), \
    !pb_part_size(part))

/*!
 * Test whether a part is properly aligned.
 *
 * \warning This function does no runtime check for a valid part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Test result
 */
#define pb_part_aligned(part) \
  (assert((part) && pb_part_valid(part)), \
    (part)->version == (pb_binary_journal((part)->binary) ? \
      (pb_journal_size(pb_binary_journal((part)->binary))) : 0))

/*!
 * Invalidate a part.
 *
 * \param[in,out] part Part
 */
#define pb_part_invalidate(part) \
  (assert(part), ((part)->version |= PB_PART_INVALID), \
    (void)(0))

/* ------------------------------------------------------------------------- */

/*!
 * Create a binary stream from a part.
 *
 * This macro is located here, since it is highly dependent on the part
 * implementation while using no binary stream internals.
 *
 * \warning This function does no runtime check for an aligned part, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] part Part
 * \return         Binary stream
 */
#define pb_binary_stream_create_from_part(part) \
  (assert((part) && pb_part_aligned(part)), \
    (pb_binary_stream_create_at((part)->binary, \
      (part)->offset.start + (part)->offset.diff.length)))

#endif /* PB_PART_H */
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

#ifndef PB_INCLUDE_MESSAGE_JOURNAL_H
#define PB_INCLUDE_MESSAGE_JOURNAL_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <protobluff/core/allocator.h>
#include <protobluff/message/buffer.h>
#include <protobluff/message/common.h>

/* ----------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------- */

struct pb_journal_entry_t;

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_journal_t {
  pb_buffer_t buffer;                  /*!< Buffer */
  struct {
    struct pb_journal_entry_t *data;   /*!< Journal entries */
    size_t size;                       /*!< Journal entry count */
  } entry;
} pb_journal_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_journal_t
pb_journal_create(
  const uint8_t data[],                /* Raw data */
  size_t size);                        /* Raw data size */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_journal_t
pb_journal_create_with_allocator(
  pb_allocator_t *allocator,           /* Allocator */
  const uint8_t data[],                /* Raw data */
  size_t size);                        /* Raw data size */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_journal_t
pb_journal_create_empty(void);

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_journal_t
pb_journal_create_empty_with_allocator(
  pb_allocator_t *allocator);          /* Allocator */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_journal_t
pb_journal_create_zero_copy(
  uint8_t data[],                      /* Raw data */
  size_t size);                        /* Raw data size */

PB_EXPORT void
pb_journal_destroy(
  pb_journal_t *journal);              /* Journal */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the underlying buffer of a journal.
 *
 * \param[in] journal Journal
 * \return            Buffer
 */
PB_INLINE pb_buffer_t *
pb_journal_buffer(pb_journal_t *journal) {
  assert(journal);
  return &(journal->buffer);
}

/*!
 * Dump a journal.
 *
 * \warning Don't use this in production, only for debugging.
 *
 * \param[in] journal Journal
 */
PB_INLINE void
pb_journal_dump(const pb_journal_t *journal) {
  assert(journal);
  pb_buffer_dump(&(journal->buffer));
}

/*!
 * Retrieve the raw data of a journal.
 *
 * \param[in] journal Journal
 * \return            Raw data
 */
PB_INLINE const uint8_t *
pb_journal_data(const pb_journal_t *journal) {
  assert(journal);
  return pb_buffer_data(&(journal->buffer));
}

/*!
 * Retrieve the size of a journal.
 *
 * \param[in] journal Journal
 * \return            Raw data size
 */
PB_INLINE size_t
pb_journal_size(const pb_journal_t *journal) {
  assert(journal);
  return pb_buffer_size(&(journal->buffer));
}

/*!
 * Test whether a journal is empty.
 *
 * \param[in] journal Journal
 * \return            Test result
 */
PB_INLINE int
pb_journal_empty(const pb_journal_t *journal) {
  assert(journal);
  return pb_buffer_empty(&(journal->buffer));
}

/*!
 * Retrieve the internal error state of a journal.
 *
 * \param[in] journal Journal
 * \return            Error code
 */
PB_INLINE pb_error_t
pb_journal_error(const pb_journal_t *journal) {
  assert(journal);
  return pb_buffer_error(&(journal->buffer));
}

/*!
 * Test whether a buffer is valid.
 *
 * \param[in] journal Journal
 * \return            Test result
 */
PB_INLINE int
pb_journal_valid(const pb_journal_t *journal) {
  assert(journal);
  return !pb_journal_error(journal);
}

#endif /* PB_INCLUDE_MESSAGE_JOURNAL_H */

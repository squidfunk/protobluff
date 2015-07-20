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

#ifndef PB_JOURNAL_H
#define PB_JOURNAL_H

#include <assert.h>
#include <stdlib.h>

#include "lib/common.h"
#include "lib/allocator.h"

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_journal_entry_t {
  size_t origin;                       /*!< Origin */
  size_t offset;                       /*!< Offset */
  ptrdiff_t delta;                     /*!< Delta */
} pb_journal_entry_t;

typedef struct pb_journal_t {
  pb_allocator_t *allocator;           /*!< Allocator */
  struct {
    pb_journal_entry_t *data;          /*!< Journal entries */
    size_t size;                       /*!< Journal entry count */
    size_t bulk;                       /*!< Allocated but empty slots */
  } entry;
} pb_journal_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_journal_t
pb_journal_create(
  size_t bulk);                        /* Bulk */

PB_WARN_UNUSED_RESULT
extern pb_journal_t
pb_journal_create_with_allocator(
  pb_allocator_t *allocator,           /* Allocator */
  size_t bulk);                        /* Bulk */

extern void
pb_journal_destroy(
  pb_journal_t *journal);              /* Journal */

extern pb_error_t
pb_journal_error(
  const pb_journal_t *journal);        /* Journal */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_journal_log(
  pb_journal_t *journal,               /* Journal */
  size_t origin,                       /* Origin */
  size_t offset,                       /* Offset */
  ptrdiff_t delta);                    /* Delta */

extern void
pb_journal_revert(
  pb_journal_t *journal);              /* Journal */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_journal_align(
  const pb_journal_t *journal,         /* Journal */
  pb_version_t *version,               /* Version */
  pb_offset_t *offset);                /* Offset */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Create an invalid journal.
 *
 * \return Journal
 */
#define pb_journal_create_invalid() \
  ((pb_journal_t){})

/*!
 * Retrieve the size of a journal.
 *
 * \param[in] journal Journal
 * \return            Journal size
 */
#define pb_journal_size(journal) \
  (assert(journal), (const size_t)(journal)->entry.size)

/*!
 * Test whether a journal is empty.
 *
 * \param[in] journal Journal
 * \return            Test result
 */
#define pb_journal_empty(journal) \
  (assert(journal), !pb_journal_size(journal))

/*!
 * Test whether a journal is valid.
 *
 * \param[in] journal Journal
 * \return            Test result
 */
#define pb_journal_valid(journal) \
  (assert(journal), !pb_journal_error(journal))

#endif /* PB_JOURNAL_H */
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

#ifndef PB_MESSAGE_JOURNAL_H
#define PB_MESSAGE_JOURNAL_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <protobluff/message/journal.h>

#include "message/buffer.h"
#include "message/common.h"

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_journal_entry_t {
  size_t origin;                       /*!< Origin */
  size_t offset;                       /*!< Offset */
  ptrdiff_t delta;                     /*!< Delta */
} pb_journal_entry_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_journal_write(
  pb_journal_t *journal,               /* Journal */
  size_t origin,                       /* Origin */
  size_t start,                        /* Start offset */
  size_t end,                          /* End offset */
  const uint8_t data[],                /* Raw data */
  size_t size);                        /* Raw data size */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_journal_clear(
  pb_journal_t *journal,               /* Journal */
  size_t origin,                       /* Origin */
  size_t start,                        /* Start offset */
  size_t end);                         /* End offset */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_journal_align(
  const pb_journal_t *journal,         /* Journal */
  pb_version_t *version,               /* Version */
  pb_offset_t *offset);                /* Offset */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Create an invalid journal.
 *
 * \return Journal
 */
PB_INLINE PB_WARN_UNUSED_RESULT
pb_journal_t
pb_journal_create_invalid(void) {
  pb_journal_t journal = {};
  return journal;
}

/*!
 * Retrieve the version of a journal.
 *
 * \param[in] journal Journal
 * \return            Version
 */
PB_INLINE pb_version_t
pb_journal_version(const pb_journal_t *journal) {
  assert(journal);
  return journal->entry.size;
}

/*!
 * Retrieve the raw data of a journal from a given offset.
 *
 * \param[in] journal Journal
 * \param[in] offset  Offset
 * \return            Raw data from offset
 */
PB_INLINE uint8_t *
pb_journal_data_from(const pb_journal_t *journal, size_t offset) {
  return pb_buffer_data_from(&(journal->buffer), offset);
}

/*!
 * Retrieve the raw data of a journal at a given offset.
 *
 * \param[in] journal Journal
 * \param[in] offset  Offset
 * \return            Raw data at offset
 */
PB_INLINE uint8_t
pb_journal_data_at(const pb_journal_t *journal, size_t offset) {
  return pb_buffer_data_at(&(journal->buffer), offset);
}

#endif /* PB_MESSAGE_JOURNAL_H */

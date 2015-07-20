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

#ifndef PB_BINARY_H
#define PB_BINARY_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <protobluff/binary.h>

#include "lib/allocator.h"
#include "lib/binary/buffer.h"
#include "lib/common.h"
#include "lib/journal.h"

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_binary_internal_t {
  pb_allocator_t *allocator;           /*!< Allocator */
  pb_journal_t journal;                /*!< Journal */
} pb_binary_internal_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_write(
  pb_binary_t *binary,                 /* Binary */
  size_t start,                        /* Start offset */
  size_t end,                          /* End offset */
  const uint8_t data[],                /* Binary data */
  size_t size);                        /* Binary size */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_clear(
  pb_binary_t *binary,                 /* Binary */
  size_t start,                        /* Start offset */
  size_t end);                         /* End offset */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Create an invalid binary.
 *
 * \return Binary
 */
#define pb_binary_create_invalid() \
  ((pb_binary_t){})

/*!
 * Retrieve the raw data of a binary from a given offset.
 *
 * \warning This function does no runtime check for a valid binary, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] binary Binary
 * \param[in] offset Offset
 * \return           Binary data from offset
 */
#define pb_binary_data_from(binary, offset) \
  (assert((binary) && pb_binary_valid(binary) && \
    (offset) < (binary)->size), &((binary)->data[(offset)]))

/*!
 * Retrieve the raw data of a binary at a given offset.
 *
 * \warning This function does no runtime check for a valid binary, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] binary Binary
 * \param[in] offset Offset
 * \return           Binary data at offset
 */
#define pb_binary_data_at(binary, offset) \
  (assert((binary) && pb_binary_valid(binary) && \
    (offset) < (binary)->size), (binary)->data[(offset)])

/*!
 * Retrieve the journal of a binary.
 *
 * \warning This function does no runtime check for a valid binary, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] binary Binary
 * \return           Journal
 */
#define pb_binary_journal(binary) \
  (assert((binary) && pb_binary_valid(binary)), \
    &(((pb_binary_internal_t *)(binary)->_)->journal))

/*!
 * Retrieve the version of a binary.
 *
 * \warning This function does no runtime check for a valid binary, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] binary Binary
 * \return           Version
 */
#define pb_binary_version(binary) \
  (assert((binary) && pb_binary_valid(binary)), \
    (pb_journal_size(pb_binary_journal(binary))))

#endif /* PB_BINARY_H */
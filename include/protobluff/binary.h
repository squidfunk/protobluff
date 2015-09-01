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

#ifndef PB_INCLUDE_BINARY_H
#define PB_INCLUDE_BINARY_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <protobluff/allocator.h>
#include <protobluff/common.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_binary_t {
  void *_;                             /*!< Internals */
  uint8_t *data;                       /*!< Binary data */
  size_t size;                         /*!< Binary size */
} pb_binary_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_binary_t
pb_binary_create(
  const uint8_t data[],                /* Binary data */
  size_t size);                        /* Binary size */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_binary_t
pb_binary_create_with_allocator(
  pb_allocator_t *allocator,           /* Allocator */
  const uint8_t data[],                /* Binary data */
  size_t size);                        /* Binary size */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_binary_t
pb_binary_create_empty(void);

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_binary_t
pb_binary_create_empty_with_allocator(
  pb_allocator_t *allocator);          /* Allocator */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_binary_t
pb_binary_create_zero_copy(
  uint8_t data[],                      /* Binary data */
  size_t size);                        /* Binary size */

PB_EXPORT void
pb_binary_destroy(
  pb_binary_t *binary);                /* Binary */

PB_EXPORT pb_error_t
pb_binary_error(
  const pb_binary_t *binary);          /* Binary */

PB_EXPORT void
pb_binary_dump(
  const pb_binary_t *binary);          /* Binary */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the raw data of a binary.
 *
 * \param[in] binary Binary
 * \return           Binary data
 */
#define pb_binary_data(binary) \
  (assert(binary), (const uint8_t *)(binary)->data)

/*!
 * Retrieve the size of a binary.
 *
 * \param[in] binary Binary
 * \return           Binary size
 */
#define pb_binary_size(binary) \
  (assert(binary), (const size_t)(binary)->size)

/*!
 * Test whether a binary is empty.
 *
 * \param[in] binary Binary
 * \return           Test result
 */
#define pb_binary_empty(binary) \
  (assert(binary), !pb_binary_size(binary))

/*!
 * Test whether a binary is valid.
 *
 * \param[in] binary Binary
 * \return           Test result
 */
#define pb_binary_valid(binary) \
  (assert(binary), !pb_binary_error(binary))

#endif /* PB_INCLUDE_BINARY_H */
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

#ifndef PB_INCLUDE_CORE_BUFFER_H
#define PB_INCLUDE_CORE_BUFFER_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <protobluff/core/allocator.h>
#include <protobluff/core/common.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_buffer_t {
  pb_allocator_t *allocator;           /*!< Allocator */
  uint8_t *data;                       /*!< Raw data */
  size_t size;                         /*!< Raw data size */
} pb_buffer_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_buffer_t
pb_buffer_create(
  const uint8_t data[],                /* Raw data */
  size_t size);                        /* Raw data size */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_buffer_t
pb_buffer_create_with_allocator(
  pb_allocator_t *allocator,           /* Allocator */
  const uint8_t data[],                /* Raw data */
  size_t size);                        /* Raw data size */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_buffer_t
pb_buffer_create_empty(void);

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_buffer_t
pb_buffer_create_empty_with_allocator(
  pb_allocator_t *allocator);          /* Allocator */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_buffer_t
pb_buffer_create_zero_copy(
  uint8_t data[],                      /* Raw data */
  size_t size);                        /* Raw data size */

PB_EXPORT void
pb_buffer_destroy(
  pb_buffer_t *buffer);                /* Buffer */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the allocator of a buffer.
 *
 * \param[in] buffer Buffer
 * \return           Allocator
 */
PB_INLINE pb_allocator_t *
pb_buffer_allocator(const pb_buffer_t *buffer) {
  assert(buffer);
  return buffer->allocator;
}

/*!
 * Retrieve the raw data of a buffer.
 *
 * \param[in] buffer Buffer
 * \return           Raw data
 */
PB_INLINE const uint8_t *
pb_buffer_data(const pb_buffer_t *buffer) {
  assert(buffer);
  return buffer->data;
}

/*!
 * Retrieve the size of a buffer.
 *
 * \param[in] buffer Buffer
 * \return           Raw data size
 */
PB_INLINE size_t
pb_buffer_size(const pb_buffer_t *buffer) {
  assert(buffer);
  return buffer->size;
}

/*!
 * Test whether a buffer is empty.
 *
 * \param[in] buffer Buffer
 * \return           Test result
 */
PB_INLINE int
pb_buffer_empty(const pb_buffer_t *buffer) {
  assert(buffer);
  return !pb_buffer_size(buffer);
}

/*!
 * Retrieve the internal error state of a buffer.
 *
 * \param[in] buffer Buffer
 * \return           Error code
 */
PB_INLINE pb_error_t
pb_buffer_error(const pb_buffer_t *buffer) {
  assert(buffer);
  return buffer->allocator
    ? PB_ERROR_NONE
    : PB_ERROR_ALLOC;
}

/*!
 * Test whether a buffer is valid.
 *
 * \param[in] buffer Buffer
 * \return           Test result
 */
PB_INLINE int
pb_buffer_valid(const pb_buffer_t *buffer) {
  assert(buffer);
  return !pb_buffer_error(buffer);
}

#endif /* PB_INCLUDE_CORE_BUFFER_H */

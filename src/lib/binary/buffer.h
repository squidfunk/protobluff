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

#ifndef PB_BINARY_BUFFER_H
#define PB_BINARY_BUFFER_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "lib/common.h"

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_binary_buffer_t {
  uint8_t data[16];                    /*!< Buffer data */
  size_t size;                         /*!< Buffer size */
} pb_binary_buffer_t;

/* ------------------------------------------------------------------------- */

typedef pb_error_t
(*pb_binary_buffer_write_f)(
  pb_binary_buffer_t *buffer,          /*!< Binary buffer */
  const void *value);                  /*!< Pointer holding value */

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_buffer_write_varint8(
  pb_binary_buffer_t *buffer,          /* Binary buffer */
  const void *value);                  /* Pointer holding value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_buffer_write_varint32(
  pb_binary_buffer_t *buffer,          /* Binary buffer */
  const void *value);                  /* Pointer holding value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_buffer_write_varint64(
  pb_binary_buffer_t *buffer,          /* Binary buffer */
  const void *value);                  /* Pointer holding value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_buffer_write_svarint32(
  pb_binary_buffer_t *buffer,          /* Binary buffer */
  const void *value);                  /* Pointer holding value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_buffer_write_svarint64(
  pb_binary_buffer_t *buffer,          /* Binary buffer */
  const void *value);                  /* Pointer holding value */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Create a fixed-size binary buffer.
 *
 * The main purpose of a binary buffer is to act as a gate keeper to a binary,
 * so that invalid variable-sized integers are not written.
 *
 * \return Binary buffer
 */
#define pb_binary_buffer_create() \
  ((pb_binary_buffer_t){})

/*!
 * Free all allocated memory.
 *
 * \param[in,out] buffer Binary buffer
 */
#define pb_binary_buffer_destroy(buffer) \
  (assert(buffer)) /* Nothing to be done */

/*!
 * Retrieve the raw data of a binary buffer.
 *
 * \param[in] buffer Binary buffer
 * \return           Binary buffer data
 */
#define pb_binary_buffer_data(buffer) \
  (assert(buffer), (const uint8_t *)(buffer)->data)

/*!
 * Retrieve the size of a binary buffer.
 *
 * \param[in] buffer Binary buffer
 * \return           Binary buffer size
 */
#define pb_binary_buffer_size(buffer) \
  (assert(buffer), (const size_t)(buffer)->size)

/*!
 * Test whether a binary buffer is empty.
 *
 * \param[in] buffer Binary buffer
 * \return           Test result
 */
#define pb_binary_buffer_empty(buffer) \
  (assert(buffer), !pb_binary_buffer_size(buffer))

/*!
 * Retrieve the number of bytes left in a binary buffer.
 *
 * \param[in] buffer Binary buffer
 * \return           Bytes left
 */
#define pb_binary_buffer_left(buffer) \
  (assert(buffer), 16 - pb_binary_buffer_size(buffer))

#endif /* PB_BINARY_BUFFER_H */
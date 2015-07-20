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

#ifndef PB_BINARY_STREAM_H
#define PB_BINARY_STREAM_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "lib/binary.h"
#include "lib/common.h"

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_binary_stream_t {
  const pb_binary_t *const binary;     /*!< Binary */
  size_t offset;                       /*!< Current offset */
} pb_binary_stream_t;

/* ------------------------------------------------------------------------- */

typedef pb_error_t
(*pb_binary_stream_skip_f)(
  pb_binary_stream_t *stream);         /*!< Binary stream */

typedef pb_error_t
(*pb_binary_stream_read_f)(
  pb_binary_stream_t *stream,          /*!< Binary stream */
  void *value);                        /*!< Pointer receiving value */

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_skip(
  pb_binary_stream_t *stream,          /* Binary stream */
  size_t bytes);                       /* Bytes to skip */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_skip_fixed32(
  pb_binary_stream_t *stream);         /* Binary stream */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_skip_fixed64(
  pb_binary_stream_t *stream);         /* Binary stream */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_skip_varint(
  pb_binary_stream_t *stream);         /* Binary stream */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_skip_length(
  pb_binary_stream_t *stream);         /* Binary stream */

/* ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_read(
  pb_binary_stream_t *stream,          /* Binary stream */
  uint8_t *value);                     /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_read_fixed32(
  pb_binary_stream_t *stream,          /* Binary stream */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_read_fixed64(
  pb_binary_stream_t *stream,          /* Binary stream */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_read_varint8(
  pb_binary_stream_t *stream,          /* Binary stream */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_read_varint32(
  pb_binary_stream_t *stream,          /* Binary stream */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_read_varint64(
  pb_binary_stream_t *stream,          /* Binary stream */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_read_svarint32(
  pb_binary_stream_t *stream,          /* Binary stream */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_read_svarint64(
  pb_binary_stream_t *stream,          /* Binary stream */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_binary_stream_read_length(
  pb_binary_stream_t *stream,          /* Binary stream */
  void *value);                        /* Pointer receiving value */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Create a stream over a binary.
 *
 * \warning A stream does not take ownership of the provided binary, so the
 * caller must ensure that the binary is not freed during operations.
 *
 * \param[in] binary Binary
 * \return           Binary stream
 */
#define pb_binary_stream_create(binary) \
  (assert(binary), (pb_binary_stream_t){ (binary), 0 })

/*!
 * Create a stream over a binary at a given offset.
 *
 * \param[in] binary Binary
 * \param[in] offset Offset
 * \return           Binary stream
 */
#define pb_binary_stream_create_at(binary, offset) \
  (assert((binary) && (offset) <= pb_binary_size(binary)), \
    (pb_binary_stream_t){ (binary), (offset) })

/*!
 * Create a copy of a binary stream.
 *
 * \param[in] stream Binary stream
 * \return           Binary stream copy
 */
#define pb_binary_stream_copy(stream) \
  (assert(stream), *(const pb_binary_stream_t *)(stream))

/*!
 * Destroy a binary stream.
 *
 * \param[in,out] stream Binary stream
 */
#define pb_binary_stream_destroy(stream) \
  (assert(stream)) /* Nothing to be done */

/*!
 * Retrieve the underlying binary of a binary stream.
 *
 * \param[in] stream Binary stream
 * \return           Binary
 */
#define pb_binary_stream_binary(stream) \
  (assert(stream), (stream)->binary)

/*!
 * Retrieve the size of a binary stream.
 *
 * \param[in] stream Binary stream
 * \return           Binary stream size
 */
#define pb_binary_stream_size(stream) \
  (assert(stream), pb_binary_size((stream)->binary))

/*!
 * Test whether a binary stream is empty.
 *
 * \param[in] stream Binary stream
 * \return           Test result
 */
#define pb_binary_stream_empty(stream) \
  (assert(stream), !pb_binary_stream_size(stream))

/*!
 * Retrieve the number of bytes left in a binary stream.
 *
 * \param[in] stream Binary stream
 * \return           Bytes left
 */
#define pb_binary_stream_left(stream) \
  (assert(stream), pb_binary_stream_size(stream) - (stream)->offset)

/*!
 * Retrieve the current offset of a binary stream.
 *
 * \param[in] stream Binary stream
 * \return           Current offset
 */
#define pb_binary_stream_offset(stream) \
  (assert(stream), (const size_t)(stream)->offset)

#endif /* PB_BINARY_STREAM_H */
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

#ifndef PB_CORE_STREAM_H
#define PB_CORE_STREAM_H

#include <assert.h>
#include <stdlib.h>

#include "core/buffer.h"
#include "core/common.h"

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_stream_t {
  const pb_buffer_t *const buffer;     /*!< Buffer */
  size_t offset;                       /*!< Current offset */
} pb_stream_t;

/* ------------------------------------------------------------------------- */

typedef pb_error_t
(*pb_stream_read_f)(
  pb_stream_t *stream,                 /*!< Stream */
  pb_type_t type,                      /*!< Type */
  void *value);                        /*!< Pointer receiving value */

typedef pb_error_t
(*pb_stream_skip_f)(
  pb_stream_t *stream);                /*!< Stream */

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

extern pb_error_t
pb_stream_advance(
  pb_stream_t *stream,                 /* Stream */
  size_t advance);                     /* Bytes to advance */

/* ----------------------------------------------------------------------------
 * Jump tables
 * ------------------------------------------------------------------------- */

/*! Jump table: type ==> read method */
extern const pb_stream_read_f
pb_stream_read_jump[];

/*! Jump table: wiretype ==> skip method */
extern const pb_stream_skip_f
pb_stream_skip_jump[];

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Create a stream over a buffer.
 *
 * \warning A stream does not take ownership of the provided buffer, so the
 * caller must ensure that the buffer is not freed during operations.
 *
 * \param[in] buffer Buffer
 * \return           Stream
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_stream_t
pb_stream_create(const pb_buffer_t *buffer) {
  assert(buffer);
  assert(pb_buffer_valid(buffer));
  pb_stream_t stream = {
    .buffer = buffer,
    .offset = 0
  };
  return stream;
}

/*!
 * Create a stream over a buffer at a given offset.
 *
 * \param[in] buffer Buffer
 * \param[in] offset Offset
 * \return           Stream
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_stream_t
pb_stream_create_at(const pb_buffer_t *buffer, size_t offset) {
  assert(buffer);
  assert(pb_buffer_valid(buffer));
  assert(offset <= pb_buffer_size(buffer));
  pb_stream_t stream = {
    .buffer = buffer,
    .offset = offset
  };
  return stream;
}

/*!
 * Destroy a stream.
 *
 * \param[in,out] stream Stream
 */
PB_INLINE void
pb_stream_destroy(pb_stream_t *stream) {
  assert(stream); /* Nothing to be done */
}

/*!
 * Retrieve the buffer of a stream.
 *
 * \param[in] stream Stream
 * \return           Buffer
 */
PB_INLINE const pb_buffer_t *
pb_stream_buffer(const pb_stream_t *stream) {
  assert(stream);
  return stream->buffer;
}

/*!
 * Retrieve the current offset of a stream.
 *
 * \param[in] stream Stream
 * \return           Current offset
 */
PB_INLINE size_t
pb_stream_offset(const pb_stream_t *stream) {
  assert(stream);
  return stream->offset;
}

/*!
 * Retrieve the number of bytes left in a stream.
 *
 * \param[in] stream Stream
 * \return           Bytes left
 */
PB_INLINE size_t
pb_stream_left(const pb_stream_t *stream) {
  assert(stream);
  return pb_buffer_size(stream->buffer) - stream->offset;
}

/*!
 * Retrieve the size of a stream.
 *
 * \param[in] stream Stream
 * \return           Stream size
 */
PB_INLINE size_t
pb_stream_size(const pb_stream_t *stream) {
  assert(stream);
  return pb_buffer_size(stream->buffer);
}

/*!
 * Test whether a stream is empty.
 *
 * \param[in] stream Stream
 * \return           Test result
 */
PB_INLINE int
pb_stream_empty(const pb_stream_t *stream) {
  assert(stream);
  return !pb_stream_size(stream);
}

/*!
 * Read a value of given type.
 *
 * \param[in,out] stream Stream
 * \param[in]     type   Type
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_error_t
pb_stream_read(pb_stream_t *stream, pb_type_t type, void *value) {
  assert(pb_stream_read_jump[type]);
  return pb_stream_read_jump[type](stream, type, value);
}

/*!
 * Skip a value of given wiretype.
 *
 * \param[in,out] stream   Stream
 * \param[in]     wiretype Wiretype
 * \return                 Error code
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_error_t
pb_stream_skip(pb_stream_t *stream, pb_wiretype_t wiretype) {
  assert(pb_stream_skip_jump[wiretype]);
  return pb_stream_skip_jump[wiretype](stream);
}

#endif /* PB_CORE_STREAM_H */

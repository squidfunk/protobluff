/*
 * Copyright (c) 2013-2020 Martin Donath <martin.donath@squidfunk.com>
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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "core/buffer.h"
#include "core/common.h"
#include "core/stream.h"
#include "core/varint.h"

/* ----------------------------------------------------------------------------
 * Stream read callbacks
 * ------------------------------------------------------------------------- */

/*!
 * Read a variable-sized integer.
 *
 * \param[in,out] stream Stream
 * \param[in]     type   Type
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
static pb_error_t
read_varint(pb_stream_t *stream, pb_type_t type, void *value) {
  assert(stream && value);
  size_t size = pb_varint_unpack(type,
    pb_buffer_data_from(stream->buffer, stream->offset),
    pb_stream_left(stream), value);
  return likely_(size != 0)
    ? pb_stream_advance(stream, size)
    : PB_ERROR_VARINT;
}

/*!
 * Read a fixed-sized 64-bit value.
 *
 * \param[in,out] stream Stream
 * \param[in]     type   Type
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
static pb_error_t
read_64bit(pb_stream_t *stream, pb_type_t type, void *value) {
  assert(stream && value);
  if (unlikely_(pb_stream_advance(stream, 8)))
    return PB_ERROR_OFFSET;
  memcpy(value, pb_buffer_data_from(
    stream->buffer, stream->offset - 8), 8);
  return PB_ERROR_NONE;
}

/*!
 * Read a length-prefixed value.
 *
 * \param[in,out] stream Stream
 * \param[in]     type   Type
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
static pb_error_t
read_length(pb_stream_t *stream, pb_type_t type, void *value) {
  assert(stream && value);
  uint32_t length = 0;
  size_t size = pb_varint_unpack_uint32(
    pb_buffer_data_from(stream->buffer, stream->offset),
    pb_stream_left(stream), &length);
  if (likely_(size != 0)) {
    *(pb_string_t *)value = pb_string_init(
      pb_buffer_data_from(stream->buffer,
        stream->offset + size), length);
    return pb_stream_advance(stream, size + length);
  }
  return PB_ERROR_VARINT;
}

/*!
 * Read a fixed-sized 32-bit value.
 *
 * \param[in,out] stream Stream
 * \param[in]     type   Type
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
static pb_error_t
read_32bit(pb_stream_t *stream, pb_type_t type, void *value) {
  assert(stream && value);
  if (unlikely_(pb_stream_advance(stream, 4)))
    return PB_ERROR_OFFSET;
  memcpy(value, pb_buffer_data_from(
    stream->buffer, stream->offset - 4), 4);
  return PB_ERROR_NONE;
}

/* ----------------------------------------------------------------------------
 * Stream skip callbacks
 * ------------------------------------------------------------------------- */

/*!
 * Skip a variable-sized integer.
 *
 * \param[in,out] stream Stream
 * \return               Error code
 */
static pb_error_t
skip_varint(pb_stream_t *stream) {
  assert(stream);
  size_t size = pb_varint_scan(
    pb_buffer_data_from(stream->buffer, stream->offset),
    pb_stream_left(stream));
  return likely_(size != 0)
    ? pb_stream_advance(stream, size)
    : PB_ERROR_VARINT;
}

/*!
 * Skip a fixed-sized 64-bit value.
 *
 * \param[in,out] stream Stream
 * \return               Error code
 */
static pb_error_t
skip_64bit(pb_stream_t *stream) {
  assert(stream);
  return pb_stream_advance(stream, 8);
}

/*!
 * Skip a length-prefixed value.
 *
 * \param[in,out] stream Stream
 * \return               Error code
 */
static pb_error_t
skip_length(pb_stream_t *stream) {
  assert(stream);
  uint32_t length = 0;
  size_t size = pb_varint_unpack_uint32(
    pb_buffer_data_from(stream->buffer, stream->offset),
    pb_stream_left(stream), &length);
  return likely_(size != 0)
    ? pb_stream_advance(stream, size + length)
    : PB_ERROR_VARINT;
}

/*!
 * Skip a fixed-sized 32-bit value.
 *
 * \param[in,out] stream Stream
 * \return               Error code
 */
static pb_error_t
skip_32bit(pb_stream_t *stream) {
  assert(stream);
  return pb_stream_advance(stream, 4);
}

/* ----------------------------------------------------------------------------
 * Jump tables
 * ------------------------------------------------------------------------- */

/*! Jump table: type ==> read method */
const pb_stream_read_f
pb_stream_read_jump[] = {
  [PB_TYPE_INT32]    = read_varint,
  [PB_TYPE_INT64]    = read_varint,
  [PB_TYPE_UINT32]   = read_varint,
  [PB_TYPE_UINT64]   = read_varint,
  [PB_TYPE_SINT32]   = read_varint,
  [PB_TYPE_SINT64]   = read_varint,
  [PB_TYPE_FIXED32]  = read_32bit,
  [PB_TYPE_FIXED64]  = read_64bit,
  [PB_TYPE_SFIXED32] = read_32bit,
  [PB_TYPE_SFIXED64] = read_64bit,
  [PB_TYPE_BOOL]     = read_varint,
  [PB_TYPE_ENUM]     = read_varint,
  [PB_TYPE_FLOAT]    = read_32bit,
  [PB_TYPE_DOUBLE]   = read_64bit,
  [PB_TYPE_STRING]   = read_length,
  [PB_TYPE_BYTES]    = read_length,
  [PB_TYPE_MESSAGE]  = read_length
};

/*! Jump table: wiretype ==> skip method */
const pb_stream_skip_f
pb_stream_skip_jump[7] = {
  [PB_WIRETYPE_VARINT] = skip_varint,
  [PB_WIRETYPE_64BIT]  = skip_64bit,
  [PB_WIRETYPE_LENGTH] = skip_length,
  [PB_WIRETYPE_32BIT]  = skip_32bit
};

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Advance a stream by the given number of bytes.
 *
 * \param[in,out] stream  Stream
 * \param[in]     advance Bytes to advance
 * \return                Error code
 */
extern pb_error_t
pb_stream_advance(pb_stream_t *stream, size_t advance) {
  assert(stream);
  if (unlikely_(pb_stream_left(stream) < advance))
    return PB_ERROR_OFFSET;
  stream->offset += advance;
  return PB_ERROR_NONE;
}

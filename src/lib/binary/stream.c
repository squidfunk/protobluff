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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lib/binary.h"
#include "lib/binary/stream.h"
#include "lib/common.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Skip a specific amount of bytes in a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[in]     bytes  Bytes to skip
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_skip(pb_binary_stream_t *stream, size_t bytes) {
  assert(stream);
  if (unlikely_(pb_binary_stream_left(stream) < bytes)) {
    stream->offset = pb_binary_stream_size(stream);
    return PB_ERROR_UNDERRUN;
  }
  stream->offset += bytes;
  return PB_ERROR_NONE;
}

/*!
 * Skip a fixed-sized 32-bit value in a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_skip_fixed32(pb_binary_stream_t *stream) {
  return pb_binary_stream_skip(stream, 4);
}

/*!
 * Skip a fixed-sized 64-bit value in a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_skip_fixed64(pb_binary_stream_t *stream) {
  return pb_binary_stream_skip(stream, 8);
}

/*!
 * Skip a variable-sized integer in a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_skip_varint(pb_binary_stream_t *stream) {
  assert(stream);
  uint8_t byte;
  do {
    if (pb_binary_stream_read(stream, &byte))
      return PB_ERROR_UNDERRUN;
  } while (byte & 0x80);
  return PB_ERROR_NONE;
}

/*!
 * Skip a length-prefixed byte sequence in a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_skip_length(pb_binary_stream_t *stream) {
  assert(stream);
  uint32_t bytes; pb_error_t error;
  if ((error = pb_binary_stream_read_varint32(stream, &bytes)))
    return error;
  return pb_binary_stream_skip(stream, bytes);
}

/* ------------------------------------------------------------------------- */

/*!
 * Read a byte from a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_read(pb_binary_stream_t *stream, uint8_t *value) {
  assert(stream && value);
  if (unlikely_(pb_binary_stream_skip(stream, 1)))
    return PB_ERROR_UNDERRUN;
  *value = pb_binary_data_at(stream->binary, stream->offset - 1);
  return PB_ERROR_NONE;
}

/*!
 * Read a fixed-sized 32-bit value from a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_read_fixed32(pb_binary_stream_t *stream, void *value) {
  assert(stream && value);
  if (unlikely_(pb_binary_stream_skip(stream, 4)))
    return PB_ERROR_UNDERRUN;
  memcpy(value, pb_binary_data_from(stream->binary, stream->offset - 4), 4);
  return PB_ERROR_NONE;
}

/*!
 * Read a fixed-sized 64-bit value from a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_read_fixed64(pb_binary_stream_t *stream, void *value) {
  assert(stream && value);
  if (unlikely_(pb_binary_stream_skip(stream, 8)))
    return PB_ERROR_UNDERRUN;
  memcpy(value, pb_binary_data_from(stream->binary, stream->offset - 8), 8);
  return PB_ERROR_NONE;
}

/*!
 * Read a 8-bit variable-sized integer from a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_read_varint8(pb_binary_stream_t *stream, void *value) {
  assert(stream && value);
  if (unlikely_(pb_binary_stream_read(stream, value)))
    return PB_ERROR_UNDERRUN;
  if (unlikely_(*(uint8_t *)value & 0x80))
    return PB_ERROR_OVERFLOW;
  return PB_ERROR_NONE;
}

/*!
 * Read a 32-bit variable-sized integer from a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_read_varint32(pb_binary_stream_t *stream, void *value) {
  assert(stream && value);
  uint8_t byte, offset = 0; *(uint32_t *)value = 0;
  do {
    if (unlikely_(pb_binary_stream_read(stream, &byte)))
      return PB_ERROR_UNDERRUN;
    *(uint32_t *)value |= (uint32_t)(byte & 0x7F) << offset;
    if (unlikely_((offset += 7) > 31 && (byte & 0x80)))
      return PB_ERROR_OVERFLOW;
  } while (byte & 0x80);
  return PB_ERROR_NONE;
}

/*!
 * Read a 64-bit variable-sized integer from a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_read_varint64(pb_binary_stream_t *stream, void *value) {
  assert(stream && value);
  uint8_t byte, offset = 0; *(uint64_t *)value = 0;
  do {
    if (unlikely_(pb_binary_stream_read(stream, &byte)))
      return PB_ERROR_UNDERRUN;
    *(uint64_t *)value |= (uint64_t)(byte & 0x7F) << offset;
    if (unlikely_((offset += 7) > 63 && (byte & 0x80)))
      return PB_ERROR_OVERFLOW;
  } while (byte & 0x80);
  return PB_ERROR_NONE;
}

/*!
 * Read a zig-zag encoded 32-bit variable-sized integer from a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_read_svarint32(pb_binary_stream_t *stream, void *value) {
  assert(stream && value);
  pb_error_t error = pb_binary_stream_read_varint32(stream, value);
  if (!error)
    *(uint32_t *)value = (*(uint32_t *)value << 1)
                       ^ (*(uint32_t *)value >> 31);
  return error;
}

/*!
 * Read a zig-zag encoded 64-bit variable-sized integer from a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_read_svarint64(pb_binary_stream_t *stream, void *value) {
  assert(stream && value);
  pb_error_t error = pb_binary_stream_read_varint64(stream, value);
  if (!error)
    *(uint64_t *)value = (*(uint64_t *)value << 1)
                       ^ (*(uint64_t *)value >> 63);
  return error;
}

/*!
 * Read a length-prefixed byte sequence in a binary stream.
 *
 * \param[in,out] stream Binary stream
 * \param[out]    value  Pointer receiving value
 * \return               Error code
 */
extern pb_error_t
pb_binary_stream_read_length(pb_binary_stream_t *stream, void *value) {
  assert(stream && value);
  uint32_t bytes; pb_error_t error;
  if (unlikely_((error = pb_binary_stream_read_varint32(stream, &bytes))))
    return error;
  if (unlikely_(pb_binary_stream_skip(stream, bytes)))
    return PB_ERROR_UNDERRUN;
  *(pb_string_t *)value = pb_string_init(pb_binary_data_from(
    stream->binary, stream->offset - bytes), bytes);
  return PB_ERROR_NONE;
}
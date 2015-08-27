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

#include "lib/binary/buffer.h"
#include "lib/common.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Write a 8-bit variable-sized integer to a binary buffer.
 *
 * \param[in,out] buffer Binary buffer
 * \param[in]     value  Pointer holding value
 * \return               Error code
 */
extern pb_error_t
pb_binary_buffer_write_varint8(
    pb_binary_buffer_t *buffer, const void *value) {
  assert(buffer && value);
  if (unlikely_(!pb_binary_buffer_left(buffer)))
    return PB_ERROR_OVERFLOW;
  buffer->data[buffer->size++] = *(const uint8_t *)value;
  return PB_ERROR_NONE;
}

/*!
 * Write a 32-bit variable-sized integer to a binary buffer.
 *
 * \param[in,out] buffer Binary buffer
 * \param[in]     value  Pointer holding value
 * \return               Error code
 */
extern pb_error_t
pb_binary_buffer_write_varint32(
    pb_binary_buffer_t *buffer, const void *value) {
  assert(buffer && value);
  uint32_t temp = *(const uint32_t *)value;
  do {
    if (unlikely_(!pb_binary_buffer_left(buffer)))
      return PB_ERROR_OVERFLOW;
    buffer->data[buffer->size++] = (uint8_t)((temp & 0x7F) | 0x80);
  } while ((temp >>= 7));
  buffer->data[buffer->size - 1] &= 0x7F;
  return PB_ERROR_NONE;
}

/*!
 * Write a 64-bit variable-sized integer to a binary buffer.
 *
 * \param[in,out] buffer Binary buffer
 * \param[in]     value  Pointer holding value
 * \return               Error code
 */
extern pb_error_t
pb_binary_buffer_write_varint64(
    pb_binary_buffer_t *buffer, const void *value) {
  assert(buffer && value);
  uint64_t temp = *(const uint64_t *)value;
  do {
    if (unlikely_(!pb_binary_buffer_left(buffer)))
      return PB_ERROR_OVERFLOW;
    buffer->data[buffer->size++] = (uint8_t)((temp & 0x7F) | 0x80);
  } while ((temp >>= 7));
  buffer->data[buffer->size - 1] &= 0x7F;
  return PB_ERROR_NONE;
}

/*!
 * Write a zig-zag encoded 32-bit variable-sized integer to a binary buffer.
 *
 * \param[in,out] buffer Binary buffer
 * \param[in]     value  Pointer holding value
 * \return               Error code
 */
extern pb_error_t
pb_binary_buffer_write_svarint32(
    pb_binary_buffer_t *buffer, const void *value) {
  assert(buffer && value);
  uint32_t temp = ((*(const uint32_t *)value) >> 1)
                ^ ((*(const uint32_t *)value) << 31);
  return pb_binary_buffer_write_varint32(buffer, &temp);
}

/*!
 * Write a zig-zag encoded 64-bit variable-sized integer to a binary buffer.
 *
 * \param[in,out] buffer Binary buffer
 * \param[in]     value  Pointer holding value
 * \return               Error code
 */
extern pb_error_t
pb_binary_buffer_write_svarint64(
    pb_binary_buffer_t *buffer, const void *value) {
  assert(buffer && value);
  uint64_t temp = ((*(const uint64_t *)value) >> 1)
                ^ ((*(const uint64_t *)value) << 63);
  return pb_binary_buffer_write_varint64(buffer, & temp);
}
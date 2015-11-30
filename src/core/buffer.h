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

#ifndef PB_CORE_BUFFER_H
#define PB_CORE_BUFFER_H

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <protobluff/core/buffer.h>

#include "core/allocator.h"
#include "core/common.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern uint8_t *
pb_buffer_grow(
  pb_buffer_t *buffer,                 /* Buffer */
  size_t size);                        /* Additional size */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Create a zero-copy buffer.
 *
 * This function is only for internal use (mainly be the decoder), as the size
 * may be initialized to zero.
 *
 * \param[in,out] data[] Raw data
 * \param[in]     size   Raw data size
 * \return               Buffer
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_buffer_t
pb_buffer_create_zero_copy_internal(uint8_t data[], size_t size) {
  assert(data);
  pb_buffer_t buffer = {
    .allocator = &allocator_zero_copy,
    .data      = data,
    .size      = size
  };
  return buffer;
}

/*!
 * Create an invalid buffer.
 *
 * \return Buffer
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_buffer_t
pb_buffer_create_invalid(void) {
  pb_buffer_t buffer = {};
  return buffer;
}

/*!
 * Retrieve the raw data of a buffer from a given offset.
 *
 * \warning This function does no runtime check for a valid buffer, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] buffer Buffer
 * \param[in] offset Offset
 * \return           Raw data from offset
 */
PB_INLINE uint8_t *
pb_buffer_data_from(const pb_buffer_t *buffer, size_t offset) {
  assert(buffer && offset <= buffer->size);
  assert(pb_buffer_valid(buffer));
  return &(buffer->data[offset]);
}

/* LCOV_EXCL_START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*!
 * Retrieve the raw data of a buffer at a given offset.
 *
 * \warning This function does no runtime check for a valid buffer, as it is
 * only used internally. Errors are catched with assertions during development.
 *
 * \param[in] buffer Buffer
 * \param[in] offset Offset
 * \return           Raw data at offset
 */
PB_INLINE uint8_t
pb_buffer_data_at(const pb_buffer_t *buffer, size_t offset) {
  assert(buffer && offset < buffer->size);
  assert(pb_buffer_valid(buffer));
  return buffer->data[offset];
}

/* LCOV_EXCL_STOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

#endif /* PB_CORE_BUFFER_H */
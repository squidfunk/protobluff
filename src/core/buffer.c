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

#include "core/allocator.h"
#include "core/buffer.h"
#include "core/common.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a buffer.
 *
 * \param[in] data[] Raw data
 * \param[in] size   Raw data size
 * \return           Buffer
 */
extern pb_buffer_t
pb_buffer_create(const uint8_t data[], size_t size) {
  return pb_buffer_create_with_allocator(&allocator_default, data, size);
}

/*!
 * Create a buffer using a custom allocator.
 *
 * \warning A buffer does not take ownership of the provided allocator, so the
 * caller must ensure that the allocator is not freed during operations.
 *
 * \param[in,out] allocator Allocator
 * \param[in]     data[]    Raw data
 * \param[in]     size      Raw data size
 * \return                  Buffer
 */
extern pb_buffer_t
pb_buffer_create_with_allocator(
    pb_allocator_t *allocator, const uint8_t data[], size_t size) {
  assert(allocator && data && size);
  uint8_t *copy = pb_allocator_allocate(allocator, sizeof(uint8_t) * size);
  if (copy) {
    pb_buffer_t buffer = {
      .allocator = allocator,
      .data      = memcpy(copy, data, size),
      .size      = size
    };
    return buffer;
  }
  return pb_buffer_create_invalid();
}

/*!
 * Create an empty buffer.
 *
 * \return Buffer
 */
extern pb_buffer_t
pb_buffer_create_empty(void) {
  return pb_buffer_create_empty_with_allocator(&allocator_default);
}

/*!
 * Create an empty buffer using a custom allocator.
 *
 * \warning A buffer does not take ownership of the provided allocator, so the
 * caller must ensure that the allocator is not freed during operations.
 *
 * \param[in,out] allocator Allocator
 * \return                  Buffer
 */
extern pb_buffer_t
pb_buffer_create_empty_with_allocator(pb_allocator_t *allocator) {
  assert(allocator);
  pb_buffer_t buffer = {
    .allocator = allocator,
    .data      = NULL,
    .size      = 0
  };
  return buffer;
}

/*!
 * Create a zero-copy buffer.
 *
 * Zero-copy buffers are assigned a static internal structure in order to
 * distinguish them from buffers for which initial allocation failed.
 *
 * \warning Zero-copy buffers cannot change in size, so growing or shrinking
 * them will always fail.
 *
 * \param[in,out] data[] Raw data
 * \param[in]     size   Raw data size
 * \return               Buffer
 */
extern pb_buffer_t
pb_buffer_create_zero_copy(uint8_t data[], size_t size) {
  assert(data && size);
  return pb_buffer_create_zero_copy_internal(data, size);
}

/*!
 * Destroy a buffer.
 *
 * \param[in,out] buffer Buffer
 */
extern void
pb_buffer_destroy(pb_buffer_t *buffer) {
  assert(buffer);
  if (buffer->allocator &&
      buffer->allocator != &allocator_zero_copy) {
    if (buffer->data) {
      pb_allocator_free(buffer->allocator, buffer->data);
      buffer->data = NULL;
    }
    buffer->allocator = NULL;
  }
}

/*!
 * Grow a buffer and return a pointer to the newly allocated space.
 *
 * The buffer's internal state is fully recoverable. If allocation fails upon
 * growing the buffer, the buffer is not altered.
 *
 * \param[in,out] buffer Buffer
 * \param[in]     size   Additional size
 * \return               Allocated space
 */
extern uint8_t *
pb_buffer_grow(pb_buffer_t *buffer, size_t size) {
  assert(buffer && size);
  if (likely_(pb_buffer_valid(buffer))) {
    if (unlikely_(buffer->allocator == &allocator_zero_copy))
      return NULL;

    /* Grow buffer and adjust size */
    uint8_t *data = pb_allocator_resize(buffer->allocator,
      buffer->data, buffer->size + size);
    if (data) {
      buffer->data  = data;
      buffer->size += size;

      /* Return reserved space */
      return &(buffer->data[buffer->size - size]);
    }
  }
  return NULL;
}
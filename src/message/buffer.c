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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "core/allocator.h"
#include "message/buffer.h"
#include "message/common.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Write data to a buffer.
 *
 * The buffer's internal state is fully recoverable. If allocation fails upon
 * growing the buffer, the buffer is not altered. In case of a shrinking buffer,
 * a failure will be silently tolerated, since excess memory is not critical.
 * Thus, shrinking a buffer will always work.
 *
 * \param[in,out] buffer Buffer
 * \param[in]     start  Start offset
 * \param[in]     end    End offset
 * \param[in]     data[] Raw data
 * \param[in]     size   Raw data size
 * \return               Error code
 */
extern pb_error_t
pb_buffer_write(
    pb_buffer_t *buffer, size_t start, size_t end,
    const uint8_t data[], size_t size) {
  assert(buffer && data && size);
  if (unlikely_(!pb_buffer_valid(buffer)))
    return PB_ERROR_INVALID;

  /* Check valid range */
  if (unlikely_(start > end || end > buffer->size))
    return PB_ERROR_OFFSET;

  /* Resize buffer and move data if necessary */
  ptrdiff_t delta = size - (end - start);
  if (delta) {
    if (unlikely_(buffer->allocator == &allocator_zero_copy))
      return PB_ERROR_ALLOC;

    /* Buffer grows, so grow space and then move data */
    if (delta > 0) {
      uint8_t *new_data = pb_allocator_resize(buffer->allocator,
        buffer->data, buffer->size + delta);
      if (new_data) {
        buffer->data = new_data;
        if (end < buffer->size)
          memmove(&(buffer->data[end + delta]), &(buffer->data[end]),
            buffer->size - end);
      } else {
        return PB_ERROR_ALLOC;
      }

    /* Buffer shrinks, so move data and then shrink space */
    } else {
      if (end < buffer->size)
        memmove(&(buffer->data[end + delta]), &(buffer->data[end]),
          buffer->size - end);
      uint8_t *new_data = pb_allocator_resize(buffer->allocator,
        buffer->data, buffer->size + delta);
      if (new_data)
        buffer->data = new_data;
    }

    /* Update buffer size */
    buffer->size += delta;
  }

  /* Finally, copy data */
  memcpy(&(buffer->data[start]), data, size);
  return PB_ERROR_NONE;
}

/*!
 * Clear data from a buffer.
 *
 * This function may not be called on zero-copy buffers, as they cannot change
 * in size. The buffer's internal state is fully recoverable.
 *
 * \param[in,out] buffer Buffer
 * \param[in]     start  Start offset
 * \param[in]     end    End offset
 * \return               Error code
 */
extern pb_error_t
pb_buffer_clear(pb_buffer_t *buffer, size_t start, size_t end) {
  assert(buffer);
  if (unlikely_(!pb_buffer_valid(buffer)))
    return PB_ERROR_INVALID;

  /* Check valid range */
  if (unlikely_(start > end || end > buffer->size))
    return PB_ERROR_OFFSET;

  /* Resize buffer and move data if necessary */
  ptrdiff_t delta = start - end;
  if (delta) {
    if (unlikely_(buffer->allocator == &allocator_zero_copy))
      return PB_ERROR_ALLOC;

    /* Buffer shrinks, so move data and then shrink space */
    if (buffer->size + delta) {
      if (end < buffer->size)
        memmove(&(buffer->data[end + delta]), &(buffer->data[end]),
          buffer->size - end);
      uint8_t *new_data = pb_allocator_resize(buffer->allocator,
        buffer->data, buffer->size + delta);
      if (new_data)
        buffer->data = new_data;

    /* Buffer is cleared completely, so free space */
    } else {
      pb_allocator_free(buffer->allocator, buffer->data);
      buffer->data = NULL;
    }

    /* Update buffer size */
    buffer->size += delta;
  }
  return PB_ERROR_NONE;
}

/* LCOV_EXCL_START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*!
 * Dump a buffer.
 *
 * \warning This function may not be used in production, and is therefore also
 * excluded from coverage analysis. It is only meant for debugging!
 *
 * \param[in] buffer Buffer
 */
extern void
pb_buffer_dump(const pb_buffer_t *buffer) {
  pb_buffer_dump_range(buffer, 0, buffer->size);
}

/*!
 * Dump a buffer within a given range.
 *
 * \warning This function may not be used in production, and is therefore also
 * excluded from coverage analysis. It is only meant for debugging!
 *
 * \param[in] buffer Buffer
 * \param[in] start  Start offset
 * \param[in] end    End offset
 */
extern void
pb_buffer_dump_range(const pb_buffer_t *buffer, size_t start, size_t end) {
  assert(buffer);
  assert(pb_buffer_valid(buffer));
  assert(start <= end && end <= buffer->size);

  /* Print statistics */
  fprintf(stderr, "\n");
  fprintf(stderr, " % 4zd  offset start\n", start);
  fprintf(stderr, " % 4zd  offset end\n", end);
  fprintf(stderr, " % 4zd  length\n", end - start);

  /* Print delimiter */
  fprintf(stderr, " ----  --------------------------------------- "
                  " -------------------\n");

  /* Now iterate buffer in blocks */
  for (size_t o = start, width = 10; o < end; o += width) {

    /* Dump numeric representation */
    fprintf(stderr, " % 4zd ", o - start);
    for (size_t p = o; p < o + width && p < end; ++p)
      fprintf(stderr, "% 4d", pb_buffer_data_at(buffer, p));

    /* Fill up missing characters */
    if (o + width > end)
      for (size_t f = width - (end - start) % width; f > 0; f--)
        fprintf(stderr, "    ");

    /* Dump ASCII representation */
    fprintf(stderr, "  ");
    for (size_t p = o; p < o + width && p < end; ++p)
      fprintf(stderr, "%c ", pb_buffer_data_at(buffer, p) > 31 &&
        pb_buffer_data_at(buffer, p) < 127
          ? pb_buffer_data_at(buffer, p)
          : '.');

    /* Reset and break */
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
}

/* LCOV_EXCL_STOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */

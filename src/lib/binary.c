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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/allocator.h"
#include "lib/binary.h"
#include "lib/common.h"
#include "lib/journal.h"

/* ----------------------------------------------------------------------------
 * Static definitions
 * ------------------------------------------------------------------------- */

/* Internals for zero-copy binaries */
static pb_binary_internal_t internal_zero_copy = {
  .allocator = NULL
};

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a binary.
 *
 * \param[in] data[] Binary data
 * \param[in] size   Binary size
 * \return           Binary
 */
extern pb_binary_t
pb_binary_create(const uint8_t data[], size_t size) {
  return pb_binary_create_with_allocator(&allocator_default, data, size);
}

/*!
 * Create a binary using a custom allocator.
 *
 * \warning A binary does not take ownership of the provided allocator, so the
 * caller must ensure that the allocator is not freed during operations.
 *
 * \warning The lines excluded from code coverage cannot be triggered within
 * the tests, as the failing allocator will fail at the first allocation.
 *
 * \param[in,out] allocator Allocator
 * \param[in]     data[]    Binary data
 * \param[in]     size      Binary size
 * \return                  Binary
 */
extern pb_binary_t
pb_binary_create_with_allocator(
    pb_allocator_t *allocator, const uint8_t data[], size_t size) {
  assert(allocator && data && size);
  uint8_t *copy = pb_allocator_alloc(allocator, sizeof(uint8_t) * size);
  if (copy) {
    pb_binary_internal_t *internal =
      pb_allocator_alloc(allocator, sizeof(pb_binary_internal_t));
    if (internal) {
      *internal = (pb_binary_internal_t){
        .allocator = allocator,
        .journal   = pb_journal_create_with_allocator(allocator, 8)
      };
      if (pb_journal_valid(&(internal->journal))) {
        pb_binary_t binary = {
          ._    = internal,
          .data = memcpy(copy, data, size),
          .size = size
        };
        return binary;
      }
      pb_journal_destroy(&(internal->journal));            /* LCOV_EXCL_LINE */
      pb_allocator_free(allocator, internal);              /* LCOV_EXCL_LINE */
    }                                                      /* LCOV_EXCL_LINE */
    pb_allocator_free(allocator, copy);                    /* LCOV_EXCL_LINE */
  }                                                        /* LCOV_EXCL_LINE */
  return pb_binary_create_invalid();
}

/*!
 * Create an empty binary.
 *
 * \return Binary
 */
extern pb_binary_t
pb_binary_create_empty(void) {
  return pb_binary_create_empty_with_allocator(&allocator_default);
}

/*!
 * Create an empty binary using a custom allocator.
 *
 * The journal is created with double the expected size as for the non-empty
 * binary, as more changes are assumed to happen.
 *
 * \warning A binary does not take ownership of the provided allocator, so the
 * caller must ensure that the allocator is not freed during operations.
 *
 * \warning The lines excluded from code coverage cannot be triggered within
 * the tests, as the failing allocator will fail at the first allocation.
 *
 * \param[in,out] allocator Allocator
 * \return                  Binary
 */
extern pb_binary_t
pb_binary_create_empty_with_allocator(pb_allocator_t *allocator) {
  assert(allocator);
  pb_binary_internal_t *internal =
    pb_allocator_alloc(allocator, sizeof(pb_binary_internal_t));
  if (internal) {
    *internal = (pb_binary_internal_t){
      .allocator = allocator,
      .journal   = pb_journal_create_with_allocator(allocator, 8)
    };
    if (pb_journal_valid(&(internal->journal))) {
      pb_binary_t binary = {
        ._    = internal,
        .data = NULL,
        .size = 0
      };
      return binary;
    }
    pb_journal_destroy(&(internal->journal));              /* LCOV_EXCL_LINE */
    pb_allocator_free(allocator, internal);                /* LCOV_EXCL_LINE */
  }                                                        /* LCOV_EXCL_LINE */
  return pb_binary_create_invalid();
}

/*!
 * Create a zero-copy binary.
 *
 * Zero-copy binaries are assigned a static internal structure, in order to
 * distinguish them from binaries for which initial allocation failed.
 *
 * \warning Binaries created without an allocator cannot change in size, so
 * growing or shrinking them will always fail.
 *
 * \param[in,out] data[] Binary data
 * \param[in]     size   Binary size
 * \return               Binary
 */
extern pb_binary_t
pb_binary_create_zero_copy(uint8_t data[], size_t size) {
  assert(data && size);
  pb_binary_t binary = {
    ._    = &internal_zero_copy,
    .data = data,
    .size = size
  };
  return binary;
}

/*!
 * Destroy a binary.
 *
 * \param[in,out] binary Binary
 */
extern void
pb_binary_destroy(pb_binary_t *binary) {
  assert(binary);
  pb_binary_internal_t *internal = binary->_;
  if (internal && internal != &internal_zero_copy) {
    pb_journal_destroy(&(internal->journal));
    if (binary->data) {
      pb_allocator_free(internal->allocator, binary->data);
      binary->data = NULL;
    }
    pb_allocator_free(internal->allocator, internal);
    binary->_ = NULL;
  }
}

/*!
 * Retrieve the internal error state of a binary.
 *
 * \param[in] binary Binary
 * \return           Error code
 */
extern pb_error_t
pb_binary_error(const pb_binary_t *binary) {
  assert(binary);
  return binary->_
    ? PB_ERROR_NONE
    : PB_ERROR_ALLOC;
}

/*!
 * Write data to a binary.
 *
 * The binary's internal state is fully recoverable. If allocation fails upon
 * growing the binary, the binary is not altered. In case of a shrinking binary,
 * a failure will be silently tolerated, since excess memory is not critical.
 * Thus, shrinking a binary will always work.
 *
 * \param[in,out] binary Binary
 * \param[in]     start  Start offset
 * \param[in]     end    End offset
 * \param[in]     data[] Binary data
 * \param[in]     size   Binary size
 * \return               Error code
 */
extern pb_error_t
pb_binary_write(
    pb_binary_t *binary, size_t start, size_t end,
    const uint8_t data[], size_t size) {
  assert(binary && data && size);
  if (unlikely_(!pb_binary_valid(binary)))
    return PB_ERROR_INVALID;

  /* Check valid range */
  if (unlikely_(start > end || end > binary->size))
    return PB_ERROR_OFFSET;

  /* Resize binary and move data if necessary */
  ptrdiff_t delta = size - (end - start);
  if (delta) {
    pb_binary_internal_t *internal = binary->_;
    if (unlikely_(internal == &internal_zero_copy))
      return PB_ERROR_ALLOC;

    /* Binary grows, so grow space and then move data */
    if (delta > 0) {
      uint8_t *new_data = pb_allocator_realloc(internal->allocator,
        binary->data, binary->size + delta);
      if (new_data) {
        binary->data = new_data;
        if (end < binary->size)
          memmove(&(binary->data[end + delta]), &(binary->data[end]),
            binary->size - end);
      } else {
        return PB_ERROR_ALLOC;
      }

    /* Binary shrinks, so move data and then shrink space */
    } else {
      if (end < binary->size)
        memmove(&(binary->data[end + delta]), &(binary->data[end]),
          binary->size - end);
      uint8_t *new_data = pb_allocator_realloc(internal->allocator,
        binary->data, binary->size + delta);
      if (new_data)
        binary->data = new_data;
    }

    /* Update binary size */
    binary->size += delta;
  }

  /* Finally, copy data */
  memcpy(&(binary->data[start]), data, size);
  return PB_ERROR_NONE;
}

/*!
 * Append data to a binary.
 *
 * The binary's internal state is fully recoverable. If allocation fails upon
 * growing the binary, the binary is not altered.
 *
 * \param[in,out] binary Binary
 * \param[in]     data[] Binary data
 * \param[in]     size   Binary size
 * \return               Error code
 */
extern pb_error_t
pb_binary_append(
    pb_binary_t *binary, const uint8_t data[], size_t size) {
  assert(binary && data && size);
  if (unlikely_(!pb_binary_valid(binary)))
    return PB_ERROR_INVALID;

  /* Ensure non-zero-copy binary */
  pb_binary_internal_t *internal = binary->_;
  assert(internal != &internal_zero_copy);

  /* Binary grows, so grow space and then move data */
  uint8_t *new_data = pb_allocator_realloc(internal->allocator,
    binary->data, binary->size + size);
  if (new_data) {
    binary->data = new_data;
  } else {
    return PB_ERROR_ALLOC;
  }

  /* Finally, copy data and update binary size */
  memcpy(&(binary->data[binary->size]), data, size);
  binary->size += size;
  return PB_ERROR_NONE;
}

/*!
 * Clear data from a binary.
 *
 * This method may not be called on zero-copy binaries, as they cannot change
 * in size. The binary's internal state is fully recoverable.
 *
 * \param[in,out] binary Binary
 * \param[in]     start  Start offset
 * \param[in]     end    End offset
 * \return               Error code
 */
extern pb_error_t
pb_binary_clear(pb_binary_t *binary, size_t start, size_t end) {
  assert(binary);
  if (unlikely_(!pb_binary_valid(binary)))
    return PB_ERROR_INVALID;

  /* Check valid range */
  if (unlikely_(start > end || end > binary->size))
    return PB_ERROR_OFFSET;

  /* Resize binary and move data if necessary */
  ptrdiff_t delta = start - end;
  if (delta) {
    pb_binary_internal_t *internal = binary->_;
    if (unlikely_(internal == &internal_zero_copy))
      return PB_ERROR_ALLOC;

    /* Binary shrinks, so move data and then shrink space */
    if (binary->size + delta) {
      if (end < binary->size)
        memmove(&(binary->data[end + delta]), &(binary->data[end]),
          binary->size - end);
      uint8_t *new_data = pb_allocator_realloc(internal->allocator,
        binary->data, binary->size + delta);
      if (new_data)
        binary->data = new_data;

    /* Binary is cleared completely, so free space */
    } else {
      pb_allocator_free(internal->allocator, binary->data);
      binary->data = NULL;
    }

    /* Update binary size */
    binary->size += delta;
  }
  return PB_ERROR_NONE;
}

/* LCOV_EXCL_START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*!
 * Dump a binary.
 *
 * \warning This function may not be used in production, and is therefore also
 * excluded from coverage analysis. It is only meant for debugging!
 *
 * \param[in] binary Binary
 */
extern void
pb_binary_dump(const pb_binary_t *binary) {
  pb_binary_dump_range(binary, 0, binary->size);
}

/*!
 * Dump a binary within a given range.
 *
 * \warning This function may not be used in production, and is therefore also
 * excluded from coverage analysis. It is only meant for debugging!
 *
 * \param[in] binary Binary
 * \param[in] start  Start offset
 * \param[in] end    End offset
 */
extern void
pb_binary_dump_range(const pb_binary_t *binary, size_t start, size_t end) {
  assert(binary);
  assert(pb_binary_valid(binary));
  assert(start <= end && end <= binary->size);

  /* Print statistics */
  fprintf(stderr, "\n");
  fprintf(stderr, " % 4zd  offset start\n", start);
  fprintf(stderr, " % 4zd  offset end\n", end);
  fprintf(stderr, " % 4zd  length\n", end - start);

  /* Print delimiter */
  fprintf(stderr, " ----  --------------------------------------- "
                  " -------------------\n");

  /* Now iterate binary in blocks */
  for (size_t o = start, width = 10; o < end; o += width) {

    /* Dump numeric representation */
    fprintf(stderr, " % 4zd ", o - start);
    for (size_t p = o; p < o + width && p < end; ++p)
      fprintf(stderr, "% 4d", pb_binary_data_at(binary, p));

    /* Fill up missing characters */
    if (o + width > end)
      for (size_t f = width - (end - start) % width; f > 0; f--)
        fprintf(stderr, "    ");

    /* Dump ASCII representation */
    fprintf(stderr, "  ");
    for (size_t p = o; p < o + width && p < end; ++p)
      fprintf(stderr, "%c ", pb_binary_data_at(binary, p) > 31 &&
        pb_binary_data_at(binary, p) < 127
          ? pb_binary_data_at(binary, p)
          : '.');

    /* Reset and break */
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
}

/* LCOV_EXCL_STOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
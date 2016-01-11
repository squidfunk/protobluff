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
#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "core/allocator.h"
#include "message/buffer.h"
#include "message/common.h"

/* ----------------------------------------------------------------------------
 * System-default allocator callback overrides
 * ------------------------------------------------------------------------- */

/*!
 * Allocator with failing allocation.
 *
 * \param[in,out] data Internal allocator data
 * \param[in]     size Bytes to be allocated
 * \return             Memory block
 */
static void *
allocator_allocate_fail(void *data, size_t size) {
  assert(!data && size);
  return NULL;
}

/*!
 * Allocator with failing reallocation.
 *
 * \param[in,out] data  Internal allocator data
 * \param[in,out] block Memory block to be resized
 * \param[in]     size  Bytes to be allocated
 * \return              Memory block
 */
static void *
allocator_resize_fail(void *data, void *block, size_t size) {
  assert(!data && size);
  return NULL;
}

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Write data to a buffer.
 */
START_TEST(test_write) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Update buffer: "SOME DATA" => "MORE DATA THAN ..." */
  uint8_t new_data[] = "MORE DATA THAN THIS BUFFER CURRENTLY HOLDS";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_buffer_write(&buffer, 0, size, new_data, 42));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(42, pb_buffer_size(&buffer));

  /* Assert same contents but different location */
  fail_if(memcmp(new_data, pb_buffer_data(&buffer), 42));
  ck_assert_ptr_ne(new_data, pb_buffer_data(&buffer));
  ck_assert_ptr_ne(data, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Write data to an empty buffer.
 */
START_TEST(test_write_empty) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create_empty();

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Update buffer: "" => "SOME DATA" */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_buffer_write(&buffer, 0, 0, data, size));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size, pb_buffer_size(&buffer));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_buffer_data(&buffer), size));
  ck_assert_ptr_ne(data, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Write data of the same length to a zero-copy buffer.
 */
START_TEST(test_write_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create_zero_copy(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Update buffer: "SOME DATA" => "NEW STUFF" */
  uint8_t new_data[] = "NEW STUFF";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_buffer_write(&buffer, 0, size, new_data, size));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size, pb_buffer_size(&buffer));

  /* Assert same contents but different location */
  fail_if(memcmp(new_data, pb_buffer_data(&buffer), size));
  ck_assert_ptr_ne(new_data, pb_buffer_data(&buffer));
  ck_assert_ptr_eq(data, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Prepend data to a buffer.
 */
START_TEST(test_write_prepend) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Update buffer: "SOME DATA" => "JUST SOME DATA" */
  uint8_t new_data[] = "JUST ";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_buffer_write(&buffer, 0, 0, new_data, 5));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(14, pb_buffer_size(&buffer));

  /* Assert new contents and different location */
  fail_if(memcmp("JUST SOME DATA",
    pb_buffer_data(&buffer), size + 5));
  ck_assert_ptr_ne(new_data, pb_buffer_data(&buffer));
  ck_assert_ptr_ne(data, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Append data to a buffer.
 */
START_TEST(test_write_append) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Update buffer: "SOME DATA" => "SOME DATA IS AWESOME" */
  uint8_t new_data[] = " IS AWESOME";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_buffer_write(&buffer, size, size, new_data, 11));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size + 11, pb_buffer_size(&buffer));

  /* Assert new contents and different location */
  fail_if(memcmp("SOME DATA IS AWESOME",
    pb_buffer_data(&buffer), size + 11));
  ck_assert_ptr_ne(new_data, pb_buffer_data(&buffer));
  ck_assert_ptr_ne(data,     pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Write data to a buffer incrementally to test versioning.
 */
START_TEST(test_write_incremental) {
  pb_buffer_t buffer = pb_buffer_create_empty();

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Iteratively grow buffer: "" => "HELP I'M TRAPPED ..." */
  uint8_t data[] = "HELP I'M TRAPPED IN A UNIVERSE FACTORY";
  for (size_t s = 1; s < 38; s++) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_write(&buffer, 0,
      pb_buffer_size(&buffer), data, s));

    /* Assert buffer size */
    fail_if(pb_buffer_empty(&buffer));
    ck_assert_uint_eq(s, pb_buffer_size(&buffer));

    /* Assert same contents but different location */
    fail_if(memcmp(data, pb_buffer_data(&buffer), s));
    ck_assert_ptr_ne(data, pb_buffer_data(&buffer));
  }

  /* Iteratively shrink buffer: "HELP ... UNIVERSE FACTORY" => "FACTORY" */
  for (size_t s = 30; s > 0; s--) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_write(&buffer, 0,
      pb_buffer_size(&buffer) - 8, data, s));

    /* Assert buffer size */
    fail_if(pb_buffer_empty(&buffer));
    ck_assert_uint_eq(s + 8, pb_buffer_size(&buffer));

    /* Assert same contents but different location */
    fail_if(memcmp(data, pb_buffer_data(&buffer), s));
    ck_assert_ptr_ne(data, pb_buffer_data(&buffer));
  }

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Write data to an invalid buffer.
 */
START_TEST(test_write_invalid) {
  pb_buffer_t buffer = pb_buffer_create_invalid();

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(&buffer));

  /* Try to update buffer */
  uint8_t data[] = "THIS WON'T WORK ANYWAY";
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_buffer_write(&buffer, 0, 0, data, 22));

  /* Assert buffer validity and error again */
  fail_if(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Assert empty buffer */
  ck_assert_ptr_eq(NULL, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Write data to a buffer but specify an invalid offset (start > end).
 */
START_TEST(test_write_invalid_offset) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Try to update buffer */
  uint8_t new_data[] = "THIS WON'T WORK ANYWAY";
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_buffer_write(&buffer, size, 0, new_data, 22));

  /* Assert buffer validity and error again */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size, pb_buffer_size(&buffer));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_buffer_data(&buffer), size));
  ck_assert_ptr_ne(new_data, pb_buffer_data(&buffer));
  ck_assert_ptr_ne(data, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Write data to a buffer but specify an invalid range (end > size).
 */
START_TEST(test_write_invalid_range) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Try to update buffer */
  uint8_t new_data[] = "WHATEVER";
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_buffer_write(&buffer, 0, size + 1, new_data, 8));

  /* Assert buffer validity and error again */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size, pb_buffer_size(&buffer));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_buffer_data(&buffer), size));
  ck_assert_ptr_ne(new_data, pb_buffer_data(&buffer));
  ck_assert_ptr_ne(data, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Write data to a buffer for which allocation fails.
 */
START_TEST(test_write_invalid_allocate) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_allocate_fail,
      .resize   = allocator_default.proc.resize,
      .free     = allocator_default.proc.free
    }
  };

  /* Create buffer */
  pb_buffer_t buffer =
    pb_buffer_create_with_allocator(&allocator, data, size);

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(&buffer));

  /* Try to update buffer */
  uint8_t new_data[] = "THIS WILL FAIL";
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_buffer_write(&buffer, 0, size, new_data, 14));

  /* Assert buffer validity and error again */
  fail_if(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Assert empty buffer */
  ck_assert_ptr_eq(NULL, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Write data to a buffer for which reallocation fails.
 */
START_TEST(test_write_invalid_resize) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_resize_fail,
      .free     = allocator_default.proc.free
    }
  };

  /* Create buffer */
  pb_buffer_t buffer =
    pb_buffer_create_with_allocator(&allocator, data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Iteratively grow buffer */
  uint8_t new_data[] = "THIS WILL FAIL";
  for (size_t s = 10; s < 14; s++) {
    ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_write(&buffer, 0,
      pb_buffer_size(&buffer), new_data, s));

    /* Assert buffer size */
    fail_if(pb_buffer_empty(&buffer));
    ck_assert_uint_eq(size, pb_buffer_size(&buffer));
  }

  /* Assert buffer validity and error again */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Iteratively shrink buffer */
  for (size_t s = 8; s > 0; s--) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_write(&buffer, 0,
      pb_buffer_size(&buffer), new_data, s));

    /* Assert buffer size */
    fail_if(pb_buffer_empty(&buffer));
    ck_assert_uint_eq(s, pb_buffer_size(&buffer));
  }

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Write data to a zero-copy buffer exceeding its current length.
 */
START_TEST(test_write_invalid_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create_zero_copy(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Try to update buffer */
  uint8_t new_data[] = "TOO MUCH DATA";
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_buffer_write(&buffer, 0, size, new_data, 13));

  /* Assert buffer validity and error again */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Clear data from a buffer.
 */
START_TEST(test_clear) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Clear buffer: "SOME DATA" => "DATA" */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_clear(&buffer, 0, 5));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(4, pb_buffer_size(&buffer));

  /* Assert same contents but different location */
  fail_if(memcmp("DATA", pb_buffer_data(&buffer), size - 5));
  ck_assert_ptr_ne(data, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Clear an empty buffer.
 */
START_TEST(test_clear_empty) {
  pb_buffer_t buffer = pb_buffer_create_empty();

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Clear buffer: "" => "" */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_clear(&buffer, 0, 0));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Assert empty buffer */
  ck_assert_ptr_eq(NULL, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Clear a buffer entirely
 */
START_TEST(test_clear_entirely) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Clear buffer "SOME DATA" => "" */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_clear(&buffer, 0, size));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Assert empty buffer */
  ck_assert_ptr_eq(NULL, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Clear data from an invalid buffer.
 */
START_TEST(test_clear_invalid) {
  pb_buffer_t buffer = pb_buffer_create_invalid();

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(&buffer));

  /* Clear buffer */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_buffer_clear(&buffer, 0, 0));

  /* Assert buffer validity and error again */
  fail_if(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Assert empty buffer */
  ck_assert_ptr_eq(NULL, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Clear data from a buffer but specify an invalid offset (start > end).
 */
START_TEST(test_clear_invalid_offset) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Clear buffer */
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_buffer_clear(&buffer, size, 0));

  /* Assert buffer validity and error again */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Clear data from a buffer but specify an invalid range (end > size).
 */
START_TEST(test_clear_invalid_range) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Clear buffer */
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_buffer_clear(&buffer, 0, size + 1));

  /* Assert buffer validity and error again */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Clear data from a buffer for which allocation fails.
 */
START_TEST(test_clear_invalid_allocate) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_allocate_fail,
      .resize   = allocator_default.proc.resize,
      .free     = allocator_default.proc.free
    }
  };

  /* Create buffer */
  pb_buffer_t buffer =
    pb_buffer_create_with_allocator(&allocator, data, size);

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(&buffer));

  /* Clear buffer */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_buffer_clear(&buffer, 0, size));

  /* Assert buffer validity and error again */
  fail_if(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Assert empty buffer */
  ck_assert_ptr_eq(NULL, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Clear data from a buffer for which reallocation fails.
 */
START_TEST(test_clear_invalid_resize) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_resize_fail,
      .free     = allocator_default.proc.free
    }
  };

  /* Create buffer */
  pb_buffer_t buffer =
    pb_buffer_create_with_allocator(&allocator, data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Clear buffer */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_clear(&buffer, 4, size));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(4, pb_buffer_size(&buffer));

  /* Clear buffer entirely */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_clear(&buffer, 0, 4));

  /* Assert buffer size again */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Assert empty buffer */
  ck_assert_ptr_eq(NULL, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Clear a zero-copy buffer.
 */
START_TEST(test_clear_invalid_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create_zero_copy(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Clear buffer: "SOME DATA" => "" */
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_clear(&buffer, 0, size));

  /* Assert buffer validity and error again */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/* ----------------------------------------------------------------------------
 * Program
 * ------------------------------------------------------------------------- */

/*
 * Create a test suite for all registered test cases and run it.
 *
 * Tests must be run sequentially (in no-fork mode) or code coverage
 * cannot be determined properly.
 */
int
main(void) {
  void *suite = suite_create("protobluff/message/buffer"),
       *tcase = NULL;

  /* Add tests to test case "write" */
  tcase = tcase_create("write");
  tcase_add_test(tcase, test_write);
  tcase_add_test(tcase, test_write_empty);
  tcase_add_test(tcase, test_write_zero_copy);
  tcase_add_test(tcase, test_write_prepend);
  tcase_add_test(tcase, test_write_append);
  tcase_add_test(tcase, test_write_incremental);
  tcase_add_test(tcase, test_write_invalid);
  tcase_add_test(tcase, test_write_invalid_offset);
  tcase_add_test(tcase, test_write_invalid_range);
  tcase_add_test(tcase, test_write_invalid_allocate);
  tcase_add_test(tcase, test_write_invalid_resize);
  tcase_add_test(tcase, test_write_invalid_zero_copy);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "clear" */
  tcase = tcase_create("clear");
  tcase_add_test(tcase, test_clear);
  tcase_add_test(tcase, test_clear_empty);
  tcase_add_test(tcase, test_clear_entirely);
  tcase_add_test(tcase, test_clear_invalid);
  tcase_add_test(tcase, test_clear_invalid_offset);
  tcase_add_test(tcase, test_clear_invalid_range);
  tcase_add_test(tcase, test_clear_invalid_allocate);
  tcase_add_test(tcase, test_clear_invalid_resize);
  tcase_add_test(tcase, test_clear_invalid_zero_copy);
  suite_add_tcase(suite, tcase);

  /* Create a test suite runner in no-fork mode */
  void *runner = srunner_create(suite);
  srunner_set_fork_status(runner, CK_NOFORK);

  /* Execute test suite runner */
  srunner_run_all(runner, CK_NORMAL);
  int failed = srunner_ntests_failed(runner);
  srunner_free(runner);

  /* Exit with status code */
  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
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

#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lib/allocator.h"
#include "lib/binary.h"
#include "lib/common.h"

/* ----------------------------------------------------------------------------
 * Allocator callback overrides
 * ------------------------------------------------------------------------- */

/*
 * Allocation that will always fail.
 */
static void *
allocator_alloc_fail(void *data, size_t size) {
  assert(!data && size);
  return NULL;
}

/*
 * Reallocation that will always fail.
 */
static void *
allocator_realloc_fail(void *data, void *block, size_t size) {
  assert(!data && size);
  return NULL;
}

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a binary.
 */
START_TEST(test_create) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_binary_data(&binary), size));
  ck_assert_ptr_ne(data, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create an empty binary.
 */
START_TEST(test_create_empty) {
  pb_binary_t binary = pb_binary_create_empty();

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create an empty binary for which allocation failed.
 */
START_TEST(test_create_empty_fail_alloc) {
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_alloc_fail,
      .realloc = allocator_default.proc.realloc,
      .free    = allocator_default.proc.free
    }
  };

  /* Create binary */
  pb_binary_t binary =
    pb_binary_create_empty_with_allocator(&allocator);

  /* Assert binary validity and error */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a zero-copy binary.
 */
START_TEST(test_create_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create_zero_copy(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Assert same contents and location */
  fail_if(memcmp(data, pb_binary_data(&binary), size));
  ck_assert_ptr_eq(data, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create an invalid binary.
 */
START_TEST(test_create_invalid) {
  pb_binary_t binary = pb_binary_create_invalid();

  /* Assert binary validity and error */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a binary for which allocation failed.
 */
START_TEST(test_create_fail_alloc) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_alloc_fail,
      .realloc = allocator_default.proc.realloc,
      .free    = allocator_default.proc.free
    }
  };

  /* Create binary */
  pb_binary_t binary =
    pb_binary_create_with_allocator(&allocator, data, size);

  /* Assert binary validity and error */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data to a binary.
 */
START_TEST(test_write) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Update binary: "SOME DATA" => "MORE DATA THAN ..." */
  uint8_t new_data[] = "MORE DATA THAN THIS BINARY CURRENTLY HOLDS";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_write(&binary, 0, size, new_data, 42));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(42, pb_binary_size(&binary));

  /* Assert same contents but different location */
  fail_if(memcmp(new_data, pb_binary_data(&binary), 42));
  ck_assert_ptr_ne(new_data, pb_binary_data(&binary));
  ck_assert_ptr_ne(data,     pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data to an empty binary.
 */
START_TEST(test_write_empty) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create_empty();

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Update binary: "" => "SOME DATA" */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_write(&binary, 0, 0, data, size));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_binary_data(&binary), size));
  ck_assert_ptr_ne(data, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data of the same length to a zero-copy binary.
 */
START_TEST(test_write_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create_zero_copy(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Update binary: "SOME DATA" => "MORE DATA" */
  uint8_t new_data[] = "MORE DATA";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_write(&binary, 0, size, new_data, size));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Assert same contents but different location */
  fail_if(memcmp(new_data, pb_binary_data(&binary), size));
  ck_assert_ptr_ne(new_data, pb_binary_data(&binary));
  ck_assert_ptr_eq(data,     pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Prepend data to a binary.
 */
START_TEST(test_write_prepend) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Update binary: "SOME DATA" => "JUST SOME DATA" */
  uint8_t new_data[] = "JUST ";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_write(&binary, 0, 0, new_data, 5));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(14, pb_binary_size(&binary));

  /* Assert new contents and different location */
  fail_if(memcmp("JUST SOME DATA",
    pb_binary_data(&binary), size + 5));
  ck_assert_ptr_ne(new_data, pb_binary_data(&binary));
  ck_assert_ptr_ne(data,     pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Append data to a binary.
 */
START_TEST(test_write_append) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Update binary: "SOME DATA" => "SOME DATA IS AWESOME" */
  uint8_t new_data[] = " IS AWESOME";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_write(&binary, size, size, new_data, 11));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size + 11, pb_binary_size(&binary));

  /* Assert new contents and different location */
  fail_if(memcmp("SOME DATA IS AWESOME",
    pb_binary_data(&binary), size + 11));
  ck_assert_ptr_ne(new_data, pb_binary_data(&binary));
  ck_assert_ptr_ne(data,     pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data to a binary incrementally to test versioning.
 */
START_TEST(test_write_incremental) {
  pb_binary_t binary = pb_binary_create_empty();

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Iteratively grow binary: "" => "HELP I'M TRAPPED ..." */
  uint8_t data[] = "HELP I'M TRAPPED IN A UNIVERSE FACTORY";
  for (size_t s = 1; s < 38; s++) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_write(&binary, 0,
      pb_binary_size(&binary), data, s));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(s, pb_binary_size(&binary));

    /* Assert same contents but different location */
    fail_if(memcmp(data, pb_binary_data(&binary), s));
    ck_assert_ptr_ne(data, pb_binary_data(&binary));
  }

  /* Iteratively shrink binary: "HELP ... UNIVERSE FACTORY" => "FACTORY" */
  for (size_t s = 30; s > 0; s--) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_write(&binary, 0,
      pb_binary_size(&binary) - 8, data, s));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(s + 8, pb_binary_size(&binary));

    /* Assert same contents but different location */
    fail_if(memcmp(data, pb_binary_data(&binary), s));
    ck_assert_ptr_ne(data, pb_binary_data(&binary));
  }

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data to an invalid binary.
 */
START_TEST(test_write_invalid) {
  pb_binary_t binary = pb_binary_create_invalid();

  /* Assert binary validity and error */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Try to update binary */
  uint8_t data[] = "THIS WON'T WORK ANYWAY";
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_binary_write(&binary, 0, 0, data, 22));

  /* Assert binary validity and error again */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data to a binary but specify an invalid offset (start > end).
 */
START_TEST(test_write_invalid_offset) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Try to update binary */
  uint8_t new_data[] = "THIS WON'T WORK ANYWAY";
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_binary_write(&binary, size, 0, new_data, 22));

  /* Assert binary validity and error again */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_binary_data(&binary), size));
  ck_assert_ptr_ne(new_data, pb_binary_data(&binary));
  ck_assert_ptr_ne(data,     pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data to a binary but specify an invalid range (end > binary size).
 */
START_TEST(test_write_invalid_range) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Try to update binary */
  uint8_t new_data[] = "WHATEVER";
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_binary_write(&binary, 0, size + 1, new_data, 8));

  /* Assert binary validity and error again */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_binary_data(&binary), size));
  ck_assert_ptr_ne(new_data, pb_binary_data(&binary));
  ck_assert_ptr_ne(data,     pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data to a binary for which allocation failed.
 */
START_TEST(test_write_fail_alloc) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_alloc_fail,
      .realloc = allocator_default.proc.realloc,
      .free    = allocator_default.proc.free
    }
  };

  /* Create binary */
  pb_binary_t binary =
    pb_binary_create_with_allocator(&allocator, data, size);

  /* Assert binary validity and error */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Try to update binary */
  uint8_t new_data[] = "THIS WILL FAIL";
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_binary_write(&binary, 0, size, new_data, 14));

  /* Assert binary validity and error again */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data to a binary for which reallocation will always fail.
 */
START_TEST(test_write_fail_realloc) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_default.proc.alloc,
      .realloc = allocator_realloc_fail,
      .free    = allocator_default.proc.free
    }
  };

  /* Create binary */
  pb_binary_t binary =
    pb_binary_create_with_allocator(&allocator, data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Iteratively grow binary */
  uint8_t new_data[] = "THIS WILL FAIL";
  for (size_t s = 10; s < 14; s++) {
    ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_write(&binary, 0,
      pb_binary_size(&binary), new_data, s));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(size, pb_binary_size(&binary));
  }

  /* Assert binary validity and error again */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Iteratively shrink binary */
  for (size_t s = 8; s > 0; s--) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_write(&binary, 0,
      pb_binary_size(&binary), new_data, s));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(s, pb_binary_size(&binary));
  }

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write data to a zero-copy binary exceeding its current length.
 */
START_TEST(test_write_fail_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create_zero_copy(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Update binary: "SOME DATA" => "TOO MUCH DATA" */
  uint8_t new_data[] = "TOO MUCH DATA";
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_binary_write(&binary, 0, size, new_data, 13));

  /* Assert binary validity and error again */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Append data to a binary.
 */
START_TEST(test_append) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Update binary: "SOME DATA" => "SOME DATA IS AWESOME" */
  uint8_t new_data[] = " IS AWESOME";
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_append(&binary, new_data, 11));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size + 11, pb_binary_size(&binary));

  /* Assert new contents and different location */
  fail_if(memcmp("SOME DATA IS AWESOME",
    pb_binary_data(&binary), size + 11));
  ck_assert_ptr_ne(new_data, pb_binary_data(&binary));
  ck_assert_ptr_ne(data,     pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Append data to an empty binary.
 */
START_TEST(test_append_empty) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create_empty();

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Update binary: "" => "SOME DATA" */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_append(&binary, data, size));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Assert same contents but different location */
  fail_if(memcmp(data, pb_binary_data(&binary), size));
  ck_assert_ptr_ne(data, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Append data to an invalid binary.
 */
START_TEST(test_append_invalid) {
  pb_binary_t binary = pb_binary_create_invalid();

  /* Assert binary validity and error */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Try to update binary */
  uint8_t data[] = "THIS WON'T WORK ANYWAY";
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_binary_append(&binary, data, 22));

  /* Assert binary validity and error again */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Append data to a binary for which allocation failed.
 */
START_TEST(test_append_fail_alloc) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_alloc_fail,
      .realloc = allocator_default.proc.realloc,
      .free    = allocator_default.proc.free
    }
  };

  /* Create binary */
  pb_binary_t binary =
    pb_binary_create_with_allocator(&allocator, data, size);

  /* Assert binary validity and error */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Try to update binary */
  uint8_t new_data[] = "THIS WILL FAIL";
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_binary_append(&binary, new_data, 14));

  /* Assert binary validity and error again */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Append data to a binary for which reallocation will always fail.
 */
START_TEST(test_append_fail_realloc) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_default.proc.alloc,
      .realloc = allocator_realloc_fail,
      .free    = allocator_default.proc.free
    }
  };

  /* Create binary */
  pb_binary_t binary =
    pb_binary_create_with_allocator(&allocator, data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Iteratively grow binary */
  uint8_t new_data[] = "THIS WILL FAIL";
  for (size_t s = 10; s < 14; s++) {
    ck_assert_uint_eq(PB_ERROR_ALLOC,
      pb_binary_append(&binary, new_data, s));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(size, pb_binary_size(&binary));
  }

  /* Assert binary validity and error again */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear data from a binary.
 */
START_TEST(test_clear) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Clear binary: "MORE DATA" => "DATA" */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_clear(&binary, 0, 5));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(4, pb_binary_size(&binary));

  /* Assert same contents but different location */
  fail_if(memcmp("DATA", pb_binary_data(&binary), size - 5));
  ck_assert_ptr_ne(data, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear an empty binary.
 */
START_TEST(test_clear_empty) {
  pb_binary_t binary = pb_binary_create_empty();

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Clear binary: "" => "" */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_clear(&binary, 0, 0));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear a binary entirely
 */
START_TEST(test_clear_entirely) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Clear binary "MORE DATA" => "" */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_clear(&binary, 0, size));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear data from an invalid binary.
 */
START_TEST(test_clear_invalid) {
  pb_binary_t binary = pb_binary_create_invalid();

  /* Assert binary validity and error */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Clear binary */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_binary_clear(&binary, 0, 0));

  /* Assert binary validity and error again */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear data from a binary but specify an invalid offset (start > end).
 */
START_TEST(test_clear_invalid_offset) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Clear binary */
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_binary_clear(&binary, size, 0));

  /* Assert binary validity and error again */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear data from a binary but specify an invalid range (end > binary size).
 */
START_TEST(test_clear_invalid_range) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Clear binary */
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_binary_clear(&binary, 0, size + 1));

  /* Assert binary validity and error again */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear data from a binary for which allocation failed.
 */
START_TEST(test_clear_fail_alloc) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_alloc_fail,
      .realloc = allocator_default.proc.realloc,
      .free    = allocator_default.proc.free
    }
  };

  /* Create binary */
  pb_binary_t binary =
    pb_binary_create_with_allocator(&allocator, data, size);

  /* Assert binary validity and error */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Clear binary */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_binary_clear(&binary, 0, size));

  /* Assert binary validity and error again */
  fail_if(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_error(&binary));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear data from a binary for which reallocation will always fail.
 */
START_TEST(test_clear_fail_realloc) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_default.proc.alloc,
      .realloc = allocator_realloc_fail,
      .free    = allocator_default.proc.free
    }
  };

  /* Create binary */
  pb_binary_t binary =
    pb_binary_create_with_allocator(&allocator, data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Clear binary */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_clear(&binary, 4, size));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(4, pb_binary_size(&binary));

  /* Clear binary entirely */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_clear(&binary, 0, 4));

  /* Assert binary size again */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Assert empty binary */
  ck_assert_ptr_eq(NULL, pb_binary_data(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear a zero-copy binary.
 */
START_TEST(test_clear_fail_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create binary */
  pb_binary_t binary = pb_binary_create_zero_copy(data, size);

  /* Assert binary validity and error */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Clear binary: "MORE DATA" => "" */
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_binary_clear(&binary, 0, size));

  /* Assert binary validity and error again */
  fail_unless(pb_binary_valid(&binary));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_error(&binary));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(size, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_binary_destroy(&binary);
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
  void *suite = suite_create("protobluff/binary"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_empty);
  tcase_add_test(tcase, test_create_empty_fail_alloc);
  tcase_add_test(tcase, test_create_zero_copy);
  tcase_add_test(tcase, test_create_invalid);
  tcase_add_test(tcase, test_create_fail_alloc);
  suite_add_tcase(suite, tcase);

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
  tcase_add_test(tcase, test_write_fail_alloc);
  tcase_add_test(tcase, test_write_fail_realloc);
  tcase_add_test(tcase, test_write_fail_zero_copy);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "append" */
  tcase = tcase_create("append");
  tcase_add_test(tcase, test_append);
  tcase_add_test(tcase, test_append_empty);
  tcase_add_test(tcase, test_append_invalid);
  tcase_add_test(tcase, test_append_fail_alloc);
  tcase_add_test(tcase, test_append_fail_realloc);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "clear" */
  tcase = tcase_create("clear");
  tcase_add_test(tcase, test_clear);
  tcase_add_test(tcase, test_clear_empty);
  tcase_add_test(tcase, test_clear_entirely);
  tcase_add_test(tcase, test_clear_invalid);
  tcase_add_test(tcase, test_clear_invalid_offset);
  tcase_add_test(tcase, test_clear_invalid_range);
  tcase_add_test(tcase, test_clear_fail_alloc);
  tcase_add_test(tcase, test_clear_fail_realloc);
  tcase_add_test(tcase, test_clear_fail_zero_copy);
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
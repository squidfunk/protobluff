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
#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "core/allocator.h"
#include "core/buffer.h"
#include "core/common.h"

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
 * Create a buffer.
 */
START_TEST(test_create) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

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
 * Create an empty buffer.
 */
START_TEST(test_create_empty) {
  pb_buffer_t buffer = pb_buffer_create_empty();

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Assert empty buffer */
  ck_assert_ptr_eq(NULL, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST


/*
 * Create a zero-copy buffer.
 */
START_TEST(test_create_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create_zero_copy(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(size, pb_buffer_size(&buffer));

  /* Assert same contents and location */
  fail_if(memcmp(data, pb_buffer_data(&buffer), size));
  ck_assert_ptr_eq(data, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Create an invalid buffer.
 */
START_TEST(test_create_invalid) {
  pb_buffer_t buffer = pb_buffer_create_invalid();

  /* Assert buffer validity and error */
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
 * Create a buffer for which allocation fails.
 */
START_TEST(test_create_invalid_allocate) {
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

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Assert empty buffer */
  ck_assert_ptr_eq(NULL, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Grow a buffer and return a pointer to the newly allocated space.
 */
START_TEST(test_grow) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Grow buffer */
  uint8_t *new_data = pb_buffer_grow(&buffer, 7);
  ck_assert_ptr_ne(NULL, new_data);

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(16, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Grow an empty buffer and return a pointer to the newly allocated space.
 */
START_TEST(test_grow_empty) {
  pb_buffer_t buffer = pb_buffer_create_empty();

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Grow buffer */
  uint8_t *new_data = pb_buffer_grow(&buffer, 16);
  ck_assert_ptr_ne(NULL, new_data);

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(16, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Grow a zero-copy buffer and expect a NULL pointer.
 */
START_TEST(test_grow_zero_copy) {
  uint8_t data[] = "SOME DATA";
  size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create_zero_copy(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Grow buffer */
  uint8_t *new_data = pb_buffer_grow(&buffer, 16);
  ck_assert_ptr_eq(NULL, new_data);

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(9, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Grow an invalid buffer and expect a NULL pointer.
 */
START_TEST(test_grow_invalid) {
  pb_buffer_t buffer = pb_buffer_create_invalid();

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(&buffer));

  /* Grow buffer */
  uint8_t *new_data = pb_buffer_grow(&buffer, 16);
  ck_assert_ptr_eq(NULL, new_data);

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Grow a buffer for which allocation fails and expect a NULL pointer.
 */
START_TEST(test_grow_invalid_allocate) {
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

  /* Grow buffer */
  uint8_t *new_data = pb_buffer_grow(&buffer, 16);
  ck_assert_ptr_eq(NULL, new_data);

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(0, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Grow a buffer for which reallocation fails and expect a NULL pointer.
 */
START_TEST(test_grow_invalid_resize) {
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

  /* Grow buffer */
  uint8_t *new_data = pb_buffer_grow(&buffer, 16);
  ck_assert_ptr_eq(NULL, new_data);

  /* Assert buffer size */
  fail_if(pb_buffer_empty(&buffer));
  ck_assert_uint_eq(9, pb_buffer_size(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Retrieve the raw data of a buffer.
 */
START_TEST(test_data) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert same contents and location */
  fail_if(memcmp(data, pb_buffer_data(&buffer), size));
  ck_assert_ptr_ne(data, pb_buffer_data(&buffer));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Retrieve the raw data of a buffer from a given offset.
 */
START_TEST(test_data_from) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert same contents and location */
  fail_if(memcmp(&(data[5]), pb_buffer_data_from(&buffer, 5), size - 5));
  ck_assert_ptr_ne(&(data[5]), pb_buffer_data_from(&buffer, 5));

  /* Free all allocated memory */
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Retrieve the raw data of a buffer at a given offset.
 */
START_TEST(test_data_at) {
  const uint8_t data[] = "SOME DATA";
  const size_t  size   = 9;

  /* Create buffer */
  pb_buffer_t buffer = pb_buffer_create(data, size);

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(&buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(&buffer));

  /* Assert buffer contents */
  ck_assert_uint_eq(data[5], pb_buffer_data_at(&buffer, 5));

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
  void *suite = suite_create("protobluff/core/buffer"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_empty);
  tcase_add_test(tcase, test_create_zero_copy);
  tcase_add_test(tcase, test_create_invalid);
  tcase_add_test(tcase, test_create_invalid_allocate);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "grow" */
  tcase = tcase_create("grow");
  tcase_add_test(tcase, test_grow);
  tcase_add_test(tcase, test_grow_empty);
  tcase_add_test(tcase, test_grow_zero_copy);
  tcase_add_test(tcase, test_grow_invalid);
  tcase_add_test(tcase, test_grow_invalid_allocate);
  tcase_add_test(tcase, test_grow_invalid_resize);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "data" */
  tcase = tcase_create("data");
  tcase_add_test(tcase, test_data);
  tcase_add_test(tcase, test_data_from);
  tcase_add_test(tcase, test_data_at);
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

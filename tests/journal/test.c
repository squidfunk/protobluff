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

#include "lib/allocator.h"
#include "lib/common.h"
#include "lib/journal.h"

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
 * Create a journal to keep track of changes on a binary.
 */
START_TEST(test_create) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create an invalid journal.
 */
START_TEST(test_create_invalid) {
  pb_journal_t journal = pb_journal_create_invalid();

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a journal for which allocation failed.
 */
START_TEST(test_create_fail_alloc) {
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_alloc_fail,
      .realloc = allocator_default.proc.realloc,
      .free    = allocator_default.proc.free
    }
  };

  /* Create journal */
  pb_journal_t journal = pb_journal_create_with_allocator(&allocator, 8);

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Add an entry to a journal.
 */
START_TEST(test_log) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Log some entries */
  for (size_t e = 0; e < 32; e++)
    ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, e, e, e + 2));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(32, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = {};
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(32,  version);
  ck_assert_uint_eq(560, offset.end);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Add an entry to a journal for which allocation failed.
 */
START_TEST(test_log_fail_alloc) {
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_alloc_fail,
      .realloc = allocator_default.proc.realloc,
      .free    = allocator_default.proc.free
    }
  };

  /* Create journal */
  pb_journal_t journal = pb_journal_create_with_allocator(&allocator, 8);

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Assert subsequent invalidation */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_journal_log(&journal, 0, 0, 2));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Add an entry to a journal for which reallocation will always fail.
 */
START_TEST(test_log_fail_realloc) {
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_default.proc.alloc,
      .realloc = allocator_realloc_fail,
      .free    = allocator_default.proc.free
    }
  };

  /* Create journal */
  pb_journal_t journal = pb_journal_create_with_allocator(&allocator, 1);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Assert correct behaviour upon re-allocation error */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 0, 0, 2));
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_journal_log(&journal, 0, 0, 2));

  /* Assert journal validity and error again */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = {};
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(2, offset.end);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Remove the latest entry from the journal.
 */
START_TEST(test_revert) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));

  /* Log some entries */
  for (size_t e = 0; e < 32; e++)
    ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, e, e, e + 2));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(32, pb_journal_size(&journal));

  /* Revert all but one entry */
  for (size_t e = 0; e < 31; e++)
    pb_journal_revert(&journal);

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = {};
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(2, offset.end);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to a journal.
 */
START_TEST(test_align) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error and log some entry */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 0, 0, 2));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = {};
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(0, offset.start);
  ck_assert_uint_eq(2, offset.end);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update on a preceding part.
 */
START_TEST(test_align_move) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error and log some entry */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 0, 0, 2));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 1, 3 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(3, offset.start);
  ck_assert_uint_eq(5, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update on a length-prefix.
 */
START_TEST(test_align_move_length) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error and log some entry */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 1, 2, 1));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 4, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(3, offset.start);
  ck_assert_uint_eq(5, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(-3, offset.diff.origin);
  ck_assert_int_eq(-3, offset.diff.tag);
  ck_assert_int_eq(-2, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update growing the current part.
 */
START_TEST(test_align_grow) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error and log some entry */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 2, 4, 4));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 4, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(2, offset.start);
  ck_assert_uint_eq(8, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(-2, offset.diff.origin);
  ck_assert_int_eq(-2, offset.diff.tag);
  ck_assert_int_eq(-1, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update shrinking the current part.
 */
START_TEST(test_align_shrink) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error and log some entry */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 2, 6, -2));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 6, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(2, offset.start);
  ck_assert_uint_eq(4, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(-2, offset.diff.origin);
  ck_assert_int_eq(-2, offset.diff.tag);
  ck_assert_int_eq(-1, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update clearing the whole part.
 */
START_TEST(test_align_clear) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error and log some entry */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 0, 4, -4));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 4, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(SIZE_MAX, version);
  ck_assert_uint_eq(0, offset.start);
  ck_assert_uint_eq(0, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update clearing the outer part.
 */
START_TEST(test_align_clear_outside) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error and log some entry */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 0, 8, -8));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 4, 6, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(SIZE_MAX, version);
  ck_assert_uint_eq(0, offset.start);
  ck_assert_uint_eq(0, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update happening after the current part.
 */
START_TEST(test_align_clear_after) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error and log some entry */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 4, 8, -4));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 2, 4, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(1, version);
  ck_assert_uint_eq(2, offset.start);
  ck_assert_uint_eq(4, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(-2, offset.diff.origin);
  ck_assert_int_eq(-2, offset.diff.tag);
  ck_assert_int_eq(-1, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to an update clearing the whole part.
 */
START_TEST(test_align_clear_alignment) {
  pb_journal_t journal = pb_journal_create(8);

  /* Assert journal validity and error and log some entry */
  fail_unless(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_error(&journal));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 2, 6, -4));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_journal_log(&journal, 0, 0,  1));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Perform alignment */
  pb_version_t version = 0; pb_offset_t offset = { 4, 6, { -2, -2, -1 } };
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_align(&journal, &version, &offset));

  /* Assert version and offset */
  ck_assert_uint_eq(SIZE_MAX, version);
  ck_assert_uint_eq(3, offset.start);
  ck_assert_uint_eq(3, offset.end);

  /* Assert diff offsets */
  ck_assert_int_eq(0, offset.diff.origin);
  ck_assert_int_eq(0, offset.diff.tag);
  ck_assert_int_eq(0, offset.diff.length);

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Align an offset according to a journal for which allocation failed.
 */
START_TEST(test_align_fail_alloc) {
  pb_allocator_t allocator = {
    .proc = {
      .alloc   = allocator_alloc_fail,
      .realloc = allocator_default.proc.realloc,
      .free    = allocator_default.proc.free
    }
  };

  /* Create journal */
  pb_journal_t journal = pb_journal_create_with_allocator(&allocator, 8);

  /* Assert journal validity and error */
  fail_if(pb_journal_valid(&journal));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_journal_error(&journal));

  /* Assert subsequent invalidation */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_journal_log(&journal, 0, 0, 2));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Assert subsequent invalidation */
  pb_version_t version = 0; pb_offset_t offset = {};
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_journal_align(&journal, &version, &offset));

  /* Free all allocated memory */
  pb_journal_destroy(&journal);
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
  void *suite = suite_create("protobluff/journal"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_invalid);
  tcase_add_test(tcase, test_create_fail_alloc);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "log" */
  tcase = tcase_create("log");
  tcase_add_test(tcase, test_log);
  tcase_add_test(tcase, test_log_fail_alloc);
  tcase_add_test(tcase, test_log_fail_realloc);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "revert" */
  tcase = tcase_create("revert");
  tcase_add_test(tcase, test_revert);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "align" */
  tcase = tcase_create("align");
  tcase_add_test(tcase, test_align);
  tcase_add_test(tcase, test_align_move);
  tcase_add_test(tcase, test_align_move_length);
  tcase_add_test(tcase, test_align_grow);
  tcase_add_test(tcase, test_align_shrink);
  tcase_add_test(tcase, test_align_clear);
  tcase_add_test(tcase, test_align_clear_outside);
  tcase_add_test(tcase, test_align_clear_after);
  tcase_add_test(tcase, test_align_clear_alignment);
  tcase_add_test(tcase, test_align_fail_alloc);
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
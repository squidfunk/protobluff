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
#include <stdlib.h>

#include "lib/allocator/chunk.h"
#include "lib/common.h"

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a chunk allocator.
 */
START_TEST(test_create) {
  pb_allocator_t allocator = pb_allocator_chunk_create();
  pb_allocator_chunk_destroy(&allocator);
} END_TEST

/*
 * Create a chunk allocator with a minimum capacity.
 */
START_TEST(test_create_with_capacity) {
  pb_allocator_t allocator = pb_allocator_chunk_create_with_capacity(128);
  pb_allocator_chunk_destroy(&allocator);
} END_TEST

/*
 * Allocate a memory block of given size.
 */
START_TEST(test_alloc) {
  pb_allocator_t allocator = pb_allocator_chunk_create();

  /* Allocate a block */
  void *block1 = pb_allocator_alloc(&allocator, 16);
  ck_assert_ptr_ne(NULL, block1);

  /* Allocate another block */
  void *block2 = pb_allocator_alloc(&allocator, 16);
  ck_assert_ptr_ne(NULL, block2);

  /* Assert different blocks */
  ck_assert_ptr_ne(block1, block2);

  /* Free all allocated memory */
  pb_allocator_chunk_destroy(&allocator);
} END_TEST

/*
 * Allocate a memory block of given size for an invalid allocator.
 */
START_TEST(test_alloc_invalid) {
  pb_allocator_t allocator = pb_allocator_chunk_create();
  pb_allocator_t allocator_invalid = {
    .proc = {
      .alloc   = allocator.proc.alloc,
      .realloc = allocator.proc.realloc,
      .free    = allocator.proc.free
    },
    .data = NULL
  };

  /* Try to allocate blocks */
  ck_assert_ptr_eq(NULL, pb_allocator_alloc(&allocator_invalid, 16));
  ck_assert_ptr_eq(NULL, pb_allocator_alloc(&allocator_invalid, 16));

  /* Free all allocated memory */
  pb_allocator_chunk_destroy(&allocator);
} END_TEST

/*
 * Change the size of a previously allocated memory block.
 */
START_TEST(test_realloc) {
  pb_allocator_t allocator = pb_allocator_chunk_create();

  /* Allocate a block */
  void *block = pb_allocator_realloc(&allocator, NULL, 16);
  ck_assert_ptr_ne(NULL, block);

  /* Reallocate the block */
  block = pb_allocator_realloc(&allocator, block, 128);
  ck_assert_ptr_ne(NULL, block);

  /* Free all allocated memory */
  pb_allocator_chunk_destroy(&allocator);
} END_TEST

/*
 * Change the size of a previously allocated memory block.
 */
START_TEST(test_realloc_over_capacity) {
  pb_allocator_t allocator = pb_allocator_chunk_create_with_capacity(16);

  /* Allocate a block */
  void *block1 = pb_allocator_alloc(&allocator, 16);
  ck_assert_ptr_ne(NULL, block1);

  /* Allocate another block */
  void *block2 = pb_allocator_realloc(&allocator, NULL, 16);
  ck_assert_ptr_ne(NULL, block2);

  /* Reallocate the second block */
  block2 = pb_allocator_realloc(&allocator, block2, 128);
  ck_assert_ptr_ne(NULL, block2);

  /* Free all allocated memory */
  pb_allocator_chunk_destroy(&allocator);
} END_TEST

/*
 * Change the size of a previously allocated block for an invalid allocator.
 */
START_TEST(test_realloc_invalid) {
  pb_allocator_t allocator = pb_allocator_chunk_create();
  pb_allocator_t allocator_invalid = {
    .proc = {
      .alloc   = allocator.proc.alloc,
      .realloc = allocator.proc.realloc,
      .free    = allocator.proc.free
    },
    .data = NULL
  };

  /* Allocate a block */
  void *block = pb_allocator_realloc(&allocator_invalid, NULL, 16);
  ck_assert_ptr_eq(NULL, block);

  /* Reallocate the block */
  block = pb_allocator_realloc(&allocator_invalid, block, 128);
  ck_assert_ptr_eq(NULL, block);

  /* Free all allocated memory */
  pb_allocator_chunk_destroy(&allocator);
} END_TEST

/*
 * Free an allocated memory block.
 */
START_TEST(test_free) {
  pb_allocator_t allocator = pb_allocator_chunk_create();

  /* Allocate a block */
  void *block1 = pb_allocator_alloc(&allocator, 16);
  ck_assert_ptr_ne(NULL, block1);

  /* Allocate another block */
  void *block2 = pb_allocator_realloc(&allocator, NULL, 16);
  ck_assert_ptr_ne(NULL, block2);

  /* Reallocate the second block */
  block2 = pb_allocator_realloc(&allocator, block2, 128);
  ck_assert_ptr_ne(NULL, block2);

  /* Free both blocks */
  pb_allocator_free(&allocator, block2);
  pb_allocator_free(&allocator, block1);

  /* Free all allocated memory */
  pb_allocator_chunk_destroy(&allocator);
} END_TEST

/*
 * Free an unordered allocated memory block.
 */
START_TEST(test_free_unordered) {
  pb_allocator_t allocator = pb_allocator_chunk_create();

  /* Allocate a block */
  void *block[10];
  for (size_t b = 0; b < 10; b++) {
    block[b] = pb_allocator_alloc(&allocator, 16);
    ck_assert_ptr_ne(NULL, block[b]);
  }

  /* Free blocks in "random" order */
  pb_allocator_free(&allocator, block[5]);
  pb_allocator_free(&allocator, block[1]);
  pb_allocator_free(&allocator, block[9]);
  pb_allocator_free(&allocator, block[0]);
  pb_allocator_free(&allocator, block[2]);
  pb_allocator_free(&allocator, block[3]);
  pb_allocator_free(&allocator, block[6]);
  pb_allocator_free(&allocator, block[4]);
  pb_allocator_free(&allocator, block[7]);
  pb_allocator_free(&allocator, block[8]);

  /* Free all allocated memory */
  pb_allocator_chunk_destroy(&allocator);
} END_TEST

/*
 * Free an allocated memory block for an invalid allocator.
 */
START_TEST(test_free_invalid) {
  pb_allocator_t allocator = pb_allocator_chunk_create();
  pb_allocator_t allocator_invalid = {
    .proc = {
      .alloc   = allocator.proc.alloc,
      .realloc = allocator.proc.realloc,
      .free    = allocator.proc.free
    },
    .data = NULL
  };

  /* Try to free a non-allocated block */
  char block[] = "DOESN'T MATTER";
  pb_allocator_free(&allocator_invalid, block);

  /* Free all allocated memory */
  pb_allocator_chunk_destroy(&allocator);
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
  void *suite = suite_create("protobluff/allocator/chunk"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_with_capacity);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "alloc" */
  tcase = tcase_create("alloc");
  tcase_add_test(tcase, test_alloc);
  tcase_add_test(tcase, test_alloc_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "realloc" */
  tcase = tcase_create("realloc");
  tcase_add_test(tcase, test_realloc);
  tcase_add_test(tcase, test_realloc_over_capacity);
  tcase_add_test(tcase, test_realloc_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "free" */
  tcase = tcase_create("free");
  tcase_add_test(tcase, test_free);
  tcase_add_test(tcase, test_free_unordered);
  tcase_add_test(tcase, test_free_invalid);
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
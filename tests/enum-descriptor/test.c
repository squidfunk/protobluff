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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <protobluff/descriptor.h>

#include "lib/enum/descriptor.h"

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

static pb_enum_descriptor_t
descriptor = { {
  (const pb_enum_descriptor_value_t []){
    { 0L, "V00" },
    { 1L, "V01" },
    { 2L, "V02" }
  }, 3 } };

static pb_enum_descriptor_t
descriptor_empty = {};

static pb_enum_descriptor_t
descriptor_scattered = { {
  (const pb_enum_descriptor_value_t []){
    { 2L, "V02" },
    { 8L, "V08" }
  }, 2 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a const-iterator over an enum descriptor.
 */
START_TEST(test_iterator) {
  pb_enum_descriptor_iter_t it =
    pb_enum_descriptor_iter_create(&descriptor);

  /* Assert enum descriptor size */
  fail_if(pb_enum_descriptor_empty(&descriptor));
  ck_assert_uint_eq(3, pb_enum_descriptor_size(&descriptor));

  /* Walk through enum descriptor values forwards */
  fail_unless(pb_enum_descriptor_iter_begin(&it));
  for (size_t v = 0; v < 3; v++, pb_enum_descriptor_iter_next(&it)) {
    const pb_enum_descriptor_value_t *descriptor =
      pb_enum_descriptor_iter_current(&it);
    ck_assert_uint_eq(v, pb_enum_descriptor_iter_pos(&it));

    /* Assemble value name */
    char name[5];
    snprintf(name, 5, "V%02d", pb_enum_descriptor_value_number(descriptor));

    /* Assert value number and name */
    ck_assert_uint_eq(v, pb_enum_descriptor_value_number(descriptor));
    fail_if(strcmp(name, pb_enum_descriptor_value_name(descriptor)));
  }

  /* Assert enum descriptor iterator validity */
  fail_if(pb_enum_descriptor_iter_next(&it));

  /* Walk through enum descriptor values backwards */
  fail_unless(pb_enum_descriptor_iter_end(&it));
  size_t v = 2;
  do {
    const pb_enum_descriptor_value_t *descriptor =
      pb_enum_descriptor_iter_current(&it);
    ck_assert_uint_eq(v, pb_enum_descriptor_iter_pos(&it));

    /* Assemble value name */
    char name[5];
    snprintf(name, 5, "V%02d", pb_enum_descriptor_value_number(descriptor));

    /* Assert value number and name */
    ck_assert_uint_eq(v, pb_enum_descriptor_value_number(descriptor));
    fail_if(strcmp(name, pb_enum_descriptor_value_name(descriptor)));
  } while (v--, pb_enum_descriptor_iter_prev(&it));

  /* Assert enum descriptor iterator validity again */
  fail_if(pb_enum_descriptor_iter_prev(&it));

  /* Free all allocated memory */
  pb_enum_descriptor_iter_destroy(&it);
} END_TEST

/*
 * Create a const-iterator over an empty enum descriptor.
 */
START_TEST(test_iterator_empty) {
  pb_enum_descriptor_iter_t it =
    pb_enum_descriptor_iter_create(&descriptor_empty);

  /* Assert enum descriptor size */
  fail_unless(pb_enum_descriptor_empty(&descriptor_empty));
  ck_assert_uint_eq(0, pb_enum_descriptor_size(&descriptor_empty));

  /* Assert failing forward- and backward iteration */
  fail_if(pb_enum_descriptor_iter_begin(&it));
  fail_if(pb_enum_descriptor_iter_end(&it));

  /* Free all allocated memory */
  pb_enum_descriptor_iter_destroy(&it);
} END_TEST

/*
 * Retrieve the value for a given number from an enum descriptor.
 */
START_TEST(test_value_by_number) {
  for (size_t v = 0; v < 3; v++)
    ck_assert_ptr_eq(&(descriptor.value.data[v]),
      pb_enum_descriptor_value_by_number(&descriptor, v));
} END_TEST

/*
 * Retrieve an absent value from an enum descriptor.
 */
START_TEST(test_value_by_number_absent) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&descriptor, 4));
} END_TEST

/*
 * Retrieve a value from an empty enum descriptor.
 */
START_TEST(test_value_by_number_empty) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&descriptor_empty, 1));
} END_TEST

/*
 * Retrieve the value for a given number from an enum descriptor.
 */
START_TEST(test_value_by_number_scattered) {
  ck_assert_ptr_eq(&(descriptor_scattered.value.data[0]),
    pb_enum_descriptor_value_by_number(&descriptor_scattered, 2));
  ck_assert_ptr_eq(&(descriptor_scattered.value.data[1]),
    pb_enum_descriptor_value_by_number(&descriptor_scattered, 8));
} END_TEST

/*
 * Retrieve an absent value from an enum descriptor.
 */
START_TEST(test_value_by_number_scattered_absent) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&descriptor_scattered, 1));
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&descriptor_scattered, 3));
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&descriptor_scattered, 7));
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&descriptor_scattered, 9));
} END_TEST

/*
 * Retrieve the value for a given name from an enum descriptor.
 */
START_TEST(test_value_by_name) {
  for (size_t v = 0; v < 3; v++) {
    char name[5];
    snprintf(name, 5, "V%02zd", v);

    /* Assert value descriptor */
    ck_assert_ptr_eq(&(descriptor.value.data[v]),
      pb_enum_descriptor_value_by_name(&descriptor, name));
  }
} END_TEST

/*
 * Retrieve an absent value from an enum descriptor.
 */
START_TEST(test_value_by_name_absent) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_name(&descriptor, "V03"));
} END_TEST

/*
 * Retrieve a value from an empty enum descriptor.
 */
START_TEST(test_value_by_name_empty) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_name(&descriptor_empty, "V00"));
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
  void *suite = suite_create("protobluff/enum/descriptor"),
       *tcase = NULL;

  /* Add tests to test case "iterator" */
  tcase = tcase_create("iterator");
  tcase_add_test(tcase, test_iterator);
  tcase_add_test(tcase, test_iterator_empty);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "value-by-number" */
  tcase = tcase_create("value-by-number");
  tcase_add_test(tcase, test_value_by_number);
  tcase_add_test(tcase, test_value_by_number_absent);
  tcase_add_test(tcase, test_value_by_number_empty);
  tcase_add_test(tcase, test_value_by_number_scattered);
  tcase_add_test(tcase, test_value_by_number_scattered_absent);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "value-by-name" */
  tcase = tcase_create("value-by-name");
  tcase_add_test(tcase, test_value_by_name);
  tcase_add_test(tcase, test_value_by_name_absent);
  tcase_add_test(tcase, test_value_by_name_empty);
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
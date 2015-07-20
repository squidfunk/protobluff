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

#include "lib/field/descriptor.h"
#include "lib/message/descriptor.h"

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

static const pb_message_descriptor_t
descriptor = { {
  (const pb_field_descriptor_t []){
    {  1, "F01", UINT32,  OPTIONAL },
    {  2, "F02", UINT64,  OPTIONAL },
    {  3, "F03", SINT32,  OPTIONAL },
    {  4, "F04", SINT64,  OPTIONAL },
    {  5, "F05", BOOL,    OPTIONAL },
    {  6, "F06", FLOAT,   OPTIONAL },
    {  7, "F07", DOUBLE,  OPTIONAL },
    {  8, "F08", STRING,  OPTIONAL },
    {  9, "F09", BYTES,   OPTIONAL },
    { 10, "F10", UINT32,  OPTIONAL },
    { 11, "F11", MESSAGE, OPTIONAL, &descriptor },
    { 12, "F12", MESSAGE, REPEATED, &descriptor }
  }, 12 } };

/* ------------------------------------------------------------------------- */

static const pb_message_descriptor_t
descriptor_empty = {};

/* ------------------------------------------------------------------------- */

static const pb_message_descriptor_t
descriptor_scattered = { {
  (const pb_field_descriptor_t []){
    {  2, "F02", UINT64,  OPTIONAL },
    {  8, "F08", STRING,  OPTIONAL }
  }, 2 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a const-iterator over a message descriptor.
 */
START_TEST(test_iterator) {
  pb_message_descriptor_iter_t it =
    pb_message_descriptor_iter_create(&descriptor);

  /* Assert message descriptor size */
  fail_if(pb_message_descriptor_empty(&descriptor));
  ck_assert_uint_eq(12, pb_message_descriptor_size(&descriptor));

  /* Walk through message descriptor fields forwards */
  fail_unless(pb_message_descriptor_iter_begin(&it));
  for (size_t f = 1; f <= 12; f++, pb_message_descriptor_iter_next(&it)) {
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_iter_current(&it);
    ck_assert_uint_eq(f - 1, pb_message_descriptor_iter_pos(&it));

    /* Assemble field name */
    char name[5];
    snprintf(name, 5, "F%02d", pb_field_descriptor_tag(descriptor));

    /* Assert field name and tag */
    ck_assert_uint_eq(f, pb_field_descriptor_tag(descriptor));
    fail_if(strcmp(name, pb_field_descriptor_name(descriptor)));
  }

  /* Assert message descriptor iterator validity */
  fail_if(pb_message_descriptor_iter_next(&it));

  /* Walk through message descriptor fields backwards */
  fail_unless(pb_message_descriptor_iter_end(&it));
  for (size_t f = 12; f >= 1; f--, pb_message_descriptor_iter_prev(&it)) {
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_iter_current(&it);
    ck_assert_uint_eq(f - 1, pb_message_descriptor_iter_pos(&it));

    /* Assemble field name */
    char name[5];
    snprintf(name, 5, "F%02d", pb_field_descriptor_tag(descriptor));

    /* Assert field name and tag */
    ck_assert_uint_eq(f, pb_field_descriptor_tag(descriptor));
    fail_if(strcmp(name, pb_field_descriptor_name(descriptor)));
  }

  /* Assert message descriptor iterator validity again */
  fail_if(pb_message_descriptor_iter_prev(&it));

  /* Free all allocated memory */
  pb_message_descriptor_iter_destroy(&it);
} END_TEST

/*
 * Create a const-iterator over an empty message descriptor.
 */
START_TEST(test_iterator_empty) {
  pb_message_descriptor_iter_t it =
    pb_message_descriptor_iter_create(&descriptor_empty);

  /* Assert message descriptor size */
  fail_unless(pb_message_descriptor_empty(&descriptor_empty));
  ck_assert_uint_eq(0, pb_message_descriptor_size(&descriptor_empty));

  /* Assert failing forward- and backward iteration */
  fail_if(pb_message_descriptor_iter_begin(&it));
  fail_if(pb_message_descriptor_iter_end(&it));

  /* Free all allocated memory */
  pb_message_descriptor_iter_destroy(&it);
} END_TEST

/*
 * Retrieve the field descriptor for a given tag from a message descriptor.
 */
START_TEST(test_field_by_tag) {
  for (size_t f = 1; f <= 12; f++)
    ck_assert_ptr_eq(&(descriptor.field.data[f - 1]),
      pb_message_descriptor_field_by_tag(&descriptor, f));
} END_TEST

/*
 * Retrieve an absent field descriptor from a message descriptor.
 */
START_TEST(test_field_by_tag_absent) {
  ck_assert_ptr_eq(NULL,
    pb_message_descriptor_field_by_tag(&descriptor, 13));
} END_TEST

/*
 * Retrieve a field descriptor from an empty message descriptor.
 */
START_TEST(test_field_by_tag_empty) {
  ck_assert_ptr_eq(NULL,
    pb_message_descriptor_field_by_tag(&descriptor_empty, 1));
} END_TEST

/*
 * Retrieve the field descriptor for a given tag from a message descriptor.
 */
START_TEST(test_field_by_tag_scattered) {
  ck_assert_ptr_eq(&(descriptor_scattered.field.data[0]),
    pb_message_descriptor_field_by_tag(&descriptor_scattered, 2));
  ck_assert_ptr_eq(&(descriptor_scattered.field.data[1]),
    pb_message_descriptor_field_by_tag(&descriptor_scattered, 8));
} END_TEST

/*
 * Retrieve an absent field descriptor from a message descriptor.
 */
START_TEST(test_field_by_tag_scattered_absent) {
  ck_assert_ptr_eq(NULL,
    pb_message_descriptor_field_by_tag(&descriptor_scattered, 1));
  ck_assert_ptr_eq(NULL,
    pb_message_descriptor_field_by_tag(&descriptor_scattered, 3));
  ck_assert_ptr_eq(NULL,
    pb_message_descriptor_field_by_tag(&descriptor_scattered, 7));
  ck_assert_ptr_eq(NULL,
    pb_message_descriptor_field_by_tag(&descriptor_scattered, 9));
} END_TEST

/*
 * Retrieve the field descriptor for a given name from a message descriptor.
 */
START_TEST(test_field_by_name) {
  for (size_t f = 1; f <= 12; f++) {
    char name[5];
    snprintf(name, 5, "F%02zd", f);

    /* Assert field descriptor */
    ck_assert_ptr_eq(&(descriptor.field.data[f - 1]),
      pb_message_descriptor_field_by_name(&descriptor, name));
  }
} END_TEST

/*
 * Retrieve an absent field descriptor from a message descriptor.
 */
START_TEST(test_field_by_name_absent) {
  ck_assert_ptr_eq(NULL,
    pb_message_descriptor_field_by_name(&descriptor, "F00"));
} END_TEST

/*
 * Retrieve a field descriptor from an empty message descriptor.
 */
START_TEST(test_field_by_name_empty) {
  ck_assert_ptr_eq(NULL,
    pb_message_descriptor_field_by_name(&descriptor_empty, "F00"));
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
  void *suite = suite_create("protobluff/message/descriptor"),
       *tcase = NULL;

  /* Add tests to test case "iterator" */
  tcase = tcase_create("iterator");
  tcase_add_test(tcase, test_iterator);
  tcase_add_test(tcase, test_iterator_empty);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "field-by-tag" */
  tcase = tcase_create("field-by-tag");
  tcase_add_test(tcase, test_field_by_tag);
  tcase_add_test(tcase, test_field_by_tag_absent);
  tcase_add_test(tcase, test_field_by_tag_empty);
  tcase_add_test(tcase, test_field_by_tag_scattered);
  tcase_add_test(tcase, test_field_by_tag_scattered_absent);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "field-by-name" */
  tcase = tcase_create("field-by-name");
  tcase_add_test(tcase, test_field_by_name);
  tcase_add_test(tcase, test_field_by_name_absent);
  tcase_add_test(tcase, test_field_by_name_empty);
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
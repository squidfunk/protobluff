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

#include <protobluff/descriptor.h>

#include "core/common.h"
#include "core/descriptor.h"

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

/* Descriptor */
static pb_descriptor_t
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

/* Empty descriptor */
static pb_descriptor_t
descriptor_empty = {};

/* Scattered descriptor */
static pb_descriptor_t
descriptor_scattered = { {
  (const pb_field_descriptor_t []){
    {  2, "F02", UINT64,  OPTIONAL },
    {  8, "F08", STRING,  OPTIONAL }
  }, 2 } };

/* Extension descriptor */
static pb_descriptor_t
descriptor_extension = { {
  (const pb_field_descriptor_t []){
    { 20, "F20", UINT32,  OPTIONAL },
    { 21, "F21", UINT64,  OPTIONAL }
  }, 2, } };

/* Nested extension descriptor */
static pb_descriptor_t
descriptor_extension_nested = { {
  (const pb_field_descriptor_t []){
    { 30, "F30", MESSAGE,  OPTIONAL, &descriptor }
  }, 1, } };

/* ------------------------------------------------------------------------- */

/* Enum descriptor */
static pb_enum_descriptor_t
enum_descriptor = { {
  (const pb_enum_descriptor_value_t []){
    {  0, "V00" },
    {  1, "V01" },
    {  2, "V02" }
  }, 3 } };

/* Empty enum descriptor */
static pb_enum_descriptor_t
enum_descriptor_empty = {};

/* Scattered enum descriptor */
static pb_enum_descriptor_t
enum_descriptor_scattered = { {
  (const pb_enum_descriptor_value_t []){
    {  2, "V02" },
    {  8, "V08" }
  }, 2 } };

/* ----------------------------------------------------------------------------
 * Fixtures
 * ------------------------------------------------------------------------- */

/*!
 * Setup
 */
static void
setup() {}

/*!
 * Teardown
 */
static void
teardown() {
  pb_descriptor_reset(&descriptor);
  pb_descriptor_reset(&descriptor_extension);
}

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Retrieve the field descriptor for a given tag from a descriptor.
 */
START_TEST(test_field_by_tag) {
  for (size_t f = 1; f <= 12; f++)
    ck_assert_ptr_eq(&(descriptor.field.data[f - 1]),
      pb_descriptor_field_by_tag(&descriptor, f));
} END_TEST

/*
 * Retrieve the field descriptor for an absent tag from a descriptor.
 */
START_TEST(test_field_by_tag_absent) {
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_tag(&descriptor, 13));
} END_TEST

/*
 * Retrieve the field descriptor for a given tag from an empty descriptor.
 */
START_TEST(test_field_by_tag_empty) {
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_tag(&descriptor_empty, 1));
} END_TEST

/*
 * Retrieve the field descriptor for a given tag from a scattered descriptor.
 */
START_TEST(test_field_by_tag_scattered) {
  ck_assert_ptr_eq(&(descriptor_scattered.field.data[0]),
    pb_descriptor_field_by_tag(&descriptor_scattered, 2));
  ck_assert_ptr_eq(&(descriptor_scattered.field.data[1]),
    pb_descriptor_field_by_tag(&descriptor_scattered, 8));
} END_TEST

/*
 * Retrieve the field descriptor for an absent given tag from a descriptor.
 */
START_TEST(test_field_by_tag_scattered_absent) {
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_tag(&descriptor_scattered, 1));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_tag(&descriptor_scattered, 3));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_tag(&descriptor_scattered, 7));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_tag(&descriptor_scattered, 9));
} END_TEST

/*
 * Retrieve the field descriptor for a given tag from an extended descriptor.
 */
START_TEST(test_field_by_tag_extended) {
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_tag(&descriptor, 20));

  /* Extend descriptor and retrieve field */
  pb_descriptor_extend(&descriptor, &descriptor_extension);
  ck_assert_ptr_eq(&(descriptor_extension.field.data[0]),
    pb_descriptor_field_by_tag(&descriptor, 20));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_tag(&descriptor, 30));

  /* Extend descriptor and retrieve field again */
  pb_descriptor_extend(&descriptor, &descriptor_extension_nested);
  ck_assert_ptr_eq(&(descriptor_extension.field.data[0]),
    pb_descriptor_field_by_tag(&descriptor, 20));
  ck_assert_ptr_eq(&(descriptor_extension_nested.field.data[0]),
    pb_descriptor_field_by_tag(&descriptor, 30));
} END_TEST

/*
 * Register an extension for the given descriptor.
 */
START_TEST(test_extend) {
  ck_assert_ptr_eq(NULL, pb_descriptor_extension(&descriptor));

  /* Extend descriptor */
  pb_descriptor_extend(&descriptor, &descriptor_extension);
  ck_assert_ptr_eq(&descriptor_extension,
    pb_descriptor_extension(&descriptor));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_extension(&descriptor_extension));

  /* Extend descriptor with same extension */
  pb_descriptor_extend(&descriptor, &descriptor_extension);
  ck_assert_ptr_eq(&descriptor_extension,
    pb_descriptor_extension(&descriptor));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_extension(&descriptor_extension));

  /* Extend descriptor with another extension */
  pb_descriptor_extend(&descriptor, &descriptor_extension_nested);
  ck_assert_ptr_eq(&descriptor_extension,
    pb_descriptor_extension(&descriptor));
  ck_assert_ptr_eq(&descriptor_extension_nested,
    pb_descriptor_extension(&descriptor_extension));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_extension(&descriptor_extension_nested));
} END_TEST

/* ------------------------------------------------------------------------- */

/*
 * Retrieve the value for a given number from an enum descriptor.
 */
START_TEST(test_enum_value_by_number) {
  for (size_t v = 0; v < 3; v++)
    ck_assert_ptr_eq(&(enum_descriptor.value.data[v]),
      pb_enum_descriptor_value_by_number(&enum_descriptor, v));
} END_TEST

/*
 * Retrieve the value for an absent number from an enum descriptor.
 */
START_TEST(test_enum_value_by_number_absent) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&enum_descriptor, 4));
} END_TEST

/*
 * Retrieve the value for a given number from an empty enum descriptor.
 */
START_TEST(test_enum_value_by_number_empty) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&enum_descriptor_empty, 1));
} END_TEST

/*
 * Retrieve the value for a given number from a scattered enum descriptor.
 */
START_TEST(test_enum_value_by_number_scattered) {
  ck_assert_ptr_eq(&(enum_descriptor_scattered.value.data[0]),
    pb_enum_descriptor_value_by_number(&enum_descriptor_scattered, 2));
  ck_assert_ptr_eq(&(enum_descriptor_scattered.value.data[1]),
    pb_enum_descriptor_value_by_number(&enum_descriptor_scattered, 8));
} END_TEST

/*
 * Retrieve the value for an absent number from an enum descriptor.
 */
START_TEST(test_enum_value_by_number_scattered_absent) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&enum_descriptor_scattered, 1));
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&enum_descriptor_scattered, 3));
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&enum_descriptor_scattered, 7));
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_number(&enum_descriptor_scattered, 9));
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
  void *suite = suite_create("protobluff/core/descriptor"),
       *tcase = NULL;

  /* Add tests to test case "field_by_tag" */
  tcase = tcase_create("field_by_tag");
  tcase_add_checked_fixture(tcase, setup, teardown);
  tcase_add_test(tcase, test_field_by_tag);
  tcase_add_test(tcase, test_field_by_tag_absent);
  tcase_add_test(tcase, test_field_by_tag_empty);
  tcase_add_test(tcase, test_field_by_tag_scattered);
  tcase_add_test(tcase, test_field_by_tag_scattered_absent);
  tcase_add_test(tcase, test_field_by_tag_extended);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "extend" */
  tcase = tcase_create("extend");
  tcase_add_checked_fixture(tcase, setup, teardown);
  tcase_add_test(tcase, test_extend);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "enum/value_by_number" */
  tcase = tcase_create("enum/value_by_number");
  tcase_add_test(tcase, test_enum_value_by_number);
  tcase_add_test(tcase, test_enum_value_by_number_absent);
  tcase_add_test(tcase, test_enum_value_by_number_empty);
  tcase_add_test(tcase, test_enum_value_by_number_scattered);
  tcase_add_test(tcase, test_enum_value_by_number_scattered_absent);
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
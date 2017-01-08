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

#include <check.h>
#include <stdio.h>
#include <stdlib.h>

#include <protobluff/descriptor.h>

#include "core/common.h"
#include "util/descriptor.h"

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
    { 30, "F30", MESSAGE, OPTIONAL, &descriptor }
  }, 1, } };

/* ------------------------------------------------------------------------- */

/* Enum descriptor */
static pb_enum_descriptor_t
enum_descriptor = { {
  (const pb_enum_value_descriptor_t []){
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
  (const pb_enum_value_descriptor_t []){
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
 * Retrieve the field descriptor for a given name from a descriptor.
 */
START_TEST(test_field_by_name) {
  for (size_t f = 1; f <= 12; f++) {
    char name[5];
    snprintf(name, 5, "F%02zd", f);

    /* Assert field descriptor */
    ck_assert_ptr_eq(&(descriptor.field.data[f - 1]),
      pb_descriptor_field_by_name(&descriptor, name));
  }
} END_TEST

/*
 * Retrieve the field descriptor for an absent name from a descriptor.
 */
START_TEST(test_field_by_name_absent) {
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_name(&descriptor, "F00"));
} END_TEST

/*
 * Retrieve the field descriptor for a given name from an empty descriptor.
 */
START_TEST(test_field_by_name_empty) {
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_name(&descriptor_empty, "F00"));
} END_TEST

/*
 * Retrieve the field descriptor for a given name from a scattered descriptor.
 */
START_TEST(test_field_by_name_scattered) {
  ck_assert_ptr_eq(&(descriptor_scattered.field.data[0]),
    pb_descriptor_field_by_name(&descriptor_scattered, "F02"));
  ck_assert_ptr_eq(&(descriptor_scattered.field.data[1]),
    pb_descriptor_field_by_name(&descriptor_scattered, "F08"));
} END_TEST

/*
 * Retrieve the field descriptor for an absent name from a descriptor.
 */
START_TEST(test_field_by_name_scattered_absent) {
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_name(&descriptor_scattered, "F01"));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_name(&descriptor_scattered, "F03"));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_name(&descriptor_scattered, "F07"));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_name(&descriptor_scattered, "F09"));
} END_TEST

/*
 * Retrieve the field descriptor for a given name from an extended descriptor.
 */
START_TEST(test_field_by_name_extended) {
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_name(&descriptor, "F20"));

  /* Extend descriptor and retrieve field */
  pb_descriptor_extend(&descriptor, &descriptor_extension);
  ck_assert_ptr_eq(&(descriptor_extension.field.data[0]),
    pb_descriptor_field_by_name(&descriptor, "F20"));
  ck_assert_ptr_eq(NULL,
    pb_descriptor_field_by_name(&descriptor, "F30"));

  /* Extend descriptor and retrieve field again */
  pb_descriptor_extend(&descriptor, &descriptor_extension_nested);
  ck_assert_ptr_eq(&(descriptor_extension.field.data[0]),
    pb_descriptor_field_by_name(&descriptor, "F20"));
  ck_assert_ptr_eq(&(descriptor_extension_nested.field.data[0]),
    pb_descriptor_field_by_name(&descriptor, "F30"));
} END_TEST

/* ------------------------------------------------------------------------- */

/*
 * Retrieve the value for a given name from an enum descriptor.
 */
START_TEST(test_enum_value_by_name) {
  for (size_t v = 0; v < 3; v++) {
    char name[5];
    snprintf(name, 5, "V%02zd", v);

    /* Assert value descriptor */
    ck_assert_ptr_eq(&(enum_descriptor.value.data[v]),
      pb_enum_descriptor_value_by_name(&enum_descriptor, name));
  }
} END_TEST

/*
 * Retrieve the value for an absent name from an enum descriptor.
 */
START_TEST(test_enum_value_by_name_absent) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_name(&enum_descriptor, "V03"));
} END_TEST

/*
 * Retrieve the value for a given name from an empty enum descriptor.
 */
START_TEST(test_enum_value_by_name_empty) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_name(&enum_descriptor_empty, "V00"));
} END_TEST

/*
 * Retrieve the value for a given name from a scattered enum descriptor.
 */
START_TEST(test_enum_value_by_name_scattered) {
  ck_assert_ptr_eq(&(enum_descriptor_scattered.value.data[0]),
    pb_enum_descriptor_value_by_name(&enum_descriptor_scattered, "V02"));
  ck_assert_ptr_eq(&(enum_descriptor_scattered.value.data[1]),
    pb_enum_descriptor_value_by_name(&enum_descriptor_scattered, "V08"));
} END_TEST

/*
 * Retrieve the value for an absent name from a scattered enum descriptor.
 */
START_TEST(test_enum_value_by_name_scattered_absent) {
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_name(&enum_descriptor_scattered, "V01"));
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_name(&enum_descriptor_scattered, "V03"));
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_name(&enum_descriptor_scattered, "V07"));
  ck_assert_ptr_eq(NULL,
    pb_enum_descriptor_value_by_name(&enum_descriptor_scattered, "V09"));
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
  void *suite = suite_create("protobluff/util/descriptor"),
       *tcase = NULL;

  /* Add tests to test case "field_by_name" */
  tcase = tcase_create("field_by_name");
  tcase_add_checked_fixture(tcase, setup, teardown);
  tcase_add_test(tcase, test_field_by_name);
  tcase_add_test(tcase, test_field_by_name_absent);
  tcase_add_test(tcase, test_field_by_name_empty);
  tcase_add_test(tcase, test_field_by_name_scattered);
  tcase_add_test(tcase, test_field_by_name_scattered_absent);
  tcase_add_test(tcase, test_field_by_name_extended);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "enum_value_by_name" */
  tcase = tcase_create("enum_value_by_name");
  tcase_add_test(tcase, test_enum_value_by_name);
  tcase_add_test(tcase, test_enum_value_by_name_absent);
  tcase_add_test(tcase, test_enum_value_by_name_empty);
  tcase_add_test(tcase, test_enum_value_by_name_scattered);
  tcase_add_test(tcase, test_enum_value_by_name_scattered_absent);
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

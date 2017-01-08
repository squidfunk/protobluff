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
#include <string.h>

#include <protobluff/descriptor.h>

#include "core/buffer.h"
#include "core/common.h"
#include "core/descriptor.h"
#include "util/validator.h"

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

/* Descriptor */
static pb_descriptor_t
descriptor = { {
  (const pb_field_descriptor_t []){
    {  1, "F01", UINT32,  REQUIRED },
    {  2, "F02", UINT64,  REQUIRED },
    {  3, "F03", MESSAGE, OPTIONAL, &descriptor },
    {  4, "F04", MESSAGE, REPEATED, &descriptor }
  }, 4 } };

/* Empty descriptor */
static pb_descriptor_t
descriptor_empty = {};

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

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Validate a buffer.
 */
START_TEST(test_check) {
  const uint8_t data[] = { 8, 127, 16, 127 };
  const size_t  size   = 4;

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate an empty buffer against an empty descriptor.
 */
START_TEST(test_check_empty) {
  pb_buffer_t    buffer    = pb_buffer_create_empty();
  pb_validator_t validator = pb_validator_create(&descriptor_empty);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer against an empty descriptor.
 */
START_TEST(test_check_empty_descriptor) {
  const uint8_t data[] = { 8, 127, 16, 127 };
  const size_t  size   = 4;

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor_empty);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate an empty buffer missing required fields.
 */
START_TEST(test_check_empty_buffer) {
  pb_buffer_t    buffer    = pb_buffer_create_empty();
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_ABSENT,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer with a valid submessage.
 */
START_TEST(test_check_message) {
  const uint8_t data[] = { 8, 127, 16, 127, 34, 4, 8, 127, 16, 127 };
  const size_t  size   = 10;

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer with an empty submessage missing required fields.
 */
START_TEST(test_check_message_empty) {
  const uint8_t data[] = { 8, 127, 16, 127, 34, 0 };
  const size_t  size   = 6;

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_ABSENT,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer with an extension field.
 */
START_TEST(test_check_extension) {
  const uint8_t data[] = { 8, 127, 16, 127, 160, 1, 127 };
  const size_t  size   = 7;

  /* Initialize descriptor extension */
  pb_descriptor_extend(&descriptor, &descriptor_extension);

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_validator_check(&validator, &buffer));

  /* Reset descriptor extension */
  pb_descriptor_reset(&descriptor);

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer with a nested extension.
 */
START_TEST(test_check_extension_nested) {
  const uint8_t data[] = { 8, 127, 16, 127, 242, 1, 4, 8, 127, 16, 127 };
  const size_t  size   = 11;

  /* Initialize descriptor extension */
  pb_descriptor_extend(&descriptor, &descriptor_extension_nested);

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_validator_check(&validator, &buffer));

  /* Reset descriptor extension */
  pb_descriptor_reset(&descriptor);

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer with an empty nested extension.
 */
START_TEST(test_check_extension_nested_empty) {
  const uint8_t data[] = { 8, 127, 16, 127, 242, 1, 0 };
  const size_t  size   = 7;

  /* Initialize descriptor extension */
  pb_descriptor_extend(&descriptor, &descriptor_extension_nested);

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_ABSENT,
    pb_validator_check(&validator, &buffer));

  /* Reset descriptor extension */
  pb_descriptor_reset(&descriptor);

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer with multiple optional fields.
 */
START_TEST(test_check_multiple_optional) {
  const uint8_t data[] = {
    8, 127, 16, 127,
    26, 6, 8, 127, 8, 127, 16, 127,
    26, 6, 8, 127, 8, 127, 16, 127 };
  const size_t  size   = 20;

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer with multiple required fields.
 */
START_TEST(test_check_multiple_required) {
  const uint8_t data[] = { 8, 127, 8, 127, 16, 127, 16, 127 };
  const size_t  size   = 8;

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate an invalid buffer.
 */
START_TEST(test_check_invalid) {
  pb_buffer_t    buffer    = pb_buffer_create_invalid();
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_validator_check(&validator, &buffer));

    /* Free all allocated memory */
    pb_validator_destroy(&validator);
    pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer with an invalid tag.
 */
START_TEST(test_check_invalid_tag) {
  const uint8_t data[] = { 8, 127, 128 };
  const size_t  size   = 3;

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_VARINT,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Validate a buffer with an invalid length prefix.
 */
START_TEST(test_check_invalid_length) {
  const uint8_t data[] = { 8, 127, 16, 127, 26, 100 };
  const size_t  size   = 6;

  /* Create validator and buffer */
  pb_buffer_t    buffer    = pb_buffer_create(data, size);
  pb_validator_t validator = pb_validator_create(&descriptor);

  /* Check buffer */
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_validator_check(&validator, &buffer));

  /* Free all allocated memory */
  pb_validator_destroy(&validator);
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
  void *suite = suite_create("protobluff/util/validator"),
       *tcase = NULL;

  /* Add tests to test case "check" */
  tcase = tcase_create("check");
  tcase_add_test(tcase, test_check);
  tcase_add_test(tcase, test_check_empty);
  tcase_add_test(tcase, test_check_empty_descriptor);
  tcase_add_test(tcase, test_check_empty_buffer);
  tcase_add_test(tcase, test_check_message);
  tcase_add_test(tcase, test_check_message_empty);
  tcase_add_test(tcase, test_check_extension);
  tcase_add_test(tcase, test_check_extension_nested);
  tcase_add_test(tcase, test_check_extension_nested_empty);
  tcase_add_test(tcase, test_check_multiple_optional);
  tcase_add_test(tcase, test_check_multiple_required);
  tcase_add_test(tcase, test_check_invalid);
  tcase_add_test(tcase, test_check_invalid_tag);
  tcase_add_test(tcase, test_check_invalid_length);
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

/*
 * Copyright (c) 2013-2020 Martin Donath <martin.donath@squidfunk.com>
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

#include <protobluff/descriptor.h>

#include "core/descriptor.h"
#include "message/common.h"
#include "message/journal.h"
#include "message/message.h"
#include "message/oneof.h"

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

/* Descriptor (forward declaration) */
static pb_descriptor_t
descriptor;

/* Oneof descriptor */
static const pb_oneof_descriptor_t
oneof_descriptor = {
  &descriptor, {
    (const size_t []){
      1, 2, 3
    }, 3 } };

/* Descriptor */
static pb_descriptor_t
descriptor = { {
  (const pb_field_descriptor_t []){
    {  1, "F01", UINT32,  OPTIONAL },
    {  2, "F02", UINT32,  ONEOF, NULL, &oneof_descriptor },
    {  3, "F03", UINT32,  ONEOF, NULL, &oneof_descriptor },
    {  4, "F04", MESSAGE, ONEOF, &descriptor, &oneof_descriptor }
  }, 4 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a oneof from a descriptor and a message.
 */
START_TEST(test_create) {
  const uint8_t data[] = { 8, 127, 16, 127 };
  const size_t  size   = 4;

  /* Create journal, message and oneof */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a oneof from a descriptor and an empty message.
 */
START_TEST(test_create_message_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a oneof from a descriptor and a message.
 */
START_TEST(test_create_message_merged) {
  const uint8_t data[] = { 8, 127, 16, 127, 24, 127 };
  const size_t  size   = 6;

  /* Create journal, message and oneof */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a oneof from a descriptor and an invalid message.
 */
START_TEST(test_create_message_invalid) {
  pb_message_t message = pb_message_create_invalid();
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_if(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_oneof_error(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
} END_TEST

/*
 * Create an invalid oneof.
 */
START_TEST(test_create_invalid) {
  pb_oneof_t oneof = pb_oneof_create_invalid();

  /* Assert oneof validity and error */
  fail_if(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_oneof_error(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
} END_TEST

/*
 * Retrieve the active tag of a oneof.
 */
START_TEST(test_case) {
  const uint8_t data[] = { 8, 127, 16, 127 };
  const size_t  size   = 4;

  /* Create journal, message and oneof */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Assert active tag */
  ck_assert_uint_eq(2, pb_oneof_case(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Retrieve the active tag of a oneof on an empty message.
 */
START_TEST(test_case_message_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Assert active tag */
  ck_assert_uint_eq(0, pb_oneof_case(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Retrieve the active tag of a oneof on a merged message.
 */
START_TEST(test_case_message_merged) {
  const uint8_t data[] = { 8, 127, 16, 127, 24, 127 };
  const size_t  size   = 6;

  /* Create journal, message and oneof */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Assert active tag */
  ck_assert_uint_eq(3, pb_oneof_case(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Retrieve the active tag of a oneof on an invalid message.
 */
START_TEST(test_case_message_invalid) {
  pb_message_t message = pb_message_create_invalid();
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_if(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_oneof_error(&oneof));

  /* Assert active tag */
  ck_assert_uint_eq(0, pb_oneof_case(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
} END_TEST

/*
 * Retrieve the active tag of an invalid oneof.
 */
START_TEST(test_case_invalid) {
  pb_oneof_t oneof = pb_oneof_create_invalid();

  /* Assert oneof validity and error */
  fail_if(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_oneof_error(&oneof));

  /* Assert active tag */
  ck_assert_uint_eq(0, pb_oneof_case(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
} END_TEST

/*
 * Clear all members of a oneof.
 */
START_TEST(test_clear) {
  const uint8_t data[] = { 8, 127, 16, 127 };
  const size_t  size   = 4;

  /* Create journal, message and oneof */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Clear oneof */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_clear(&oneof));

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear all members of a oneof on an empty message.
 */
START_TEST(test_clear_message_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Clear oneof */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_clear(&oneof));

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear all members of a oneof on a merged message.
 */
START_TEST(test_clear_message_merged) {
  const uint8_t data[] = { 8, 127, 16, 127, 24, 127 };
  const size_t  size   = 6;

  /* Create journal, message and oneof */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Clear oneof */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_clear(&oneof));

  /* Assert oneof validity and error */
  fail_unless(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_error(&oneof));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear all members of a oneof on an invalid message.
 */
START_TEST(test_clear_message_invalid) {
  pb_message_t message = pb_message_create_invalid();
  pb_oneof_t   oneof   = pb_oneof_create(&oneof_descriptor, &message);

  /* Assert oneof validity and error */
  fail_if(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_oneof_error(&oneof));

  /* Clear oneof */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_clear(&oneof));

  /* Assert oneof validity and error */
  fail_if(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_oneof_error(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
  pb_message_destroy(&message);
} END_TEST

/*
 * Clear all members of an invalid oneof.
 */
START_TEST(test_clear_invalid) {
  pb_oneof_t oneof = pb_oneof_create_invalid();

  /* Assert oneof validity and error */
  fail_if(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_oneof_error(&oneof));

  /* Clear oneof */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_oneof_clear(&oneof));

  /* Assert oneof validity and error */
  fail_if(pb_oneof_valid(&oneof));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_oneof_error(&oneof));

  /* Free all allocated memory */
  pb_oneof_destroy(&oneof);
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
  void *suite = suite_create("protobluff/message/oneof"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_message_empty);
  tcase_add_test(tcase, test_create_message_merged);
  tcase_add_test(tcase, test_create_message_invalid);
  tcase_add_test(tcase, test_create_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "case" */
  tcase = tcase_create("case");
  tcase_add_test(tcase, test_case);
  tcase_add_test(tcase, test_case_message_empty);
  tcase_add_test(tcase, test_case_message_merged);
  tcase_add_test(tcase, test_case_message_invalid);
  tcase_add_test(tcase, test_case_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "clear" */
  tcase = tcase_create("clear");
  tcase_add_test(tcase, test_clear);
  tcase_add_test(tcase, test_clear_message_empty);
  tcase_add_test(tcase, test_clear_message_merged);
  tcase_add_test(tcase, test_clear_message_invalid);
  tcase_add_test(tcase, test_clear_invalid);
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

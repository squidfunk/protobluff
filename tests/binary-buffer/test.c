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

#include "lib/binary/buffer.h"
#include "lib/common.h"

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a fixed-size binary buffer.
 */
START_TEST(test_create) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Assert binary buffer size */
  fail_unless(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(0,  pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(16, pb_binary_buffer_left(&buffer));

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write a 8-bit variable-sized integer to a binary buffer.
 */
START_TEST(test_write_varint8) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write data to buffer */
  const uint8_t data[] = "DATA";
  for (size_t b = 0; b < 4; b++)
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_binary_buffer_write_varint8(&buffer, &(data[b])));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(4,  pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(12, pb_binary_buffer_left(&buffer));

  /* Assert binary buffer contents */
  fail_if(memcmp("DATA",
    pb_binary_buffer_data(&buffer),
    pb_binary_buffer_size(&buffer)));

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write 8-bit variable-sized integers to a binary buffer until overflow.
 */
START_TEST(test_write_varint8_overflow) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write data to buffer */
  const uint8_t data[] = "OH NO, OVERFLOW!", overflow = 'x';
  for (size_t b = 0; b < 16; b++)
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_binary_buffer_write_varint8(&buffer, &(data[b])));
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_buffer_write_varint8(&buffer, &overflow));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(16, pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(0,  pb_binary_buffer_left(&buffer));

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write a 32-bit variable-sized integer to a binary buffer.
 */
START_TEST(test_write_varint32) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write value to buffer */
  uint32_t value = 1000000000;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_varint32(&buffer, &value));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(5,  pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(11, pb_binary_buffer_left(&buffer));

  /* Assert binary buffer contents */
  uint8_t byte, offset = 0; uint32_t temp = 0;
  const uint8_t *data  = pb_binary_buffer_data(&buffer);
  for (size_t b = 0; b < pb_binary_buffer_size(&buffer); b++) {
    temp |= (uint32_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 31 && (byte & 0x80));
  } while (byte & 0x80);
  ck_assert_uint_eq(1000000000, temp);

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write 32-bit variable-sized integers to a binary buffer until overflow.
 */
START_TEST(test_write_varint32_overflow) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write value to buffer */
  uint32_t value = 1000000000;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_varint32(&buffer, &value));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_varint32(&buffer, &value));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_varint32(&buffer, &value));
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_buffer_write_varint32(&buffer, &value));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(16, pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(0,  pb_binary_buffer_left(&buffer));

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write a 64-bit variable-sized integer to a binary buffer.
 */
START_TEST(test_write_varint64) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write value to buffer */
  uint64_t value = 1000000000000000000;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_varint64(&buffer, &value));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(9, pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(7, pb_binary_buffer_left(&buffer));

  /* Assert binary buffer contents */
  uint8_t byte, offset = 0; uint64_t temp = 0;
  const uint8_t *data  = pb_binary_buffer_data(&buffer);
  for (size_t b = 0; b < pb_binary_buffer_size(&buffer); b++) {
    temp |= (uint64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  } while (byte & 0x80);
  ck_assert_uint_eq(1000000000000000000, temp);

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write 64-bit variable-sized integers to a binary buffer until overflow.
 */
START_TEST(test_write_varint64_overflow) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write value to buffer */
  uint64_t value = 1000000000000000000;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_varint64(&buffer, &value));
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_buffer_write_varint64(&buffer, &value));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(16, pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(0,  pb_binary_buffer_left(&buffer));

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write a zig-zag encoded 32-bit variable-sized integer to a binary buffer.
 */
START_TEST(test_write_svarint32) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write value to buffer */
  int32_t value = -1000000000;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_svarint32(&buffer, &value));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(5,  pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(11, pb_binary_buffer_left(&buffer));

  /* Assert binary buffer contents */
  uint8_t byte, offset = 0; uint32_t temp = 0;
  const uint8_t *data  = pb_binary_buffer_data(&buffer);
  for (size_t b = 0; b < pb_binary_buffer_size(&buffer); b++) {
    temp |= (uint32_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 31 && (byte & 0x80));
  } while (byte & 0x80);
  ck_assert_int_eq(-1000000000,
    (int32_t)((temp << 1) ^ (temp >> 31)));

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write 32-bit variable-sized integers to a binary buffer until overflow.
 */
START_TEST(test_write_svarint32_overflow) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write value to buffer */
  int32_t value = -1000000000;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_svarint32(&buffer, &value));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_svarint32(&buffer, &value));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_svarint32(&buffer, &value));
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_buffer_write_svarint32(&buffer, &value));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(16, pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(0,  pb_binary_buffer_left(&buffer));

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write a zig-zag encoded 64-bit variable-sized integer to a binary buffer.
 */
START_TEST(test_write_svarint64) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write value to buffer */
  int64_t value = -1000000000000000000;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_svarint64(&buffer, &value));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(9, pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(7, pb_binary_buffer_left(&buffer));

  /* Assert binary buffer contents */
  uint8_t byte, offset = 0; uint64_t temp = 0;
  const uint8_t *data  = pb_binary_buffer_data(&buffer);
  for (size_t b = 0; b < pb_binary_buffer_size(&buffer); b++) {
    temp |= (uint64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  } while (byte & 0x80);
  ck_assert_int_eq(-1000000000000000000,
    (int64_t)((temp << 1) ^ (temp >> 63)));

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
} END_TEST

/*
 * Write 64-bit variable-sized integers to a binary buffer until overflow.
 */
START_TEST(test_write_svarint64_overflow) {
  pb_binary_buffer_t buffer = pb_binary_buffer_create();

  /* Write value to buffer */
  int64_t value = -1000000000000000000;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_buffer_write_svarint64(&buffer, &value));
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_buffer_write_svarint64(&buffer, &value));

  /* Assert binary buffer size */
  fail_if(pb_binary_buffer_empty(&buffer));
  ck_assert_uint_eq(16, pb_binary_buffer_size(&buffer));
  ck_assert_uint_eq(0,  pb_binary_buffer_left(&buffer));

  /* Free all allocated memory */
  pb_binary_buffer_destroy(&buffer);
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
  void *suite = suite_create("protobluff/binary/buffer"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "write" */
  tcase = tcase_create("write");
  tcase_add_test(tcase, test_write_varint8);
  tcase_add_test(tcase, test_write_varint8_overflow);
  tcase_add_test(tcase, test_write_varint32);
  tcase_add_test(tcase, test_write_varint32_overflow);
  tcase_add_test(tcase, test_write_varint64);
  tcase_add_test(tcase, test_write_varint64_overflow);
  tcase_add_test(tcase, test_write_svarint32);
  tcase_add_test(tcase, test_write_svarint32_overflow);
  tcase_add_test(tcase, test_write_svarint64);
  tcase_add_test(tcase, test_write_svarint64_overflow);
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
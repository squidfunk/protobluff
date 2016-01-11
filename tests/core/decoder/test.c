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

#include <assert.h>
#include <check.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <protobluff/descriptor.h>

#include "core/buffer.h"
#include "core/common.h"
#include "core/decoder.h"
#include "core/descriptor.h"

/* ----------------------------------------------------------------------------
 * Decoder callback
 * ------------------------------------------------------------------------- */

/*!
 * Field handler that counts occurrences.
 *
 * \param[in]     descriptor Field descriptor
 * \param[in]     value      Pointer holding value
 * \param[in,out] user       User data
 */
static pb_error_t
handler(
    const pb_field_descriptor_t *descriptor, const void *value, void *user) {
  assert(descriptor && value && user);
  pb_tag_t *tags = user;
  tags[pb_field_descriptor_tag(descriptor) - 1]++;
  return PB_ERROR_NONE;
}

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

/* Enum descriptor */
static pb_enum_descriptor_t
enum_descriptor = { {
  (const pb_enum_descriptor_value_t []){
    {  0, "V00" },
    {  1, "V01" },
    {  2, "V02" }
  }, 3 } };

/* Descriptor */
static pb_descriptor_t
descriptor = { {
  (const pb_field_descriptor_t []){
    {  1, "F01", UINT32,  OPTIONAL },
    {  2, "F02", UINT64,  OPTIONAL },
    {  3, "F03", SINT32,  OPTIONAL },
    {  4, "F04", SINT64,  OPTIONAL },
    {  5, "F05", BOOL,    OPTIONAL },
    {  6, "F06", FLOAT,   REPEATED, NULL, NULL, PACKED },
    {  7, "F07", DOUBLE,  OPTIONAL },
    {  8, "F08", STRING,  OPTIONAL },
    {  9, "F09", BYTES,   OPTIONAL },
    { 10, "F10", ENUM,    OPTIONAL, &enum_descriptor },
    { 11, "F11", MESSAGE, OPTIONAL, &descriptor },
    { 12, "F12", MESSAGE, REPEATED, &descriptor }
  }, 12 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a decoder.
 */
START_TEST(test_create) {
  pb_buffer_t  buffer  = pb_buffer_create_empty();
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode a buffer using a handler.
 */
START_TEST(test_decode) {
  const uint8_t data[] = { 8, 127, 8, 127 };
  const size_t  size   = 4;

  /* Create buffer and decoder */
  pb_buffer_t  buffer  = pb_buffer_create(data, size);
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Decode using the handler */
  pb_tag_t tags[12] = {};
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_decoder_decode(&decoder, handler, tags));

  /* Assert expected occurences */
  ck_assert_uint_eq(2, tags[0]);

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode a buffer with a message using a handler.
 */
START_TEST(test_decode_message) {
  const uint8_t data[] = { 98, 2, 8, 127, 8, 127 };
  const size_t  size   = 6;

  /* Create buffer and decoder */
  pb_buffer_t  buffer  = pb_buffer_create(data, size);
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Decode using the handler */
  pb_tag_t tags[12] = {};
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_decoder_decode(&decoder, handler, tags));

  /* Assert expected occurences */
  ck_assert_uint_eq(1, tags[0]);
  ck_assert_uint_eq(1, tags[11]);

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode a buffer with an unknown tag using a handler.
 */
START_TEST(test_decode_skip) {
  const uint8_t data[] = { 104, 1, 8, 127, 16, 127 };
  const size_t  size   = 6;

  /* Create buffer and decoder */
  pb_buffer_t  buffer  = pb_buffer_create(data, size);
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Decode using the handler */
  pb_tag_t tags[12] = {};
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_decoder_decode(&decoder, handler, tags));

  /* Assert expected occurences */
  ck_assert_uint_eq(1, tags[0]);
  ck_assert_uint_eq(1, tags[1]);

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode a buffer with a packed field using a handler.
 */
START_TEST(test_decode_packed) {
  const uint8_t data[] = { 50, 8, 0, 202, 154, 59, 0, 202, 154, 59 };
  const size_t  size   = 10;

  /* Create buffer and decoder */
  pb_buffer_t  buffer  = pb_buffer_create(data, size);
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Decode using the handler */
  pb_tag_t tags[12] = {};
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_decoder_decode(&decoder, handler, tags));

  /* Assert expected occurences */
  ck_assert_uint_eq(2, tags[5]);

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode an invalid buffer using a handler.
 */
START_TEST(test_decode_invalid) {
  pb_buffer_t  buffer  = pb_buffer_create_invalid();
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_if(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_decoder_error(&decoder));

  /* Decode using the handler */
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_decoder_decode(&decoder, handler, NULL));

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode a buffer with an invalid tag using a handler.
 */
START_TEST(test_decode_invalid_tag) {
  const uint8_t data[] = { 255 };
  const size_t  size   = 1;

  /* Create buffer and decoder */
  pb_buffer_t  buffer  = pb_buffer_create(data, size);
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Decode using the handler */
  ck_assert_uint_eq(PB_ERROR_VARINT,
    pb_decoder_decode(&decoder, handler, NULL));

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode a buffer with invalid length using a handler.
 */
START_TEST(test_decode_invalid_length) {
  const uint8_t data[] = { 26, 5 };
  const size_t  size   = 2;

  /* Create buffer and decoder */
  pb_buffer_t  buffer  = pb_buffer_create(data, size);
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Decode using the handler */
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_decoder_decode(&decoder, handler, NULL));

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode a buffer with an invalid length prefix using a handler.
 */
START_TEST(test_decode_invalid_length_data) {
  const uint8_t data[] = { 26, 130, 131, 132, 133, 134, 135, 136 };
  const size_t  size   = 7;

  /* Create buffer and decoder */
  pb_buffer_t  buffer  = pb_buffer_create(data, size);
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Decode using the handler */
  ck_assert_uint_eq(PB_ERROR_VARINT,
    pb_decoder_decode(&decoder, handler, NULL));

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode a buffer with an invalid value using a handler.
 */
START_TEST(test_decode_invalid_value) {
  const uint8_t data[] = { 8, 255 };
  const size_t  size   = 2;

  /* Create buffer and decoder */
  pb_buffer_t  buffer  = pb_buffer_create(data, size);
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Decode using the handler */
  ck_assert_uint_eq(PB_ERROR_VARINT,
    pb_decoder_decode(&decoder, handler, NULL));

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Decode a buffer with an invalid packed field using a handler.
 */
START_TEST(test_decode_invalid_packed) {
  const uint8_t data[] = { 50, 255 };
  const size_t  size   = 2;

  /* Create buffer and decoder */
  pb_buffer_t  buffer  = pb_buffer_create(data, size);
  pb_decoder_t decoder = pb_decoder_create(&descriptor, &buffer);

  /* Assert decoder validity and error */
  fail_unless(pb_decoder_valid(&decoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_decoder_error(&decoder));

  /* Decode using the handler */
  ck_assert_uint_eq(PB_ERROR_VARINT,
    pb_decoder_decode(&decoder, handler, NULL));

  /* Free all allocated memory */
  pb_decoder_destroy(&decoder);
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
  void *suite = suite_create("protobluff/core/decoder"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "decode" */
  tcase = tcase_create("decode");
  tcase_add_test(tcase, test_decode);
  tcase_add_test(tcase, test_decode_message);
  tcase_add_test(tcase, test_decode_skip);
  tcase_add_test(tcase, test_decode_packed);
  tcase_add_test(tcase, test_decode_invalid);
  tcase_add_test(tcase, test_decode_invalid_tag);
  tcase_add_test(tcase, test_decode_invalid_length);
  tcase_add_test(tcase, test_decode_invalid_length_data);
  tcase_add_test(tcase, test_decode_invalid_value);
  tcase_add_test(tcase, test_decode_invalid_packed);
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
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

#include <protobluff/descriptor.h>

#include "lib/binary.h"
#include "lib/common.h"
#include "lib/cursor.h"
#include "lib/field.h"
#include "lib/message.h"

/* ----------------------------------------------------------------------------
 * Defaults
 * ------------------------------------------------------------------------- */

static const uint32_t
default_uint32 = 1000000000UL;

static const uint64_t
default_uint64 = 1000000000000000000ULL;

static const int32_t
default_int32 = -1000000000L;

static const int64_t
default_int64 = -1000000000000000000LL;

static const uint8_t
default_bool = 0;

static const float
default_float = 0.0001;

static const double
default_double = 0.00000001;

static const pb_string_t
default_string = pb_string_const("DEFAULT");

static const pb_enum_t
default_enum = 1L;

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

static const pb_enum_descriptor_t
descriptor_enum = { {
  (const pb_enum_descriptor_value_t []){
    { 1L, "V01" },
    { 2L, "V02" },
    { 3L, "V03" }
  }, 3 } };

static pb_message_descriptor_t
descriptor = { {
  (const pb_field_descriptor_t []){
    {  1, "F01", UINT32,  OPTIONAL, NULL, &default_uint32 },
    {  2, "F02", UINT64,  OPTIONAL, NULL, &default_uint64 },
    {  3, "F03", SINT32,  OPTIONAL, NULL, &default_int32 },
    {  4, "F04", SINT64,  OPTIONAL, NULL, &default_int64 },
    {  5, "F05", BOOL,    OPTIONAL, NULL, &default_bool },
    {  6, "F06", FLOAT,   OPTIONAL, NULL, &default_float },
    {  7, "F07", DOUBLE,  OPTIONAL, NULL, &default_double },
    {  8, "F08", STRING,  OPTIONAL, NULL, &default_string },
    {  9, "F09", ENUM,    OPTIONAL, &descriptor_enum, &default_enum },
    { 10, "F10", UINT64,  REPEATED },
    { 11, "F11", STRING,  REPEATED },
    { 12, "F12", MESSAGE, OPTIONAL, &descriptor }
  }, 12 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a field within a message for a specific tag.
 */
START_TEST(test_create) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(1, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Assert field offsets */
  ck_assert_uint_eq(1, pb_field_start(&field));
  ck_assert_uint_eq(2, pb_field_end(&field));

  /* Assert binary contents */
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(2, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create an existing field within a message for a specific tag.
 */
START_TEST(test_create_existing) {
  const uint8_t data[] = { 8, 128, 168, 214, 185, 7 };
  const size_t  size   = 6;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Read value from field */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_uint_eq(2000000000, value);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(5, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Assert field offsets */
  ck_assert_uint_eq(1, pb_field_start(&field));
  ck_assert_uint_eq(6, pb_field_end(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a set of repeated fields within a message for a specific tag.
 */
START_TEST(test_create_repeated) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);

  /* Create a hundred fields and write values to them */
  for (size_t f = 1; f < 101; f++) {
    pb_field_t field = pb_field_create(&message, 10);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &f));

    /* Assert field validity and error */
    fail_unless(pb_field_valid(&field));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

    /* Assert field size and version */
    fail_if(pb_field_empty(&field));
    ck_assert_uint_eq(1, pb_field_size(&field));
    ck_assert_uint_eq(f * 2, pb_field_version(&field));

    /* Assert field offsets */
    ck_assert_uint_eq(f * 2 - 1, pb_field_start(&field));
    ck_assert_uint_eq(f * 2, pb_field_end(&field));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(f * 2, pb_binary_size(&binary));

    /* Free all allocated memory */
    pb_field_destroy(&field);
  }

  /* Assert binary contents */
  for (size_t f = 1; f < 101; f++)
    ck_assert_uint_eq(f, pb_binary_data_at(&binary, f * 2 - 1));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a field within an empty message for a specific tag.
 */
START_TEST(test_create_message_empty) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(5, pb_field_size(&field));
  ck_assert_uint_eq(2, pb_field_version(&field));

  /* Assert field offsets */
  ck_assert_uint_eq(1, pb_field_start(&field));
  ck_assert_uint_eq(6, pb_field_end(&field));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(6, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a field within an invalid message for a specific tag.
 */
START_TEST(test_create_message_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Create field from invalid message */
  pb_field_t field = pb_field_create(&message, 1);

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
} END_TEST

/*
 * Create a field within a message for a specific tag without setting defaults.
 */
START_TEST(test_create_without_default) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create_without_default(&message, 1);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Assert field size and version */
  fail_unless(pb_field_empty(&field));
  ck_assert_uint_eq(0, pb_field_size(&field));
  ck_assert_uint_eq(1, pb_field_version(&field));

  /* Assert field offsets */
  ck_assert_uint_eq(1, pb_field_start(&field));
  ck_assert_uint_eq(1, pb_field_end(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create an existing field within a message without setting defaults.
 */
START_TEST(test_create_without_default_existing) {
  const uint8_t data[] = { 8, 128, 168, 214, 185, 7 };
  const size_t  size   = 6;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create_without_default(&message, 1);

  /* Read value from field */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_uint_eq(2000000000, value);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(5, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Assert field offsets */
  ck_assert_uint_eq(1, pb_field_start(&field));
  ck_assert_uint_eq(6, pb_field_end(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a field within an invalid message for a specific tag.
 */
START_TEST(test_create_without_default_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Create field from invalid message */
  pb_field_t field = pb_field_create_without_default(&message, 1);

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
} END_TEST

/*
 * Create a field within a nested message for a branch of tags.
 */
START_TEST(test_create_nested) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &binary) };

  /* Create a hundred nested submessages */
  for (size_t m = 1; m < 101; m++)
    messages[m] = pb_message_create_within(&(messages[m - 1]), 12);

  /* Write value to innermost message */
  uint32_t value = 2000000000, check;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_put(&(messages[100]), 1, &value));

  /* Free all allocated memory */
  for (size_t m = 1; m < 101; m++)
    pb_message_destroy(&(messages[m]));

  /* Create tags */
  pb_tag_t tags[101] = {};
  for (size_t t = 0; t < 100; t++)
    tags[t] = 12;
  tags[100] = 1;

  /* Create field for innermost message */
  pb_field_t field = pb_field_create_nested(&(messages[0]), tags, 101);
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &check));
  ck_assert_uint_eq(value, check);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&(messages[0]));
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a field within an invalid nested message.
 */
START_TEST(test_create_nested_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Create tags */
  pb_tag_t tags[100] = {};
  for (size_t t = 0; t < 100; t++)
    tags[t] = 1;

  /* Create field for innermost message */
  pb_field_t field = pb_field_create_nested(&message, tags, 100);

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
} END_TEST

/*
 * Create a field at the current position of a cursor.
 */
START_TEST(test_create_from_cursor) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);

  /* Create a hundred fields and write values to them */
  for (size_t f = 1; f < 101; f++) {
    pb_field_t field = pb_field_create(&message, 10);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &f));

    /* Free all allocated memory */
    pb_field_destroy(&field);
  }

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 10);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through message and read fields */
  for (size_t f = 1, value; f < 101; f++, pb_cursor_next(&cursor)) {
    pb_field_t field = pb_field_create_from_cursor(&cursor);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
    ck_assert_uint_eq(f, value);

    /* Assert field validity and error */
    fail_unless(pb_field_valid(&field));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

    /* Assert field size and version */
    fail_if(pb_field_empty(&field));
    ck_assert_uint_eq(1, pb_field_size(&field));
    ck_assert_uint_eq(200, pb_field_version(&field));

    /* Assert field offsets */
    ck_assert_uint_eq(f * 2 - 1, pb_field_start(&field));
    ck_assert_uint_eq(f * 2, pb_field_end(&field));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(200, pb_binary_size(&binary));

    /* Alter value of the field */
    value = 101 - f;
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));

    /* Free all allocated memory */
    pb_field_destroy(&field);
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Assert binary contents */
  for (size_t f = 1; f < 101; f++)
    ck_assert_uint_eq(101 - f, pb_binary_data_at(&binary, f * 2 - 1));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a field from an invalid cursor.
 */
START_TEST(test_create_from_cursor_invalid) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Create field from cursor */
  pb_field_t field = pb_field_create_from_cursor(&cursor);

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a field with an invalid tag from a cursor.
 */
START_TEST(test_create_from_cursor_invalid_tag) {
  const uint8_t data[] = { 144, 1, 0 };
  const size_t  size   = 3;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(18, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0,  pb_cursor_pos(&cursor));

  /* Create field for invalid tag */
  pb_field_t field = pb_field_create_from_cursor(&cursor);

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a field with an invalid wiretype from a cursor.
 */
START_TEST(test_create_from_cursor_invalid_wiretype) {
  const uint8_t data[] = { 98, 1, 33 };
  const size_t  size   = 3;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(12, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0,  pb_cursor_pos(&cursor));

  /* Create field for invalid wiretype */
  pb_field_t field = pb_field_create_from_cursor(&cursor);

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create an invalid field.
 */
START_TEST(test_create_invalid) {
  pb_field_t field = pb_field_create_invalid();

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
} END_TEST

/*
 * Create a field with an invalid length prefix.
 */
START_TEST(test_create_invalid_length) {
  const uint8_t data[] = { 66, 100, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  const size_t  size   = 11;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a 32-bit unsigned integer field with the given value.
 */
START_TEST(test_match) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Compare with value of field */
  uint32_t value = 127;
  fail_unless(pb_field_match(&field, &value));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a 32-bit unsigned integer field with the given value.
 */
START_TEST(test_match_not) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Compare with value of field */
  uint32_t value = 100;
  fail_if(pb_field_match(&field, &value));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a 32-bit unsigned integer field with the given value.
 */
START_TEST(test_match_uint32) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Compare with value of field */
  fail_unless(pb_field_match(&field, &default_uint32));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a 64-bit unsigned integer field with the given value.
 */
START_TEST(test_match_uint64) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 2);

  /* Compare with value of field */
  fail_unless(pb_field_match(&field, &default_uint64));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a 32-bit integer field with the given value.
 */
START_TEST(test_match_int32) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 3);

  /* Compare with value of field */
  fail_unless(pb_field_match(&field, &default_int32));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a 64-bit integer field with the given value.
 */
START_TEST(test_match_int64) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 4);

  /* Compare with value of field */
  fail_unless(pb_field_match(&field, &default_int64));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a boolean field with the given value.
 */
START_TEST(test_match_bool) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 5);

  /* Compare with value of field */
  fail_unless(pb_field_match(&field, &default_bool));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a single-precision float field with the given value.
 */
START_TEST(test_match_float) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 6);

  /* Compare with value of field */
  fail_unless(pb_field_match(&field, &default_float));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a double-precision float field with the given value.
 */
START_TEST(test_match_double) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 7);

  /* Compare with value of field */
  fail_unless(pb_field_match(&field, &default_double));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of a string field with the given value.
 */
START_TEST(test_match_string) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Compare with value of field */
  fail_unless(pb_field_match(&field, &default_string));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of an enum field with the given value.
 */
START_TEST(test_match_enum) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 9);

  /* Compare with value of field */
  fail_unless(pb_field_match(&field, &default_enum));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of an unaligned field with the given value.
 */
START_TEST(test_match_unaligned) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 1);
  pb_field_t   field2  = pb_field_create(&message, 1);

  /* Write and compare from two instances */
  uint32_t value = 100;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field1, &value));
  fail_unless(pb_field_match(&field2, &value));

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field2));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of an unaligned, invalid field with the given value.
 */
START_TEST(test_match_unaligned_invalid) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 1);
  pb_field_t   field2  = pb_field_create(&message, 1);

  /* Clear first field and read from second */
  uint32_t value = 100;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field1, &value));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&field1));
  fail_if(pb_field_match(&field2, &value));

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field2));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of an invalid field with the given value.
 */
START_TEST(test_match_invalid) {
  pb_field_t field = pb_field_create_invalid();

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Compare with value of field */
  uint32_t value = 0;
  fail_if(pb_field_match(&field, &value));

  /* Free all allocated memory */
  pb_field_destroy(&field);
} END_TEST

/*
 * Compare the value of a field with invalid length prefix.
 */
START_TEST(test_match_invalid_length) {
  uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  size_t  size   = 11;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_zero_copy(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Sabotage binary and compare with value from field */
  data[1] = 100; pb_string_t value = pb_string_init("SOME DATA");
  fail_if(pb_field_match(&field, &value));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from a field.
 */
START_TEST(test_get) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Read value from field */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_uint_eq(127, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(1, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from a 32-bit unsigned integer field.
 */
START_TEST(test_get_uint32) {
  const uint8_t data[] = { 8, 128, 168, 214, 185, 7 };
  const size_t  size   = 6;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Read value from field */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_uint_eq(2000000000, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(5, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from a 64-bit unsigned integer field.
 */
START_TEST(test_get_uint64) {
  const uint8_t data[] = { 16, 128, 128, 160, 246, 244, 172, 219, 224, 27 };
  const size_t  size   = 10;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 2);

  /* Read value from field */
  uint64_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_uint_eq(2000000000000000000, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(9, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from a 32-bit integer field.
 */
START_TEST(test_get_int32) {
  const uint8_t data[] = { 24, 128, 236, 148, 163, 4 };
  const size_t  size   = 6;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 3);

  /* Read value from field */
  int32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_int_eq(-2000000000, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(5, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from a 64-bit integer field.
 */
START_TEST(test_get_int64) {
  const uint8_t data[] = { 32, 128, 128, 240, 196, 197, 169, 210, 143, 114 };
  const size_t  size   = 10;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 4);

  /* Read value from field */
  int64_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_int_eq(-2000000000000000000, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(9, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from a boolean field.
 */
START_TEST(test_get_bool) {
  const uint8_t data[] = { 40, 1 };
  const size_t  size   = 2;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 5);

  /* Read value from field */
  uint8_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_uint_eq(1, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(1, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from a single-precision float field.
 */
START_TEST(test_get_float) {
  const uint8_t data[] = { 53, 23, 183, 81, 57 };
  const size_t  size   = 5;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 6);

  /* Read value from field */
  float value = 0.0002, check;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &check));
  fail_if(memcmp(&value, &check, sizeof(float)));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(4, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from a single-precision float field.
 */
START_TEST(test_get_double) {
  const uint8_t data[] = { 57, 58, 140, 48, 226, 142, 121, 85, 62 };
  const size_t  size   = 9;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 7);

  /* Read value from field */
  double value = 0.00000002, check;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &check));
  fail_if(memcmp(&value, &check, sizeof(double)));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(8, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from a string field.
 */
START_TEST(test_get_string) {
  const uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  const size_t  size   = 11;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Read value from field */
  pb_string_t value = pb_string_init("SOME DATA"), check;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &check));
  fail_if(memcmp(value.data, check.data, value.size));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(9, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from an enum field.
 */
START_TEST(test_get_enum) {
  const uint8_t data[] = { 72, 1 };
  const size_t  size   = 2;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 9);

  /* Read value from field */
  pb_enum_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_int_eq(1, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(1, pb_field_size(&field));
  ck_assert_uint_eq(0, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the default value from a 32-bit unsigned integer field.
 */
START_TEST(test_get_default_uint32) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Read default value from field */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_uint_eq(default_uint32, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(5, pb_field_size(&field));
  ck_assert_uint_eq(2, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the default value from a 64-bit unsigned integer field.
 */
START_TEST(test_get_default_uint64) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 2);

  /* Read default value from field */
  uint64_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_uint_eq(default_uint64, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(9, pb_field_size(&field));
  ck_assert_uint_eq(2, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the default value from a 32-bit integer field.
 */
START_TEST(test_get_default_int32) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 3);

  /* Read default value from field */
  int32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_int_eq(default_int32, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(5, pb_field_size(&field));
  ck_assert_uint_eq(2, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the default value from a 64-bit integer field.
 */
START_TEST(test_get_default_int64) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 4);

  /* Read default value from field */
  int64_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_int_eq(default_int64, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(9, pb_field_size(&field));
  ck_assert_uint_eq(2, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the default value from a boolean field.
 */
START_TEST(test_get_default_bool) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 5);

  /* Read default value from field */
  uint8_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_uint_eq(default_bool, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(1, pb_field_size(&field));
  ck_assert_uint_eq(2, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the default value from a single-precision float field.
 */
START_TEST(test_get_default_float) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 6);

  /* Read default value from field */
  float value = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  fail_if(memcmp(&default_float, &value, sizeof(float)));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(4, pb_field_size(&field));
  ck_assert_uint_eq(2, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the default value from a double-precision float field.
 */
START_TEST(test_get_default_double) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 7);

  /* Read default value from field */
  double value = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  fail_if(memcmp(&default_double, &value, sizeof(double)));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(8, pb_field_size(&field));
  ck_assert_uint_eq(2, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the default value from a string field.
 */
START_TEST(test_get_default_string) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Read default value from field */
  pb_string_t value;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  fail_if(memcmp(default_string.data, value.data, default_string.size));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the default value from an enum field.
 */
START_TEST(test_get_default_enum) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 9);

  /* Read default value from field */
  pb_enum_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  ck_assert_int_eq(default_enum, value);

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(1, pb_field_size(&field));
  ck_assert_uint_eq(2, pb_field_version(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from an unaligned field.
 */
START_TEST(test_get_unaligned) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 1);
  pb_field_t   field2  = pb_field_create(&message, 1);

  /* Write and read from two instances */
  uint32_t value = 100, check = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field1, &value));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field2, &check));
  ck_assert_uint_eq(100, check);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field2));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from an unaligned, invalid field.
 */
START_TEST(test_get_unaligned_invalid) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 1);
  pb_field_t   field2  = pb_field_create(&message, 1);

  /* Clear first field and read from second */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&field1));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_get(&field2, &value));

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field2));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value from an invalid field.
 */
START_TEST(test_get_invalid) {
  pb_field_t field = pb_field_create_invalid();

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Read value from field */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_get(&field, &value));

  /* Free all allocated memory */
  pb_field_destroy(&field);
} END_TEST

/*
 * Read the value from a string field with an invalid length prefix.
 */
START_TEST(test_get_invalid_length) {
  uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  size_t  size   = 11;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_zero_copy(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Sabotage binary and read value from field */
  data[1] = 100; pb_string_t value;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN, pb_field_get(&field, &value));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a value to a field.
 */
START_TEST(test_put) {
  uint8_t data[] = { 8, 127 };
  size_t  size   = 2;

  /* Create binary, mesage and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Write value to field */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(1, pb_field_size(&field));
  ck_assert_uint_eq(3, pb_field_version(&field));

  /* Assert field offsets */
  ck_assert_uint_eq(1, pb_field_start(&field));
  ck_assert_uint_eq(2, pb_field_end(&field));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(2, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a 32-bit unsigned integer value to a field.
 */
START_TEST(test_put_uint32) {
  const uint8_t data[] = { 8, 128, 168, 214, 185, 7 };
  const size_t  size   = 6;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Write value to field */
  uint32_t value = 2000000000;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a 64-bit unsigned integer value to a field.
 */
START_TEST(test_put_uint64) {
  const uint8_t data[] = { 16, 128, 128, 160, 246, 244, 172, 219, 224, 27 };
  const size_t  size   = 10;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 2);

  /* Write value to field */
  uint64_t value = 2000000000000000000;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a 32-bit integer value to a field.
 */
START_TEST(test_put_int32) {
  const uint8_t data[] = { 24, 128, 236, 148, 163, 4 };
  const size_t  size   = 6;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 3);

  /* Write value to field */
  int32_t value = -2000000000;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a 64-bit integer value to a field.
 */
START_TEST(test_put_int64) {
  const uint8_t data[] = { 32, 128, 128, 240, 196, 197, 169, 210, 143, 114 };
  const size_t  size   = 10;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 4);

  /* Write value to field */
  int64_t value = -2000000000000000000;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a boolean value to a field.
 */
START_TEST(test_put_bool) {
  const uint8_t data[] = { 40, 1 };
  const size_t  size   = 2;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 5);

  /* Write value to field */
  uint8_t value = 1;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a single-precision float value to a field.
 */
START_TEST(test_put_float) {
  const uint8_t data[] = { 53, 23, 183, 81, 57 };
  const size_t  size   = 5;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 6);

  /* Write value to field */
  float value = 0.0002;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a double-precision float value to a field.
 */
START_TEST(test_put_double) {
  const uint8_t data[] = { 57, 58, 140, 48, 226, 142, 121, 85, 62 };
  const size_t  size   = 9;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 7);

  /* Write value to field */
  double value = 0.00000002;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a string value to a field.
 */
START_TEST(test_put_string) {
  const uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  const size_t  size   = 11;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Write value to field */
  pb_string_t value = pb_string_init("SOME DATA");
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field));
  ck_assert_uint_eq(9, pb_field_size(&field));
  ck_assert_uint_eq(3, pb_field_version(&field));

  /* Assert field offsets */
  ck_assert_uint_eq(2,  pb_field_start(&field));
  ck_assert_uint_eq(11, pb_field_end(&field));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(11, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write an enum value to a field.
 */
START_TEST(test_put_enum) {
  const uint8_t data[] = { 72, 2 };
  const size_t  size   = 2;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 9);

  /* Write value to field */
  pb_enum_t value = 2;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));
  fail_if(memcmp(data, pb_binary_data(&binary), size));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a string value to an existing field.
 */
START_TEST(test_put_string_existing) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Write value to field */
  pb_string_t value1 = pb_string_init("SOME DATA"), check1;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &check1));
  fail_if(memcmp(value1.data, check1.data, value1.size));

  /* Assert binary size */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(11, pb_binary_size(&binary));

  /* Write different value to field */
  pb_string_t value2 = pb_string_init("EVEN MORE DATA"), check2;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &check2));
  fail_if(memcmp(value2.data, check2.data, value2.size));

  /* Assert binary size again */
  fail_if(pb_binary_empty(&binary));
  ck_assert_uint_eq(16, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a set of string value to a repeated field.
 */
START_TEST(test_put_string_repeated) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);

  /* Create a hundred fields and write strings to them */
  pb_string_t value = pb_string_init("DATA"), check;
  for (size_t f = 1; f < 101; f++) {
    pb_field_t field = pb_field_create(&message, 11);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));

    /* Assert field validity and error */
    fail_unless(pb_field_valid(&field));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

    /* Assert field size and version */
    fail_if(pb_field_empty(&field));
    ck_assert_uint_eq(4, pb_field_size(&field));
    ck_assert_uint_eq(f * 2, pb_field_version(&field));

    /* Assert field offsets */
    ck_assert_uint_eq(f * 6 - 4, pb_field_start(&field));
    ck_assert_uint_eq(f * 6, pb_field_end(&field));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(f * 6, pb_binary_size(&binary));

    /* Free all allocated memory */
    pb_field_destroy(&field);
  }

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 11);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through message and read fields */
  for (size_t f = 1; f < 101; f++, pb_cursor_next(&cursor)) {
    pb_field_t field = pb_field_create_from_cursor(&cursor);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &check));
    fail_if(memcmp(value.data, check.data, value.size));

    /* Assert field validity and error */
    fail_unless(pb_field_valid(&field));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

    /* Assert field size and version */
    fail_if(pb_field_empty(&field));
    ck_assert_uint_eq(4, pb_field_size(&field));
    ck_assert_uint_eq(200, pb_field_version(&field));

    /* Assert field offsets */
    ck_assert_uint_eq(f * 6 - 4, pb_field_start(&field));
    ck_assert_uint_eq(f * 6, pb_field_end(&field));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(600, pb_binary_size(&binary));

    /* Free all allocated memory */
    pb_field_destroy(&field);
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a string value incrementally to a field.
 */
START_TEST(test_put_string_incremental) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Create a hundred fields and write strings to them */
  for (size_t f = 1; f < 38; f++) {
    pb_string_t value = pb_string_init(
      "HELP I'M TRAPPED IN A UNIVERSE FACTORY", f);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));

    /* Assert field validity and error */
    fail_unless(pb_field_valid(&field));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

    /* Assert field size and version */
    fail_if(pb_field_empty(&field));
    ck_assert_uint_eq(f, pb_field_size(&field));
    ck_assert_uint_eq(f + 2, pb_field_version(&field));

    /* Assert field offsets */
    ck_assert_uint_eq(2, pb_field_start(&field));
    ck_assert_uint_eq(f + 2, pb_field_end(&field));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(f + 2, pb_binary_size(&binary));
  }

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a value to an unaligned field.
 */
START_TEST(test_put_unaligned) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 1);
  pb_field_t   field2  = pb_field_create(&message, 1);

  /* Write values to fields */
  uint32_t value1 = 100, value2 = 200;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field1, &value1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field2, &value2));

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field1));

  /* Assert contents */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field1, &value1));
  ck_assert_uint_eq(200, value1);

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a value to an unaligned, invalid field.
 */
START_TEST(test_put_unaligned_invalid) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 1);
  pb_field_t   field2  = pb_field_create(&message, 1);

  /* Clear first field and write to second */
  uint32_t value = 100;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&field1));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_put(&field2, &value));

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field2));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a value to an invalid field.
 */
START_TEST(test_put_invalid) {
  pb_field_t field = pb_field_create_invalid();

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Write value to field */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_put(&field, &value));

  /* Free all allocated memory */
  pb_field_destroy(&field);
} END_TEST

/*
 * Clear a field entirely.
 */
START_TEST(test_clear) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 1);

  /* Clear field */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_clear(&field));

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear a string field entirely.
 */
START_TEST(test_clear_string) {
  const uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  const size_t  size   = 11;

  /* Create binary, message and field */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 8);

  /* Clear field */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_clear(&field));

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear a set of repeated string fields entirely.
 */
START_TEST(test_clear_string_repeated) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   fields[100];

  /* Create a hundred fields and write strings to them */
  pb_string_t value = pb_string_init("DATA");
  for (size_t f = 0; f < 100; f++) {
    fields[f] = pb_field_create(&message, 11);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&(fields[f]), &value));
  }

  /* Clear fields */
  for (size_t f = 0; f < 100; f++) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&(fields[f])));
    ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_clear(&(fields[f])));

    /* Assert field validity and error */
    fail_if(pb_field_valid(&(fields[f])));
    ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&(fields[f])));

    /* Assert binary size */
    ck_assert_uint_eq(f == 99, pb_binary_empty(&binary));
    ck_assert_uint_eq((99 - f) * 6, pb_binary_size(&binary));

    /* Free all allocated memory */
    pb_field_destroy(&(fields[f]));
  }

  /* Align message and assert size */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear an unaligned string field entirely.
 */
START_TEST(test_clear_string_unaligned) {
  const uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  const size_t  size   = 11;

  /* Create binary, message and fields */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 8);
  pb_field_t   field2  = pb_field_create(&message, 8);

  /* Clear field */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&field1));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_clear(&field1));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_clear(&field2));

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field2));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear an unaligned field entirely.
 */
START_TEST(test_clear_unaligned) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 1);
  pb_field_t   field2  = pb_field_create(&message, 1);

  /* Write values to fields */
  uint32_t value = 100;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field1, &value));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&field2));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_clear(&field2));

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field2));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear an unaligned, invalid field entirely.
 */
START_TEST(test_clear_unaligned_invalid) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 1);
  pb_field_t   field2  = pb_field_create(&message, 1);

  /* Clear both fields */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&field1));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_clear(&field1));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_clear(&field2));

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field2));

  /* Assert binary size */
  fail_unless(pb_binary_empty(&binary));
  ck_assert_uint_eq(0, pb_binary_size(&binary));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Clear an invalid field entirely.
 */
START_TEST(test_clear_invalid) {
  pb_field_t field = pb_field_create_invalid();

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Clear field */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_clear(&field));

  /* Free all allocated memory */
  pb_field_destroy(&field);
} END_TEST

/*
 * Retrieve a pointer to the raw data of a field.
 */
START_TEST(test_raw) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 6);

  /* Read default value from field */
  float value = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  fail_if(memcmp(&default_float, &value, sizeof(float)));

  /* Obtain and change raw data */
  float *raw = pb_field_raw(&field);
  ck_assert_ptr_ne(NULL, raw);
  *raw = 0.123456789;

  /* Read new value from field */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  fail_if(memcmp(raw, &value, sizeof(float)));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Retrieve a pointer to the raw data of an unaligned field.
 */
START_TEST(test_raw_unaligned) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 6);
  pb_field_t   field2  = pb_field_create(&message, 6);

  /* Read default value from field */
  float value = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field1, &value));
  fail_if(memcmp(&default_float, &value, sizeof(float)));

  /* Obtain and change raw data */
  float *raw = pb_field_raw(&field2);
  ck_assert_ptr_ne(NULL, raw);
  *raw = 0.123456789;

  /* Read new value from field */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field2, &value));
  fail_if(memcmp(raw, &value, sizeof(float)));

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field2));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Retrieve a pointer to the raw data of an unaligned, invalid field.
 */
START_TEST(test_raw_unaligned_invalid) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 6);
  pb_field_t   field2  = pb_field_create(&message, 6);

  /* Read default value from field */
  float value = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field1, &value));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_clear(&field1));

  /* Obtain and change raw data */
  float *raw = pb_field_raw(&field2);
  ck_assert_ptr_eq(NULL, raw);

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field2));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Retrieve a pointer to the raw data of an invalid field.
 */
START_TEST(test_raw_invalid) {
  pb_field_t field = pb_field_create_invalid();

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Obtain raw data */
  void *raw = pb_field_raw(&field);
  ck_assert_ptr_eq(NULL, raw);

  /* Free all allocated memory */
  pb_field_destroy(&field);
} END_TEST

/*
 * Ensure that a field is properly aligned.
 */
START_TEST(test_align) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);

  /* Create message with two field references */
  pb_message_t submessage = pb_message_create_within(&message, 12);
  pb_field_t   field1     = pb_field_create(&submessage, 8);
  pb_field_t   field2     = pb_field_create(&submessage, 8);

  /* Write value to field */
  pb_string_t value = pb_string_init(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam euismod "
    "vehicula nibh, et egestas erat eleifend quis. Nam hendrerit egestas "
    "quam nec egestas. Donec lacinia vestibulum erat, ac suscipit nisi "
    "vehicula nec. Praesent ullamcorper vitae lorem vel euismod. Quisque "
    "fringilla lobortis convallis. Aliquam accumsan lacus eu viverra dapibus. "
    "Phasellus in adipiscing sem, in congue massa. Vestibulum ullamcorper "
    "orci nec semper pretium.");
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field1, &value));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field1));
  ck_assert_uint_eq(437, pb_field_size(&field1));
  ck_assert_uint_eq(6, pb_field_version(&field1));

  /* Assert alignment */
  ck_assert_uint_eq(3, pb_field_version(&field2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_align(&field2));
  fail_unless(pb_field_equals(&field1, &field2));

  /* Free all allocated memory */
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
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
  void *suite = suite_create("protobluff/field"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_existing);
  tcase_add_test(tcase, test_create_repeated);
  tcase_add_test(tcase, test_create_message_empty);
  tcase_add_test(tcase, test_create_message_invalid);
  tcase_add_test(tcase, test_create_without_default);
  tcase_add_test(tcase, test_create_without_default_existing);
  tcase_add_test(tcase, test_create_without_default_invalid);
  tcase_add_test(tcase, test_create_nested);
  tcase_add_test(tcase, test_create_nested_invalid);
  tcase_add_test(tcase, test_create_from_cursor);
  tcase_add_test(tcase, test_create_from_cursor_invalid);
  tcase_add_test(tcase, test_create_from_cursor_invalid_tag);
  tcase_add_test(tcase, test_create_from_cursor_invalid_wiretype);
  tcase_add_test(tcase, test_create_invalid);
  tcase_add_test(tcase, test_create_invalid_length);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "match" */
  tcase = tcase_create("match");
  tcase_add_test(tcase, test_match);
  tcase_add_test(tcase, test_match_not);
  tcase_add_test(tcase, test_match_uint32);
  tcase_add_test(tcase, test_match_uint64);
  tcase_add_test(tcase, test_match_int32);
  tcase_add_test(tcase, test_match_int64);
  tcase_add_test(tcase, test_match_bool);
  tcase_add_test(tcase, test_match_float);
  tcase_add_test(tcase, test_match_double);
  tcase_add_test(tcase, test_match_string);
  tcase_add_test(tcase, test_match_enum);
  tcase_add_test(tcase, test_match_unaligned);
  tcase_add_test(tcase, test_match_unaligned_invalid);
  tcase_add_test(tcase, test_match_invalid);
  tcase_add_test(tcase, test_match_invalid_length);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "get" */
  tcase = tcase_create("get");
  tcase_add_test(tcase, test_get);
  tcase_add_test(tcase, test_get_uint32);
  tcase_add_test(tcase, test_get_uint64);
  tcase_add_test(tcase, test_get_int32);
  tcase_add_test(tcase, test_get_int64);
  tcase_add_test(tcase, test_get_bool);
  tcase_add_test(tcase, test_get_float);
  tcase_add_test(tcase, test_get_double);
  tcase_add_test(tcase, test_get_string);
  tcase_add_test(tcase, test_get_enum);
  tcase_add_test(tcase, test_get_default_uint32);
  tcase_add_test(tcase, test_get_default_uint64);
  tcase_add_test(tcase, test_get_default_int32);
  tcase_add_test(tcase, test_get_default_int64);
  tcase_add_test(tcase, test_get_default_bool);
  tcase_add_test(tcase, test_get_default_float);
  tcase_add_test(tcase, test_get_default_double);
  tcase_add_test(tcase, test_get_default_string);
  tcase_add_test(tcase, test_get_default_enum);
  tcase_add_test(tcase, test_get_unaligned);
  tcase_add_test(tcase, test_get_unaligned_invalid);
  tcase_add_test(tcase, test_get_invalid);
  tcase_add_test(tcase, test_get_invalid_length);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "put" */
  tcase = tcase_create("put");
  tcase_add_test(tcase, test_put);
  tcase_add_test(tcase, test_put_uint32);
  tcase_add_test(tcase, test_put_uint64);
  tcase_add_test(tcase, test_put_int32);
  tcase_add_test(tcase, test_put_int64);
  tcase_add_test(tcase, test_put_bool);
  tcase_add_test(tcase, test_put_float);
  tcase_add_test(tcase, test_put_double);
  tcase_add_test(tcase, test_put_string);
  tcase_add_test(tcase, test_put_enum);
  tcase_add_test(tcase, test_put_string_existing);
  tcase_add_test(tcase, test_put_string_repeated);
  tcase_add_test(tcase, test_put_string_incremental);
  tcase_add_test(tcase, test_put_unaligned);
  tcase_add_test(tcase, test_put_unaligned_invalid);
  tcase_add_test(tcase, test_put_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "clear" */
  tcase = tcase_create("clear");
  tcase_add_test(tcase, test_clear);
  tcase_add_test(tcase, test_clear_string);
  tcase_add_test(tcase, test_clear_string_repeated);
  tcase_add_test(tcase, test_clear_string_unaligned);
  tcase_add_test(tcase, test_clear_unaligned);
  tcase_add_test(tcase, test_clear_unaligned_invalid);
  tcase_add_test(tcase, test_clear_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "raw" */
  tcase = tcase_create("raw");
  tcase_add_test(tcase, test_raw);
  tcase_add_test(tcase, test_raw_unaligned);
  tcase_add_test(tcase, test_raw_unaligned_invalid);
  tcase_add_test(tcase, test_raw_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "align" */
  tcase = tcase_create("align");
  tcase_add_test(tcase, test_align);
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
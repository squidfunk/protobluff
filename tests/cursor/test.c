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

static const float
default_float = 0.0001;

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

static const pb_message_descriptor_t
descriptor = { {
  (const pb_field_descriptor_t []){
    {  1, "F01", UINT32,  REPEATED },
    {  2, "F02", UINT64,  OPTIONAL },
    {  3, "F03", SINT32,  REPEATED },
    {  4, "F04", SINT64,  REPEATED },
    {  5, "F05", BOOL,    REPEATED },
    {  6, "F06", FLOAT,   OPTIONAL, NULL, &default_float },
    {  7, "F07", DOUBLE,  REPEATED },
    {  8, "F08", STRING,  REPEATED },
    {  9, "F09", MESSAGE, REPEATED, &descriptor },
    { 10, "F10", MESSAGE, OPTIONAL, &descriptor }
  }, 10 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a cursor over a message for a specific tag.
 */
START_TEST(test_create) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 1);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(1, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Assert same contents but different location */
  fail_unless(pb_message_equals(&message, pb_cursor_message(&cursor)));
  ck_assert_ptr_ne(&message, pb_cursor_message(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a cursor over a message for an absent field.
 */
START_TEST(test_create_absent) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a cursor over an empty message for a specific tag.
 */
START_TEST(test_create_message_empty) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 1);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a cursor over an invalid message for a specific tag.
 */
START_TEST(test_create_message_invalid) {
  pb_message_t message = pb_message_create_invalid();
  pb_cursor_t  cursor  = pb_cursor_create(&message, 1);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
} END_TEST

/*
 * Create a cursor without a tag, halting on every field.
 */
START_TEST(test_create_without_tag) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(1, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Assert same contents but different location */
  fail_unless(pb_message_equals(&message, pb_cursor_message(&cursor)));
  ck_assert_ptr_ne(&message, pb_cursor_message(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a cursor without a tag over an empty message.
 */
START_TEST(test_create_without_tag_message_empty) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a cursor without a tag over an invalid message.
 */
START_TEST(test_create_without_tag_message_invalid) {
  pb_message_t message = pb_message_create_invalid();
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
} END_TEST

/*
 * Create a cursor over a nested message for a branch of tags.
 */
START_TEST(test_create_nested) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &binary) };

  /* Create a hundred nested submessages */
  for (size_t m = 1; m < 101; m++)
    messages[m] = pb_message_create_within(&(messages[m - 1]), 10);

  /* Write value to innermost submessage */
  uint32_t value = 2000000000, check;
  for (size_t f = 0; f < 4; f++)
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[100]), 1, &value));

  /* Free all allocated memory */
  for (size_t m = 1; m < 101; m++)
    pb_message_destroy(&(messages[m]));

  /* Create tags */
  pb_tag_t tags[101] = {};
  for (size_t t = 0; t < 100; t++)
    tags[t] = 10;
  tags[100] = 1;

  /* Create cursor for fields within innermost message */
  pb_cursor_t cursor = pb_cursor_create_nested(&(messages[0]), tags, 101);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through message and read fields */
  for (size_t f = 0; f < 4; f++, pb_cursor_next(&cursor)) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &check));
    ck_assert_uint_eq(value, check);

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(1, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f, pb_cursor_pos(&cursor));
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&(messages[0]));
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a cursor over an invalid nested message for a branch of tags.
 */
START_TEST(test_create_nested_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Create tags */
  pb_tag_t tags[101] = {};
  for (size_t t = 0; t < 100; t++)
    tags[t] = 10;
  tags[100] = 1;

  /* Create cursor for fields within innermost message */
  pb_cursor_t cursor = pb_cursor_create_nested(&message, tags, 101);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
} END_TEST

/*
 * Create an invalid cursor.
 */
START_TEST(test_create_invalid) {
  pb_cursor_t cursor = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
} END_TEST

/*
 * Create a copy of a cursor.
 */
START_TEST(test_copy) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create binary, message and cursors */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 1);
  pb_cursor_t  copy    = pb_cursor_copy(&cursor);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&copy));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&copy));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(1, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&copy);
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Move a cursor to the next field.
 */
START_TEST(test_next) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through message and read fields */
  for (size_t f = 1, value; f < 5; f++, pb_cursor_next(&cursor)) {
    pb_field_t field = pb_field_create_from_cursor(&cursor);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
    ck_assert_uint_eq(f, value);

    /* Assert field validity and error */
    fail_unless(pb_field_valid(&field));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

    /* Assert field size and version */
    fail_if(pb_field_empty(&field));
    ck_assert_uint_eq(1, pb_field_size(&field));
    ck_assert_uint_eq(0, pb_field_version(&field));

    /* Assert field offsets */
    ck_assert_uint_eq(f * 2 - 1, pb_field_start(&field));
    ck_assert_uint_eq(f * 2, pb_field_end(&field));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(2, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f - 1, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(8, pb_binary_size(&binary));

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
 * Move a cursor to the next length-prefixed field.
 */
START_TEST(test_next_length) {
  const uint8_t data[] = { 74, 2, 16, 1, 74, 2, 16, 2 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 9);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through message and read submessages */
  for (size_t m = 1, value; m < 3; m++, pb_cursor_next(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&submessage, 2, &value));
    ck_assert_uint_eq(m, value);

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&message));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

    /* Assert submessage size and version */
    fail_if(pb_message_empty(&submessage));
    ck_assert_uint_eq(2, pb_message_size(&submessage));
    ck_assert_uint_eq(0, pb_message_version(&submessage));

    /* Assert submessage offsets */
    ck_assert_uint_eq(m * 4 - 2, pb_message_start(&submessage));
    ck_assert_uint_eq(m * 4, pb_message_end(&submessage));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(9, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(m - 1, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(8, pb_binary_size(&binary));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
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
 * Move an unaligned cursor to the next field.
 */
START_TEST(test_next_unaligned) {
  const uint8_t data[] = { 32, 1, 32, 2, 32, 3, 32, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 4);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through message and read fields */
  for (size_t f = 1, value; f < 5; f++, pb_cursor_next(&cursor)) {
    pb_field_t field = pb_field_create_from_cursor(&cursor);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
    ck_assert_uint_eq(f * 2, value);

    /* Write a field with a smaller tag to ensure misalignment */
    uint32_t value1 = 127;
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 1, &value1));

    /* Align field to perform checks */
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_align(&field));

    /* Assert field validity and error */
    fail_unless(pb_field_valid(&field));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

    /* Assert field size and version */
    fail_if(pb_field_empty(&field));
    ck_assert_uint_eq(1, pb_field_size(&field));
    ck_assert_uint_eq(f * 2, pb_field_version(&field));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(4, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f - 1, pb_cursor_pos(&cursor));

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
 * Move an invalid cursor to the next field.
 */
START_TEST(test_next_invalid) {
  pb_cursor_t cursor = pb_cursor_create_invalid();

  /* Move cursor to the next field */
  fail_if(pb_cursor_next(&cursor));

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
} END_TEST

/*
 * Move a cursor to the next field with invalid data.
 */
START_TEST(test_next_invalid_data) {
  const uint8_t data[] = { 53, 0, 0, 0 };
  const size_t  size   = 4;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 6);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_UNDERRUN, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Move a cursor to the next field with an invalid tag.
 */
START_TEST(test_next_invalid_tag) {
  const uint8_t data[] = { 128 };
  const size_t  size   = 1;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_UNDERRUN, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Move a cursor to the next field with an invalid length prefix.
 */
START_TEST(test_next_invalid_length) {
  const uint8_t data[] = { 74, 128 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_UNDERRUN, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Move a cursor to the next length-prefixed field with invalid data.
 */
START_TEST(test_next_invalid_length_data) {
  const uint8_t data[] = { 74, 130, 131, 132, 133, 134, 135, 136 };
  const size_t  size   = 7;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OVERFLOW, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Rewind a cursor to its initial position.
 */
START_TEST(test_rewind) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Use up and rewind cursor */
  while (pb_cursor_next(&cursor));
  fail_unless(pb_cursor_rewind(&cursor));

  /* Walk through message and read fields */
  for (size_t f = 1, value; f < 5; f++, pb_cursor_next(&cursor)) {
    pb_field_t field = pb_field_create_from_cursor(&cursor);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
    ck_assert_uint_eq(f, value);

    /* Assert field validity and error */
    fail_unless(pb_field_valid(&field));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

    /* Assert field size and version */
    fail_if(pb_field_empty(&field));
    ck_assert_uint_eq(1, pb_field_size(&field));
    ck_assert_uint_eq(0, pb_field_version(&field));

    /* Assert field offsets */
    ck_assert_uint_eq(f * 2 - 1, pb_field_start(&field));
    ck_assert_uint_eq(f * 2, pb_field_end(&field));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(2, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f - 1, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(8, pb_binary_size(&binary));

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
 * Rewind an unaligned cursor to its initial position.
 */
START_TEST(test_rewind_unaligned) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Write a field with a smaller tag to ensure misalignment */
  uint32_t value1 = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 1, &value1));

  /* Use up and rewind cursor */
  while (pb_cursor_next(&cursor));
  fail_unless(pb_cursor_rewind(&cursor));

  /* Seek value with cursor */
  uint64_t value2 = 3;
  fail_unless(pb_cursor_seek(&cursor, &value2));

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Seek value located before with cursor */
  value2 = 2;
  fail_if(pb_cursor_seek(&cursor, &value2));

  /* Assert cursor validity and error again */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(2, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(3, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Rewind an invalid cursor to its initial position.
 */
START_TEST(test_rewind_invalid) {
  pb_cursor_t cursor = pb_cursor_create_invalid();

  /* Use up and rewind cursor */
  while (pb_cursor_next(&cursor));
  fail_if(pb_cursor_rewind(&cursor));

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
} END_TEST

/*
 * Seek a cursor from its current position to a field containing the value.
 */
START_TEST(test_seek) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Seek value with cursor */
  uint64_t value = 3;
  fail_unless(pb_cursor_seek(&cursor, &value));
  fail_if(pb_cursor_seek(&cursor, &value));

  /* Assert cursor validity and error again */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(2, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(3, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Seek a cursor from its current position to a string field.
 */
START_TEST(test_seek_string) {
  const uint8_t data[] = { 66, 7, 68, 69, 70, 65, 85, 76, 84,
                           66, 7, 68, 69, 70, 65, 85, 76, 84 };
  const size_t  size   = 18;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 8);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Seek value with cursor */
  pb_string_t value = pb_string_init("DEFAULT");
  fail_unless(pb_cursor_seek(&cursor, &value));
  fail_if(pb_cursor_seek(&cursor, &value));

  /* Assert cursor validity and error again */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(8, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(1, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Seek an unaligned cursor to a field containing the value.
 */
START_TEST(test_seek_unaligned) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Write a field with a smaller tag to ensure misalignment */
  uint32_t value1 = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 1, &value1));

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Seek value with cursor */
  uint64_t value2 = 3;
  fail_unless(pb_cursor_seek(&cursor, &value2));
  fail_if(pb_cursor_seek(&cursor, &value2));

  /* Assert cursor validity and error again */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(2, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(3, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Seek an invalid cursor to a field containing the value.
 */
START_TEST(test_seek_invalid) {
  pb_cursor_t cursor = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Seek value with cursor */
  uint32_t value = 1;
  fail_if(pb_cursor_seek(&cursor, &value));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
} END_TEST

/*
 * Seek a cursor from its current position to a field with an invalid tag.
 */
START_TEST(test_seek_invalid_tag) {
  const uint8_t data[] = { 88, 1 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 11);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Seek value with cursor */
  uint32_t value = 1;
  fail_if(pb_cursor_seek(&cursor, &value));

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(11, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0,  pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Seek a cursor from its current position to a field of invalid type.
 */
START_TEST(test_seek_invalid_type) {
  const uint8_t data[] = { 74, 0 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Seek value with cursor */
  uint32_t value = 0;
  fail_if(pb_cursor_seek(&cursor, &value));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(9, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of the current field of a cursor with the given value.
 */
START_TEST(test_match) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Read value from cursor */
  for (size_t f = 1; f < 5; f++, pb_cursor_next(&cursor)) {
    fail_unless(pb_cursor_match(&cursor, &f));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(2, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f - 1, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(8, pb_binary_size(&binary));
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
 * Compare the value of a string field of a cursor with the given value.
 */
START_TEST(test_match_string) {
  const uint8_t data[] = { 66, 7, 68, 69, 70, 65, 85, 76, 84,
                           66, 7, 68, 69, 70, 65, 85, 76, 84 };
  const size_t  size   = 18;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 8);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Read value from cursor */
  for (size_t f = 0; f < 2; f++, pb_cursor_next(&cursor)) {
    pb_string_t value = pb_string_init("DEFAULT");
    fail_unless(pb_cursor_match(&cursor, &value));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(8, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(18, pb_binary_size(&binary));
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
 * Compare the value of the current field of an unaligned cursor.
 */
START_TEST(test_match_unaligned) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Create field at current cursor position */
  pb_field_t field = pb_field_create_from_cursor(&cursor);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Write value to field */
  uint64_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));

  /* Compare value from cursor */
  fail_unless(pb_cursor_match(&cursor, &value));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of the current field of an invalid cursor.
 */
START_TEST(test_match_invalid) {
  pb_cursor_t cursor = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Compare value from cursor */
  uint32_t value = 1;
  fail_if(pb_cursor_match(&cursor, &value));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
} END_TEST

/*
 * Compare the value of the current field with an invalid tag.
 */
START_TEST(test_match_invalid_tag) {
  const uint8_t data[] = { 88, 1 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Compare value from cursor */
  uint32_t value = 1;
  fail_if(pb_cursor_match(&cursor, &value));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(11, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0,  pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Compare the value of the current field with an invalid type.
 */
START_TEST(test_match_invalid_type) {
  const uint8_t data[] = { 74, 0 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Compare value from cursor */
  uint32_t value = 0;
  fail_if(pb_cursor_match(&cursor, &value));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(9, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value of the current field from a cursor.
 */
START_TEST(test_get) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Read value from cursor */
  for (size_t f = 1, value; f < 5; f++, pb_cursor_next(&cursor)) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &value));
    ck_assert_uint_eq(f, value);

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(2, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f - 1, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(8, pb_binary_size(&binary));
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
 * Read the value of the current string field from a cursor.
 */
START_TEST(test_get_string) {
  const uint8_t data[] = { 66, 7, 68, 69, 70, 65, 85, 76, 84,
                           66, 7, 68, 69, 70, 65, 85, 76, 84 };
  const size_t  size   = 18;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 8);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Read value from cursor */
  for (size_t f = 0; f < 2; f++, pb_cursor_next(&cursor)) {
    pb_string_t value1 = pb_string_init("DEFAULT"), value2;
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &value2));
    fail_if(memcmp(value2.data, value1.data, value2.size));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(8, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(18, pb_binary_size(&binary));
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
 * Read the value of the current field from an unaligned cursor.
 */
START_TEST(test_get_unaligned) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Create field at current cursor position */
  pb_field_t field = pb_field_create_from_cursor(&cursor);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Write value to field */
  uint64_t value = 127, check = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));

  /* Read value from cursor */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &check));
  ck_assert_uint_eq(value, check);

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value of the current field from an invalid cursor.
 */
START_TEST(test_get_invalid) {
  pb_cursor_t cursor = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Read value from cursor */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_get(&cursor, &value));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
} END_TEST

/*
 * Read the value of the current field with an invalid tag from a cursor.
 */
START_TEST(test_get_invalid_tag) {
  const uint8_t data[] = { 88, 1 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Read value from cursor */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_DESCRIPTOR, pb_cursor_get(&cursor, &value));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(11, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0,  pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read the value of the current field of invalid type from a cursor.
 */
START_TEST(test_get_invalid_type) {
  const uint8_t data[] = { 74, 0 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Read value from cursor */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_get(&cursor, &value));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(9, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a value to the current field of a cursor.
 */
START_TEST(test_put) {
  const uint8_t data[] = { 16, 0, 16, 0, 16, 0, 16, 0 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Write values to cursor */
  for (size_t f = 1, value; f < 5; f++, pb_cursor_next(&cursor)) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_put(&cursor, &f));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Read value from cursor */
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &value));
    ck_assert_uint_eq(f, value);

    /* Assert cursor tag and position */
    ck_assert_uint_eq(2, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f - 1, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(8, pb_binary_size(&binary));
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
 * Write a value to the current field of an unaligned cursor.
 */
START_TEST(test_put_unaligned) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Create field at current cursor position */
  pb_field_t field = pb_field_create_from_cursor(&cursor);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Write value to field */
  uint64_t value = 127, check = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));

  /* Write value to cursor */
  value = 65535;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_put(&cursor, &value));

  /* Read value from cursor */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &check));
  ck_assert_uint_eq(value, check);

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a value to the current string field of a cursor.
 */
START_TEST(test_put_string) {
  const uint8_t data[] = { 66, 7, 68, 69, 70, 65, 85, 76, 84,
                           66, 7, 68, 69, 70, 65, 85, 76, 84 };
  const size_t  size   = 18;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 8);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Write values to cursor */
  for (size_t f = 0; f < 2; f++, pb_cursor_next(&cursor)) {
    pb_string_t value = pb_string_init("TEST");
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_put(&cursor, &value));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(8, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary));
    ck_assert_uint_eq(18 - (f + 1) * 3, pb_binary_size(&binary));
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
 * Write a message to the current message field of a cursor.
 */
START_TEST(test_put_message) {
  const uint8_t data[] = { 74, 2, 16, 0, 74, 2, 16, 1 };
  const size_t  size   = 8;

  /* Create binaries and messages */
  pb_binary_t  binary1 = pb_binary_create_empty();
  pb_binary_t  binary2 = pb_binary_create(data, size);
  pb_message_t message1 = pb_message_create(&descriptor, &binary1);
  pb_message_t message2 = pb_message_create(&descriptor, &binary2);

  /* Write value to first message and first message to second */
  pb_string_t value = pb_string_init("OVERWRITE");
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message1, 8, &value));

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message2, 9);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Write values to cursor */
  for (size_t f = 0; f < 2; f++, pb_cursor_next(&cursor)) {
    ck_assert_uint_eq(0, pb_cursor_put(&cursor, &message1));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(9, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f, pb_cursor_pos(&cursor));

    /* Assert binary size */
    fail_if(pb_binary_empty(&binary2));
    ck_assert_uint_eq(8 + (f + 1) * 9, pb_binary_size(&binary2));
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_binary_destroy(&binary2);
  pb_binary_destroy(&binary1);
} END_TEST

/*
 * Write an invalid message to the current message field of a cursor.
 */
START_TEST(test_put_message_invalid) {
  const uint8_t data[] = { 74, 2, 16, 0, 74, 2, 16, 1 };
  const size_t  size   = 8;

  /* Create binary and messages */
  pb_binary_t  binary = pb_binary_create(data, size);
  pb_message_t message1 = pb_message_create_invalid();
  pb_message_t message2 = pb_message_create(&descriptor, &binary);

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message2, 9);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Write values to cursor */
  for (size_t f = 0; f < 2; f++, pb_cursor_next(&cursor)) {
    ck_assert_uint_eq(PB_ERROR_INVALID,
      pb_cursor_put(&cursor, &message1));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(9, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f, pb_cursor_pos(&cursor));
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_OFFSET, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Write a value to the current field of an invalid cursor.
 */
START_TEST(test_put_invalid) {
  pb_cursor_t cursor = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Write value to cursor */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_put(&cursor, &value));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
} END_TEST

/*
 * Write a value to the current field with an invalid tag of a cursor.
 */
START_TEST(test_put_invalid_tag) {
  const uint8_t data[] = { 88, 1 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Write value to cursor */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_DESCRIPTOR, pb_cursor_put(&cursor, &value));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(11, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0,  pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Erase the current field from a cursor.
 */
START_TEST(test_erase) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Write value to cursor */
  for (size_t f = 1, value; f < 5; f++, pb_cursor_next(&cursor)) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &value));
    ck_assert_uint_eq(f, value);

    /* Clear field */
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_erase(&cursor));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(2, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f - 1, pb_cursor_pos(&cursor));

    /* Assert binary size */
    ck_assert_uint_eq(8 - f * 2, pb_binary_size(&binary));
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Erase the current string field from a cursor.
 */
START_TEST(test_erase_string) {
  const uint8_t data[] = { 66, 7, 68, 69, 70, 65, 85, 76, 84,
                           66, 7, 68, 69, 70, 65, 85, 76, 84 };
  const size_t  size   = 18;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 8);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Clear value from cursor */
  for (size_t f = 0; f < 2; f++, pb_cursor_next(&cursor)) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_erase(&cursor));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(8, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f, pb_cursor_pos(&cursor));

    /* Assert binary size */
    ck_assert_uint_eq(18 - (f + 1) * 9, pb_binary_size(&binary));
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Erase the current submessage from a cursor.
 */
START_TEST(test_erase_message) {
  const uint8_t data[] = { 74, 0, 74, 0, 74, 0, 74, 0 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Clear value from cursor */
  for (size_t f = 1; f < 5; f++, pb_cursor_next(&cursor)) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_erase(&cursor));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Assert cursor tag and position */
    ck_assert_uint_eq(9, pb_cursor_tag(&cursor));
    ck_assert_uint_eq(f - 1, pb_cursor_pos(&cursor));

    /* Assert binary size */
    ck_assert_uint_eq(8 - f * 2, pb_binary_size(&binary));
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Erase the current field from an unaligned cursor.
 */
START_TEST(test_erase_unaligned) {
  const uint8_t data[] = { 16, 1, 16, 2, 16, 3, 16, 4 };
  const size_t  size   = 8;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create(&message, 2);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Create field at current cursor position */
  pb_field_t field = pb_field_create_from_cursor(&cursor);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Write value to field */
  uint64_t value = 127, check = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));

  /* Clear value from cursor */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_erase(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_get(&cursor, &check));

  /* Move cursor to the next field */
  fail_unless(pb_cursor_next(&cursor));

  /* Read next value from cursor */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &check));
  ck_assert_uint_eq(2, check);

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Erase the current field or submessage from an invalid cursor.
 */
START_TEST(test_erase_invalid) {
  pb_cursor_t cursor = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Clear value from cursor */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_erase(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
} END_TEST

/*
 * Erase the current field or submessage with an invalid tag from a cursor.
 */
START_TEST(test_erase_invalid_tag) {
  const uint8_t data[] = { 88, 1 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Clear value from cursor */
  ck_assert_uint_eq(PB_ERROR_DESCRIPTOR, pb_cursor_erase(&cursor));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(11, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0,  pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Retrieve a pointer to the raw data of the current field from a cursor.
 */
START_TEST(test_raw) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 6);

  /* Read default value from field */
  float value = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value));
  fail_if(memcmp(&default_float, &value, sizeof(float)));

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 6);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Obtain and change raw data */
  float *raw = pb_cursor_raw(&cursor);
  ck_assert_ptr_ne(NULL, raw);
  *raw = 0.123456789;

  /* Read new value from cursor */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &value));
  fail_if(memcmp(raw, &value, sizeof(float)));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(6, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Retrieve a pointer from an unaligned cursor.
 */
START_TEST(test_raw_unaligned) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field   = pb_field_create(&message, 6);

  /* Read default value from field */
  float value1 = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_get(&field, &value1));
  fail_if(memcmp(&default_float, &value1, sizeof(float)));

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 6);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Write a field with a smaller tag to ensure misalignment */
  uint32_t value2 = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 2, &value2));

  /* Obtain and change raw data */
  float *raw = pb_cursor_raw(&cursor);
  ck_assert_ptr_ne(NULL, raw);
  *raw = 0.123456789;

  /* Read new value from cursor */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &value1));
  fail_if(memcmp(raw, &value1, sizeof(float)));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(6, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Retrieve a pointer from an invalid cursor.
 */
START_TEST(test_raw_invalid) {
  pb_cursor_t cursor = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Obtain raw data */
  float *raw = pb_cursor_raw(&cursor);
  ck_assert_ptr_eq(NULL, raw);

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
} END_TEST

/*
 * Retrieve a pointer for a field with an invalid tag from a cursor.
 */
START_TEST(test_raw_invalid_tag) {
  const uint8_t data[] = { 88, 1 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Obtain raw data */
  ck_assert_ptr_eq(NULL, pb_cursor_raw(&cursor));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(11, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0,  pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Retrieve a pointer for a field with an invalid type from a cursor.
 */
START_TEST(test_raw_invalid_type) {
  const uint8_t data[] = { 74, 0 };
  const size_t  size   = 2;

  /* Create binary, message and cursor */
  pb_binary_t  binary  = pb_binary_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Obtain raw data */
  ck_assert_ptr_eq(NULL, pb_cursor_raw(&cursor));

  /* Assert cursor validity and error again */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(9, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Ensure that a cursor is properly aligned.
 */
START_TEST(test_align) {
  pb_binary_t  binary  = pb_binary_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &binary);
  pb_field_t   field1  = pb_field_create(&message, 8);

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 8);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Write value to cursor */
  pb_string_t value = pb_string_init(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam euismod "
    "vehicula nibh, et egestas erat eleifend quis. Nam hendrerit egestas "
    "quam nec egestas. Donec lacinia vestibulum erat, ac suscipit nisi "
    "vehicula nec. Praesent ullamcorper vitae lorem vel euismod. Quisque "
    "fringilla lobortis convallis. Aliquam accumsan lacus eu viverra dapibus. "
    "Phasellus in adipiscing sem, in congue massa. Vestibulum ullamcorper "
    "orci nec semper pretium.");
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field1, &value));

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field1));

  /* Grab another field reference */
  pb_field_t field2 = pb_field_create_from_cursor(&cursor);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field2));

  /* Assert field size and version */
  fail_if(pb_field_empty(&field1));
  ck_assert_uint_eq(437, pb_field_size(&field1));
  ck_assert_uint_eq(3, pb_field_version(&field1));

  /* Assert alignment */
  ck_assert_uint_eq(3, pb_field_version(&field2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_align(&field2));
  fail_unless(pb_field_equals(&field1, &field2));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(8, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_field_destroy(&field2);
  pb_field_destroy(&field1);
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
  void *suite = suite_create("protobluff/cursor"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_absent);
  tcase_add_test(tcase, test_create_message_empty);
  tcase_add_test(tcase, test_create_message_invalid);
  tcase_add_test(tcase, test_create_without_tag);
  tcase_add_test(tcase, test_create_without_tag_message_empty);
  tcase_add_test(tcase, test_create_without_tag_message_invalid);
  tcase_add_test(tcase, test_create_nested);
  tcase_add_test(tcase, test_create_nested_invalid);
  tcase_add_test(tcase, test_create_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "copy" */
  tcase = tcase_create("copy");
  tcase_add_test(tcase, test_copy);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "next" */
  tcase = tcase_create("next");
  tcase_add_test(tcase, test_next);
  tcase_add_test(tcase, test_next_length);
  tcase_add_test(tcase, test_next_unaligned);
  tcase_add_test(tcase, test_next_invalid);
  tcase_add_test(tcase, test_next_invalid_data);
  tcase_add_test(tcase, test_next_invalid_tag);
  tcase_add_test(tcase, test_next_invalid_length);
  tcase_add_test(tcase, test_next_invalid_length_data);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "rewind" */
  tcase = tcase_create("rewind");
  tcase_add_test(tcase, test_rewind);
  tcase_add_test(tcase, test_rewind_unaligned);
  tcase_add_test(tcase, test_rewind_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "seek" */
  tcase = tcase_create("seek");
  tcase_add_test(tcase, test_seek);
  tcase_add_test(tcase, test_seek_string);
  tcase_add_test(tcase, test_seek_unaligned);
  tcase_add_test(tcase, test_seek_invalid);
  tcase_add_test(tcase, test_seek_invalid_tag);
  tcase_add_test(tcase, test_seek_invalid_type);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "match" */
  tcase = tcase_create("match");
  tcase_add_test(tcase, test_match);
  tcase_add_test(tcase, test_match_string);
  tcase_add_test(tcase, test_match_unaligned);
  tcase_add_test(tcase, test_match_invalid);
  tcase_add_test(tcase, test_match_invalid_tag);
  tcase_add_test(tcase, test_match_invalid_type);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "get" */
  tcase = tcase_create("get");
  tcase_add_test(tcase, test_get);
  tcase_add_test(tcase, test_get_string);
  tcase_add_test(tcase, test_get_unaligned);
  tcase_add_test(tcase, test_get_invalid);
  tcase_add_test(tcase, test_get_invalid_tag);
  tcase_add_test(tcase, test_get_invalid_type);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "put" */
  tcase = tcase_create("put");
  tcase_add_test(tcase, test_put);
  tcase_add_test(tcase, test_put_string);
  tcase_add_test(tcase, test_put_message);
  tcase_add_test(tcase, test_put_message_invalid);
  tcase_add_test(tcase, test_put_unaligned);
  tcase_add_test(tcase, test_put_invalid);
  tcase_add_test(tcase, test_put_invalid_tag);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "erase" */
  tcase = tcase_create("erase");
  tcase_add_test(tcase, test_erase);
  tcase_add_test(tcase, test_erase_string);
  tcase_add_test(tcase, test_erase_message);
  tcase_add_test(tcase, test_erase_unaligned);
  tcase_add_test(tcase, test_erase_invalid);
  tcase_add_test(tcase, test_erase_invalid_tag);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "raw" */
  tcase = tcase_create("raw");
  tcase_add_test(tcase, test_raw);
  tcase_add_test(tcase, test_raw_unaligned);
  tcase_add_test(tcase, test_raw_invalid);
  tcase_add_test(tcase, test_raw_invalid_tag);
  tcase_add_test(tcase, test_raw_invalid_type);
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
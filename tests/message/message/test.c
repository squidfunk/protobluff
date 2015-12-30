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

#include "core/varint.h"
#include "message/common.h"
#include "message/cursor.h"
#include "message/field.h"
#include "message/journal.h"
#include "message/message.h"
#include "util/descriptor.h"

/* ----------------------------------------------------------------------------
 * Defaults
 * ------------------------------------------------------------------------- */

static const uint32_t
default_uint32 = 1000000000U;

static const uint64_t
default_uint64 = 1000000000000000000ULL;

static const int32_t
default_int32 = -1000000000;

static const int64_t
default_int64 = -1000000000000000000LL;

static const uint8_t
default_bool =  0;

static const float
default_float = 0.0001;

static const double
default_double = 0.00000001;

static const pb_string_t
default_string = pb_string_const("DEFAULT");

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

/* Descriptor */
static pb_descriptor_t
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
    {  9, "F09", BYTES,   OPTIONAL },
    { 10, "F10", UINT32,  OPTIONAL },
    { 11, "F11", MESSAGE, OPTIONAL, &descriptor },
    { 12, "F12", MESSAGE, REPEATED, &descriptor }
  }, 12 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a message from a descriptor and a journal.
 */
START_TEST(test_create) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert same locations */
  ck_assert_ptr_eq(&journal, pb_message_journal(&message));
  ck_assert_ptr_eq(&descriptor, pb_message_descriptor(&message));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(2, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(2, pb_message_end(&message));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a message from a descriptor and an empty journal.
 */
START_TEST(test_create_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(0, pb_message_end(&message));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a submessage within a message for a specific tag.
 */
START_TEST(test_create_within) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_message_t submessage = pb_message_create_within(&message, 11);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(2, pb_message_size(&message));
  ck_assert_uint_eq(1, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(2, pb_message_end(&message));

  /* Assert submessage validity and error */
  fail_unless(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

  /* Assert submessage size and version */
  fail_unless(pb_message_empty(&submessage));
  ck_assert_uint_eq(0, pb_message_size(&submessage));
  ck_assert_uint_eq(1, pb_message_version(&submessage));

  /* Assert submessage offsets */
  ck_assert_uint_eq(2, pb_message_start(&submessage));
  ck_assert_uint_eq(2, pb_message_end(&submessage));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create an existing submessage within a message for a specific tag.
 */
START_TEST(test_create_within_existing) {
  const uint8_t data[] = { 90, 0 };
  const size_t  size   = 2;

  /* Create journal, message and submessage */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_message_t submessage = pb_message_create_within(&message, 11);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(2, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(2, pb_message_end(&message));

  /* Assert submessage validity and error */
  fail_unless(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

  /* Assert submessage size and version */
  fail_unless(pb_message_empty(&submessage));
  ck_assert_uint_eq(0, pb_message_size(&submessage));
  ck_assert_uint_eq(0, pb_message_version(&submessage));

  /* Assert submessage offsets */
  ck_assert_uint_eq(2, pb_message_start(&submessage));
  ck_assert_uint_eq(2, pb_message_end(&submessage));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create submessage siblings within a message for a specific tag.
 */
START_TEST(test_create_within_siblings) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create a hundred submessage siblings and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;

    /* Create submessage and write value */
    pb_message_t submessage = pb_message_create_within(&message, 12);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&submessage, 2, &value));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 12);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through submessages and read fields */
  for (size_t m = 1; m < 101; m++, pb_cursor_next(&cursor)) {
    uint64_t value = m;

    /* Create submessage and write value */
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&submessage, 2, &value));
    ck_assert_uint_eq(m, value);

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

    /* Assert submessage size and version */
    fail_if(pb_message_empty(&submessage));
    ck_assert_uint_eq(2, pb_message_size(&submessage));
    ck_assert_uint_eq(300, pb_message_version(&submessage));

    /* Assert submessage offsets */
    ck_assert_uint_eq(m * 4 - 2, pb_message_start(&submessage));
    ck_assert_uint_eq(m * 4, pb_message_end(&submessage));

    /* Assert journal size */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(400, pb_journal_size(&journal));

    /* Alter value of the field */
    value = 101 - m;
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&submessage, 2, &value));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Assert journal contents */
  for (size_t m = 1; m < 101; m++)
    ck_assert_uint_eq(101 - m, pb_journal_data_at(&journal, m * 4 - 1));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create empty submessage siblings within a message for a specific tag.
 */
START_TEST(test_create_within_siblings_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create a hundred empty submessage siblings */
  for (size_t m = 1; m < 101; m++) {
    pb_message_t submessage = pb_message_create_within(&message, 12);

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 12);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through submessages and read fields */
  for (size_t m = 1; m < 101; m++, pb_cursor_next(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

    /* Assert submessage size and version */
    fail_unless(pb_message_empty(&submessage));
    ck_assert_uint_eq(0, pb_message_size(&submessage));
    ck_assert_uint_eq(100, pb_message_version(&submessage));

    /* Assert submessage offsets */
    ck_assert_uint_eq(m * 2, pb_message_start(&submessage));
    ck_assert_uint_eq(m * 2, pb_message_end(&submessage));

    /* Assert journal size */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(200, pb_journal_size(&journal));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create submessage siblings with strings within a message for a specific tag.
 */
START_TEST(test_create_within_siblings_strings) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create a hundred submessage siblings and write strings to them */
  pb_string_t value = pb_string_init_from_chars("DATA"), check;
  for (size_t m = 1; m < 101; m++) {
    pb_message_t submessage = pb_message_create_within(&message, 12);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&submessage, 8, &value));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 12);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through submessages and read fields */
  for (size_t m = 1; m < 101; m++, pb_cursor_next(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_get(&submessage, 8, &check));
    fail_unless(pb_string_equals(&value, &check));

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

    /* Assert submessage size and version */
    fail_if(pb_message_empty(&submessage));
    ck_assert_uint_eq(6, pb_message_size(&submessage));
    ck_assert_uint_eq(300, pb_message_version(&submessage));

    /* Assert submessage offsets */
    ck_assert_uint_eq(m * 8 - 6, pb_message_start(&submessage));
    ck_assert_uint_eq(m * 8, pb_message_end(&submessage));

    /* Assert journal size */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(800, pb_journal_size(&journal));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create nested submessages within a message for a specific tag.
 */
START_TEST(test_create_within_nested) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;

    /* Create submessage and write value */
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Free all allocated memory */
  for (size_t m = 1; m < 101; m++)
    pb_message_destroy(&(messages[m]));

  /* Walk through nested submessages */
  size_t start = 0, end = pb_journal_size(&journal); uint64_t value;
  for (size_t m = 1; m < 101; m++) {
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_get(&(messages[m]), 2, &value));
    ck_assert_uint_eq(m, value);

    /* Calculate length prefix size */
    uint32_t length = pb_message_size(&(messages[m]));
    start += 1 + pb_varint_size_uint32(&length) + (m != 1) * 2;

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&(messages[m])));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[m])));

    /* Assert submessage size and version */
    ck_assert_uint_eq(end - start, pb_message_size(&(messages[m])));
    ck_assert_uint_eq(368, pb_message_version(&(messages[m])));

    /* Assert submessage offsets */
    ck_assert_uint_eq(start, pb_message_start(&(messages[m])));
    ck_assert_uint_eq(end, pb_message_end(&(messages[m])));
  }

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create empty nested submessages within a message for a specific tag.
 */
START_TEST(test_create_within_nested_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred empty nested submessages */
  for (size_t m = 1; m < 101; m++)
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);

  /* Free all allocated memory */
  for (size_t m = 1; m < 101; m++)
    pb_message_destroy(&(messages[m]));

  /* Walk through nested submessages */
  size_t start = 0, end = pb_journal_size(&journal);
  for (size_t m = 1; m < 101; m++) {
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);

    /* Calculate length prefix size */
    uint32_t length = pb_message_size(&(messages[m]));
    start += 1 + pb_varint_size_uint32(&length);

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&(messages[m])));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[m])));

    /* Assert submessage size and version */
    ck_assert_uint_eq(end - start, pb_message_size(&(messages[m])));
    ck_assert_uint_eq(136, pb_message_version(&(messages[m])));

    /* Assert submessage offsets */
    ck_assert_uint_eq(start, pb_message_start(&(messages[m])));
    ck_assert_uint_eq(end, pb_message_end(&(messages[m])));
  }

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create nested submessages with strings within a message for a specific tag.
 */
START_TEST(test_create_within_nested_strings) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write strings to them */
  pb_string_t value = pb_string_init_from_chars("DATA"), check;
  for (size_t m = 1; m < 101; m++) {
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 8, &value));
  }

  /* Free all allocated memory */
  for (size_t m = 1; m < 101; m++)
    pb_message_destroy(&(messages[m]));

  /* Walk through nested submessages */
  size_t start = 0, end = pb_journal_size(&journal);
  for (size_t m = 1; m < 101; m++) {
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_get(&(messages[m]), 8, &check));
    fail_if(memcmp(value.data, check.data, value.size));

    /* Calculate length prefix size */
    uint32_t length = pb_message_size(&(messages[m]));
    start += 1 + pb_varint_size_uint32(&length) + (m != 1) * 6;

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&(messages[m])));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[m])));

    /* Assert submessage size and version */
    ck_assert_uint_eq(end - start, pb_message_size(&(messages[m])));
    ck_assert_uint_eq(384, pb_message_version(&(messages[m])));

    /* Assert submessage offsets */
    ck_assert_uint_eq(start, pb_message_start(&(messages[m])));
    ck_assert_uint_eq(end, pb_message_end(&(messages[m])));
  }

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a submessage within an invalid message for a specific tag.
 */
START_TEST(test_create_within_invalid) {
  pb_message_t message = pb_message_create_invalid();
  pb_message_t submessage = pb_message_create_within(&message, 11);

  /* Assert submessage validity and error */
  fail_if(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&submessage));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
} END_TEST

/*
 * Create a message within a nested message for a branch of tags.
 */
START_TEST(test_create_nested) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;

    /* Create submessage and write value */
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Free all allocated memory */
  for (size_t m = 2; m < 101; m++)
    pb_message_destroy(&(messages[m]));

  /* Create tags */
  pb_tag_t tags[100] = {};
  for (size_t t = 0; t < 100; t++)
    tags[t] = 11;

  /* Walk through nested submessages */
  uint64_t value;
  for (size_t m = 2; m < 101; m++) {
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_get(&(messages[m]), 2, &value));
    ck_assert_uint_eq(m, value);

    /* Create same submessage from root message */
    pb_message_t submessage = pb_message_create_nested(
      &(messages[0]), tags, m);
    fail_unless(pb_message_equals(&(messages[m]), &submessage));

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&(messages[m])));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[m])));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a submessage within a message for a specific tag.
 */
START_TEST(test_create_nested_within) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_message_t submessage = pb_message_create_nested(&message,
    (const pb_tag_t []){ 11 }, 1);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(2, pb_message_size(&message));
  ck_assert_uint_eq(1, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(2, pb_message_end(&message));

  /* Assert submessage validity and error */
  fail_unless(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

  /* Assert submessage size and version */
  fail_unless(pb_message_empty(&submessage));
  ck_assert_uint_eq(0, pb_message_size(&submessage));
  ck_assert_uint_eq(1, pb_message_version(&submessage));

  /* Assert submessage offsets */
  ck_assert_uint_eq(2, pb_message_start(&submessage));
  ck_assert_uint_eq(2, pb_message_end(&submessage));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a repeated message within a nested message for a branch of tags.
 */
START_TEST(test_create_nested_repeated) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;

    /* Create submessage and write value */
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 12;

  /* Create ten submessage siblings in the innermost message */
  for (size_t m = 0; m < 10; m++) {
    pb_message_t submessage =
      pb_message_create_nested(&(messages[0]), tags, 102);

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Create cursor for submessages within innermost message */
  pb_cursor_t cursor = pb_cursor_create_nested(&(messages[0]), tags, 102);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through submessages */
  for (size_t m = 0; m < 10; m++, pb_cursor_next(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a message within an invalid nested message.
 */
START_TEST(test_create_nested_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Create tags */
  pb_tag_t tags[100] = {};
  for (size_t t = 0; t < 100; t++)
    tags[t] = 11;

  /* Walk through nested submessages */
  for (size_t m = 2; m < 101; m++) {
    pb_message_t submessage = pb_message_create_nested(&message, tags, m);

    /* Assert submessage validity and error */
    fail_if(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&submessage));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Create a message at the current position of a cursor.
 */
START_TEST(test_create_from_cursor) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create a hundred submessage siblings and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;

    /* Create submessage and write value */
    pb_message_t submessage = pb_message_create_within(&message, 12);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&submessage, 2, &value));

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 12);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through submessages and read fields */
  uint64_t value;
  for (size_t m = 1; m < 101; m++, pb_cursor_next(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_get(&submessage, 2, &value));
    ck_assert_uint_eq(m, value);

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

    /* Assert submessage size and version */
    fail_if(pb_message_empty(&submessage));
    ck_assert_uint_eq(2, pb_message_size(&submessage));
    ck_assert_uint_eq(300, pb_message_version(&submessage));

    /* Assert submessage offsets */
    ck_assert_uint_eq(m * 4 - 2, pb_message_start(&submessage));
    ck_assert_uint_eq(m * 4, pb_message_end(&submessage));

    /* Assert journal size */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(400, pb_journal_size(&journal));

    /* Alter value of the field */
    value = 101 - m;
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&submessage, 2, &value));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Assert journal contents */
  for (size_t m = 1; m < 101; m++)
    ck_assert_uint_eq(101 - m, pb_journal_data_at(&journal, m * 4 - 1));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a message at the current position of an invalid cursor.
 */
START_TEST(test_create_from_cursor_invalid) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_cursor_t  cursor  = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Create submessage from cursor */
  pb_message_t submessage = pb_message_create_from_cursor(&cursor);

  /* Assert submessage validity and error */
  fail_if(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&submessage));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a submessage with an invalid tag from a cursor.
 */
START_TEST(test_create_from_cursor_invalid_tag) {
  const uint8_t data[] = { 106, 0 };
  const size_t  size   = 2;

  /* Create journal, message and cursor */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(0, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Create submessage for invalid tag */
  pb_message_t submessage = pb_message_create_from_cursor(&cursor);

  /* Assert submessage validity and error */
  fail_if(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&submessage));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a submessage with an invalid wiretype from a cursor.
 */
START_TEST(test_create_from_cursor_invalid_wiretype) {
  const uint8_t data[] = { 8, 0 };
  const size_t  size   = 2;

  /* Create journal, message and cursor */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_cursor_t  cursor  = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Assert cursor tag and position */
  ck_assert_uint_eq(1, pb_cursor_tag(&cursor));
  ck_assert_uint_eq(0, pb_cursor_pos(&cursor));

  /* Create submessage for invalid wiretype */
  pb_message_t submessage = pb_message_create_from_cursor(&cursor);

  /* Assert submessage validity and error */
  fail_if(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&submessage));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a message from a descriptor and a bytes field.
 */
START_TEST(test_create_from_field) {
  const uint8_t data[] = { 74, 2, 8, 127 };
  const size_t  size   = 4;

  /* Create journal, message and field */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_field_t   field   = pb_field_create(&message, 9);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Create submessage from bytes field */
  pb_message_t submessage = pb_message_create_from_field(&descriptor, &field);

  /* Assert submessage validity and error */
  fail_unless(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

  /* Assert submessage size and version */
  fail_if(pb_message_empty(&submessage));
  ck_assert_uint_eq(2, pb_message_size(&submessage));
  ck_assert_uint_eq(0, pb_message_version(&submessage));

  /* Assert submessage offsets */
  ck_assert_uint_eq(2, pb_message_start(&submessage));
  ck_assert_uint_eq(4, pb_message_end(&submessage));

  /* Assert field value */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&submessage, 1, &value));
  ck_assert_uint_eq(127, value);

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a message from a descriptor and an empty bytes field.
 */
START_TEST(test_create_from_field_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_field_t   field   = pb_field_create(&message, 9);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Create submessage from bytes field */
  pb_message_t submessage = pb_message_create_from_field(&descriptor, &field);

  /* Assert submessage validity and error */
  fail_unless(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

  /* Assert submessage size and version */
  fail_unless(pb_message_empty(&submessage));
  ck_assert_uint_eq(0, pb_message_size(&submessage));
  ck_assert_uint_eq(1, pb_message_version(&submessage));

  /* Assert submessage offsets */
  ck_assert_uint_eq(2, pb_message_start(&submessage));
  ck_assert_uint_eq(2, pb_message_end(&submessage));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_field_destroy(&field);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a message from a descriptor and an invalid bytes field.
 */
START_TEST(test_create_from_field_invalid) {
  pb_field_t field = pb_field_create_invalid();

  /* Assert field validity and error */
  fail_if(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_field_error(&field));

  /* Create submessage from bytes field */
  pb_message_t submessage = pb_message_create_from_field(&descriptor, &field);

  /* Assert submessage validity and error */
  fail_if(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&submessage));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_field_destroy(&field);
} END_TEST

/*
 * Create an invalid message.
 */
START_TEST(test_create_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Create a copy of a message.
 */
START_TEST(test_copy) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal, message and cursors */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_message_t copy    = pb_message_copy(&message);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&copy));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&copy));

  /* Assert same contents */
  fail_unless(pb_message_equals(&message, &copy));

  /* Free all allocated memory */
  pb_message_destroy(&copy);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Test whether a message contains at least one occurrence for a given tag.
 */
START_TEST(test_has) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert field (non-)existence */
  fail_unless(pb_message_has(&message, 1));
  fail_if(pb_message_has(&message, 2));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Test whether an empty message contains at least one occurrence.
 */
START_TEST(test_has_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert field (non-)existence */
  fail_if(pb_message_has(&message, 1));
  fail_if(pb_message_has(&message, 2));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Test whether a message contains at least one repeated occurrence.
 */
START_TEST(test_has_repeated) {
  const uint8_t data[] = { 98, 0, 98, 0 };
  const size_t  size   = 4;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert field (non-)existence */
  fail_unless(pb_message_has(&message, 12));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Test whether an unaligned message contains at least one occurrence.
 */
START_TEST(test_has_unaligned) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Write value to message */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 1, &value));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert field (non-)existence */
  fail_unless(pb_message_has(&message, 1));
  fail_if(pb_message_has(&message, 2));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Test whether an invalid message contains at least one occurrence.
 */
START_TEST(test_has_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Assert field (non-)existence */
  fail_if(pb_message_has(&message, 1));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Compare the value for a given tag from a message with the given value.
 */
START_TEST(test_match) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Compare value with value from message */
  uint32_t value1 = 127, value2 = 0;
  fail_unless(pb_message_match(&message, 1, &value1));
  fail_if(pb_message_match(&message, 2, &value1));
  fail_if(pb_message_match(&message, 1, &value2));
  fail_if(pb_message_match(&message, 2, &value2));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Compare the value from an empty message with the given value.
 */
START_TEST(test_match_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Compare value with value from message */
  uint32_t value = 127;
  fail_if(pb_message_match(&message, 1, &value));
  fail_if(pb_message_match(&message, 2, &value));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Compare the value for a given tag from a message with the given value.
 */
START_TEST(test_match_merged) {
  const uint8_t data[] = { 8, 1, 8, 2, 8, 3, 8, 4 };
  const size_t  size   = 8;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Compare value with value from message */
  uint32_t value1 = 1, value2 = 4;
  fail_if(pb_message_match(&message, 1, &value1));
  fail_unless(pb_message_match(&message, 1, &value2));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Compare the value from an unaligned message with the given value.
 */
START_TEST(test_match_unaligned) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Create field */
  pb_field_t field = pb_field_create(&message, 1);

  /* Assert field validity and error */
  fail_unless(pb_field_valid(&field));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_error(&field));

  /* Write value to field */
  uint64_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_field_put(&field, &value));

  /* Compare value with value from message */
  fail_unless(pb_message_match(&message, 1, &value));
  fail_if(pb_message_match(&message, 2, &value));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Compare the value from an invalid message with the given value.
 */
START_TEST(test_match_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Compare value with value from message */
  uint32_t value = 127;
  fail_if(pb_message_match(&message, 1, &value));
  fail_if(pb_message_match(&message, 2, &value));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Read the value for a given tag from a message.
 */
START_TEST(test_get) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Read value from message */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 1, &value));
  ck_assert_uint_eq(127, value);

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(2, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the value for a given tag from a merged message.
 */
START_TEST(test_get_merged) {
  const uint8_t data[] = { 8, 1, 8, 2, 8, 3, 8, 4 };
  const size_t  size   = 8;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Read value from message */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 1, &value));
  ck_assert_uint_eq(4, value);

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(8, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the default value from a 32-bit unsigned integer field.
 */
START_TEST(test_get_default_uint32) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 1, &value));
  ck_assert_uint_eq(default_uint32, value);

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the default value from a 64-bit unsigned integer field.
 */
START_TEST(test_get_default_uint64) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  uint64_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 2, &value));
  ck_assert_uint_eq(default_uint64, value);

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the default value from a 32-bit integer field.
 */
START_TEST(test_get_default_int32) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  int32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 3, &value));
  ck_assert_int_eq(default_int32, value);

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the default value from a 64-bit integer field.
 */
START_TEST(test_get_default_int64) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  int64_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 4, &value));
  ck_assert_int_eq(default_int64, value);

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the default value from a boolean field.
 */
START_TEST(test_get_default_bool) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  uint8_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 5, &value));
  ck_assert_uint_eq(default_bool, value);

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the default value from a single-precision float field.
 */
START_TEST(test_get_default_float) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  float value = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 6, &value));
  fail_if(memcmp(&default_float, &value, sizeof(float)));

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the default value from a double-precision float field.
 */
START_TEST(test_get_default_double) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  double value = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 7, &value));
  fail_if(memcmp(&default_double, &value, sizeof(double)));

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the default value from a string field.
 */
START_TEST(test_get_default_string) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  pb_string_t value;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 8, &value));
  fail_unless(pb_string_equals(&value, &default_string));

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the value for a given tag from an absent field.
 */
START_TEST(test_get_absent) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_ABSENT, pb_message_get(&message, 10, &value));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(0, pb_message_version(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the value for a given tag from an unaligned message.
 */
START_TEST(test_get_unaligned) {
  pb_journal_t  journal   = pb_journal_create_empty();
  pb_message_t message1 = pb_message_create(&descriptor, &journal);
  pb_message_t message2 = pb_message_create(&descriptor, &journal);

  /* Write and read from two instances */
  uint32_t value = 100, check = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message1, 1, &value));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message2, 1, &check));
  ck_assert_uint_eq(100, check);

  /* Free all allocated memory */
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the value for a given tag from an invalid message.
 */
START_TEST(test_get_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Read value from message */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_get(&message, 1, &value));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Write a value for a given tag to a message.
 */
START_TEST(test_put) {
  uint8_t data[] = { 8, 127 };
  size_t  size   = 2;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Write value to message */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 1, &value));
  fail_if(memcmp(data, pb_journal_data(&journal), size));

  /* Align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(2, pb_message_size(&message));
  ck_assert_uint_eq(2, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(2, pb_message_end(&message));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a value for a given tag and an existing field to a message.
 */
START_TEST(test_put_existing) {
  uint8_t data[] = { 8, 127 };
  size_t  size   = 2;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Write value to message */
  uint32_t value = 255, check;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 1, &value));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 1, &check));
  ck_assert_uint_eq(value, check);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(3, pb_message_size(&message));
  ck_assert_uint_eq(1, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(3, pb_message_end(&message));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(3, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a message for a given tag to a message.
 */
START_TEST(test_put_message) {
  uint8_t data[] = { 90, 2, 8, 127 };
  size_t  size   = 4;

  /* Create journals and messages */
  pb_journal_t journal1 = pb_journal_create_empty();
  pb_journal_t journal2 = pb_journal_create_empty();
  pb_message_t message1 = pb_message_create(&descriptor, &journal1);
  pb_message_t message2 = pb_message_create(&descriptor, &journal2);

  /* Write value to first message and first message to second */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message1, 1, &value));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message2, 11, &message1));
  fail_if(memcmp(data, pb_journal_data(&journal2), size));

  /* Align message to perform checks */
  ck_assert_uint_eq(0, pb_message_align(&message2));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message2));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message2));
  ck_assert_uint_eq(4, pb_message_size(&message2));
  ck_assert_uint_eq(2, pb_message_version(&message2));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message2));
  ck_assert_uint_eq(4, pb_message_end(&message2));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal2));
  ck_assert_uint_eq(4, pb_journal_size(&journal2));

  /* Free all allocated memory */
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_journal_destroy(&journal2);
  pb_journal_destroy(&journal1);
} END_TEST

/*
 * Write an existing message for a given tag to a message.
 */
START_TEST(test_put_message_existing) {
  uint8_t data[] = { 90, 2, 8, 127 };
  size_t  size   = 4;

  /* Create journals and messages */
  pb_journal_t journal1 = pb_journal_create_empty();
  pb_journal_t journal2 = pb_journal_create(data, size);
  pb_message_t message1 = pb_message_create(&descriptor, &journal1);
  pb_message_t message2 = pb_message_create(&descriptor, &journal2);

  /* Write value to first message and first message to second */
  pb_string_t value = pb_string_init_from_chars("OVERWRITE");
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message1, 8, &value));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message2, 11, &message1));
  fail_unless(memcmp(data, pb_journal_data(&journal2), size));

  /* Align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message2));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message2));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message2));
  ck_assert_uint_eq(13, pb_message_size(&message2));
  ck_assert_uint_eq(1,  pb_message_version(&message2));

  /* Assert message offsets */
  ck_assert_uint_eq(0,  pb_message_start(&message2));
  ck_assert_uint_eq(13, pb_message_end(&message2));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal2));
  ck_assert_uint_eq(13, pb_journal_size(&journal2));

  /* Free all allocated memory */
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_journal_destroy(&journal2);
  pb_journal_destroy(&journal1);
} END_TEST

/*
 * Write repeated messages for a given tag to a message.
 */
START_TEST(test_put_message_repeated) {
  uint8_t data[] = { 98, 2, 8, 127 };
  size_t  size   = 4;

  /* Create journals and message */
  pb_journal_t journal1 = pb_journal_create_empty();
  pb_journal_t journal2 = pb_journal_create(data, size);
  pb_message_t message  = pb_message_create(&descriptor, &journal1);

  /* Create ten submessages and write to message */
  for (size_t m = 0; m < 10; m++) {
    pb_message_t submessage = pb_message_create(&descriptor, &journal2);

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

    /* Write submessage to message */
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&message, 12, &submessage));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Create cursor for submessages within first message */
  pb_cursor_t cursor = pb_cursor_create(&message, 12);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through submessages */
  for (size_t m = 0; m < 10; m++, pb_cursor_next(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);

    /* Assert submessage validity and error */
    fail_unless(pb_message_valid(&submessage));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

    /* Assert cursor validity and error */
    fail_unless(pb_cursor_valid(&cursor));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal2);
  pb_journal_destroy(&journal1);
} END_TEST

/*
 * Write an invalid message for a given tag to a message.
 */
START_TEST(test_put_message_invalid) {
  uint8_t data[] = { 90, 2, 8, 127 };
  size_t  size   = 4;

  /* Create journal and messages */
  pb_journal_t  journal   = pb_journal_create(data, size);
  pb_message_t message1 = pb_message_create_invalid();
  pb_message_t message2 = pb_message_create(&descriptor, &journal);

  /* Write invalid message */
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_message_put(&message2, 11, &message1));

  /* Align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message2));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message2));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message2));
  ck_assert_uint_eq(4, pb_message_size(&message2));
  ck_assert_uint_eq(0, pb_message_version(&message2));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message2));
  ck_assert_uint_eq(4, pb_message_end(&message2));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(4, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a set of values in reverse order to a message.
 */
START_TEST(test_put_reverse) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create iterator and advance to last non-length field */
  pb_descriptor_iter_t it =
    pb_descriptor_iter_create(&descriptor);
  fail_unless(pb_descriptor_iter_begin(&it));
  do {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_iter_current(&it);
    if (!pb_field_descriptor_default(descriptor))
      break;
  } while (pb_descriptor_iter_next(&it));

  /* Write values in reverse order to message */
  while (pb_descriptor_iter_prev(&it)) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_iter_current(&it);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message,
      pb_field_descriptor_tag(descriptor),
      pb_field_descriptor_default(descriptor)));
  }
  pb_descriptor_iter_destroy(&it);

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create_without_tag(&message);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Check ascending order of tags */
  for (size_t f = 1; f <= 8; f++, pb_cursor_next(&cursor))
    fail_unless(pb_cursor_match(&cursor,
      pb_field_descriptor_default(
        pb_descriptor_field_by_tag(&descriptor, f))));

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a set of values in reverse order to a submessage.
 */
START_TEST(test_put_reverse_nested) {
  pb_journal_t  journal     = pb_journal_create_empty();
  pb_message_t message    = pb_message_create(&descriptor, &journal);
  pb_message_t submessage = pb_message_create_within(&message, 12);

  /* Create iterator and advance to last non-length field */
  pb_descriptor_iter_t it =
    pb_descriptor_iter_create(&descriptor);
  if (pb_descriptor_iter_begin(&it)) {
    do {
      const pb_field_descriptor_t *descriptor =
        pb_descriptor_iter_current(&it);
      if (!pb_field_descriptor_default(descriptor))
        break;
    } while (pb_descriptor_iter_next(&it));
  }

  /* Write values in reverse order to message */
  while (pb_descriptor_iter_prev(&it)) {
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_iter_current(&it);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&submessage,
      pb_field_descriptor_tag(descriptor),
      pb_field_descriptor_default(descriptor)));
  }
  pb_descriptor_iter_destroy(&it);

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create_without_tag(&submessage);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Check ascending order of tags */
  for (size_t f = 1; f <= 8; f++, pb_cursor_next(&cursor))
    fail_unless(pb_cursor_match(&cursor,
      pb_field_descriptor_default(
        pb_descriptor_field_by_tag(&descriptor, f))));

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a value for a given tag to an unaligned message.
 */
START_TEST(test_put_unaligned) {
  pb_journal_t  journal   = pb_journal_create_empty();
  pb_message_t message1 = pb_message_create(&descriptor, &journal);
  pb_message_t message2 = pb_message_create(&descriptor, &journal);

  /* Write values to messages */
  uint32_t value1 = 100, value2 = 200;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message1, 1, &value1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message1, 1, &value2));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message1));

  /* Assert messages are unaligned */
  fail_if(pb_message_equals(&message1, &message2));

  /* Assert contents */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message1, 1, &value1));
  ck_assert_uint_eq(200, value1);

  /* Free all allocated memory */
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a value for a given tag to an invalid message.
 */
START_TEST(test_put_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Write value to message */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_put(&message, 1, &value));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Erase a field for a given tag from a message.
 */
START_TEST(test_erase) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Clear field from message */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message, 1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message, 1));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Erase a field for a given tag from an empty message.
 */
START_TEST(test_erase_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Clear field from message */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message, 1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message, 1));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear a submessage for a given tag from a message.
 */
START_TEST(test_erase_message) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_message_t submessage = pb_message_create_within(&message, 11);

  /* Align messages to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(2, pb_message_size(&message));
  ck_assert_uint_eq(1, pb_message_version(&message));

  /* Clear submessage and align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message, 11));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message, 11));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(2, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(0, pb_message_end(&message));

  /* Align submessage to perform checks */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_align(&submessage));

  /* Assert submessage validity and error */
  fail_if(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&submessage));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear an existing submessage for a given tag from a message.
 */
START_TEST(test_erase_message_existing) {
  const uint8_t data[] = { 90, 0 };
  const size_t  size   = 2;

  /* Create journal, message and submessage */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_message_t submessage = pb_message_create_within(&message, 11);

  /* Clear submessage and align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message, 11));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert message size and version */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(1, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(0, pb_message_end(&message));

  /* Align submessage */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_align(&submessage));

  /* Assert submessage validity and error */
  fail_if(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&submessage));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear a repeated submessage for a given tag from a message.
 */
START_TEST(test_erase_message_repeated) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create a hundred submessage siblings and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;

    /* Create submessage and write value */
    pb_message_t submessage = pb_message_create_within(&message, 12);
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&submessage, 2, &value));

    /* Free all allocated memory */
    pb_message_destroy(&submessage);
  }

  /* Align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert message size and version */
  fail_if(pb_message_empty(&message));
  ck_assert_uint_eq(400, pb_message_size(&message));
  ck_assert_uint_eq(300, pb_message_version(&message));

  /* Assert message offsets */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(400, pb_message_end(&message));

  /* Clear submessage and align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message, 12));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert message size and version again */
  fail_unless(pb_message_empty(&message));
  ck_assert_uint_eq(0, pb_message_size(&message));
  ck_assert_uint_eq(400, pb_message_version(&message));

  /* Assert message offsets again */
  ck_assert_uint_eq(0, pb_message_start(&message));
  ck_assert_uint_eq(0, pb_message_end(&message));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Erase a field for a given tag from a submessage.
 */
START_TEST(test_erase_message_nested) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_message_t submessage = pb_message_create_within(&message, 11);

  /* Write value to submessage */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&submessage, 1, &value));

  /* Clear field from submessage */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&submessage, 1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&submessage, 1));

  /* Assert submessage validity and error */
  fail_unless(pb_message_valid(&submessage));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&submessage));

  /* Assert submessage size and version */
  fail_unless(pb_message_empty(&submessage));
  ck_assert_uint_eq(0, pb_message_size(&submessage));
  ck_assert_uint_eq(4, pb_message_version(&submessage));

  /* Assert submessage offsets */
  ck_assert_uint_eq(2, pb_message_start(&submessage));
  ck_assert_uint_eq(2, pb_message_end(&submessage));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Erase a field for a given tag from an unaligned message.
 */
START_TEST(test_erase_unaligned) {
  pb_journal_t  journal   = pb_journal_create_empty();
  pb_message_t message1 = pb_message_create(&descriptor, &journal);
  pb_message_t message2 = pb_message_create(&descriptor, &journal);

  /* Clear field from both messages */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message1, 1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message1, 1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_erase(&message2, 1));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message1));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message1));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Erase a field for a given tag from an invalid message.
 */
START_TEST(test_erase_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Clear field from message */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_erase(&message, 1));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Clear a message entirely.
 */
START_TEST(test_clear) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Clear message entirely */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_clear(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_clear(&message));

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear a message with a submessage entirely.
 */
START_TEST(test_clear_message) {
  const uint8_t data[] = { 82, 0 };
  const size_t  size   = 2;

  /* Create journal, message and submessage */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_message_t submessage = pb_message_create_within(&message, 11);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Clear message entirely */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_clear(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_clear(&message));

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&submessage);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear a message with submessage siblings entirely.
 */
START_TEST(test_clear_message_siblings) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred submessage siblings */
  for (size_t m = 1; m < 101; m++)
    messages[m] = pb_message_create_within(&(messages[m - 1]), 12);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Clear message entirely */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_clear(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_clear(&(messages[0])));

  /* Align and destroy submessage siblings */
  for (size_t m = 1; m < 101; m++) {
    ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_align(&(messages[m])));

    /* Assert submessage validity and error */
    fail_if(pb_message_valid(&(messages[m])));
    ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&(messages[m])));

    /* Free all allocated memory */
    pb_message_destroy(&(messages[m]));
  }

  /* Assert message validity and error */
  fail_if(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&(messages[0])));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&(messages[0]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear a message with nested submessages entirely.
 */
START_TEST(test_clear_message_nested) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++)
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Clear message entirely */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_clear(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_clear(&(messages[0])));

  /* Align and destroy nested submessages */
  for (size_t m = 1; m < 101; m++) {
    ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_align(&(messages[m])));

    /* Assert submessage validity and error */
    fail_if(pb_message_valid(&(messages[m])));
    ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&(messages[m])));

    /* Free all allocated memory */
    pb_message_destroy(&(messages[m]));
  }

  /* Assert message validity and error */
  fail_if(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&(messages[0])));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&(messages[0]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear a message with nested submessages inside-out entirely.
 */
START_TEST(test_clear_message_nested_inside_out) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++)
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Align and clear nested submessages */
  for (size_t m = 100; m > 0; m--) {
    ck_assert_uint_eq(PB_ERROR_NONE, pb_message_clear(&(messages[m])));
    ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_clear(&(messages[m])));

    /* Assert submessage validity and error */
    fail_if(pb_message_valid(&(messages[m])));
    ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&(messages[m])));

    /* Free all allocated memory */
    pb_message_destroy(&(messages[m]));
  }

  /* Clear message entirely */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_clear(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_clear(&(messages[0])));

  /* Assert message validity and error */
  fail_if(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&(messages[0])));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&(messages[0]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear an unaligned message entirely.
 */
START_TEST(test_clear_unaligned) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal and message */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Write value to message */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 1, &value));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Clear message entirely */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_clear(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_clear(&message));

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear an invalid message entirely.
 */
START_TEST(test_clear_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Clear message */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_clear(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Retrieve a pointer to the raw data for a given tag from a message.
 */
START_TEST(test_raw) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Write value to message */
  double value = 1000000.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 7, &value));

  /* Align message to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&message));

  /* Obtain and change raw data */
  double *raw = pb_message_raw(&message, 7);
  ck_assert_ptr_ne(NULL, raw);
  fail_if(memcmp(raw, &value, sizeof(double)));
  *raw = 0.123456789;

  /* Read new value from message */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 7, &value));
  fail_if(memcmp(raw, &value, sizeof(double)));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Retrieve a pointer to the raw data for a given tag from a message.
 */
START_TEST(test_raw_default) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Read default value from message */
  double value = 0.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 7, &value));
  fail_if(memcmp(&default_double, &value, sizeof(double)));

  /* Obtain raw data */
  double *raw = pb_message_raw(&message, 7);
  ck_assert_ptr_eq(NULL, raw);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Retrieve a pointer to the raw data from an unaligned message.
 */
START_TEST(test_raw_unaligned) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Write value to message */
  double value = 1000000.0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message, 7, &value));

  /* Obtain and change raw data */
  double *raw = pb_message_raw(&message, 7);
  ck_assert_ptr_ne(NULL, raw);
  fail_if(memcmp(raw, &value, sizeof(double)));
  *raw = 0.123456789;

  /* Read new value from message */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_get(&message, 7, &value));
  fail_if(memcmp(raw, &value, sizeof(double)));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Retrieve a pointer to the raw data for a given tag from an invalid message.
 */
START_TEST(test_raw_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Obtain raw data */
  double *raw = pb_message_raw(&message, 7);
  ck_assert_ptr_eq(NULL, raw);

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Ensure that a message is properly aligned.
 */
START_TEST(test_align) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create submessages */
  pb_message_t submessage1 = pb_message_create_within(&message, 11);
  pb_message_t submessage2 = pb_message_create_within(&message, 11);

  /* Write value to submessage */
  pb_string_t value = pb_string_init_from_chars(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam euismod "
    "vehicula nibh, et egestas erat eleifend quis. Nam hendrerit egestas "
    "quam nec egestas. Donec lacinia vestibulum erat, ac suscipit nisi "
    "vehicula nec. Praesent ullamcorper vitae lorem vel euismod. Quisque "
    "fringilla lobortis convallis. Aliquam accumsan lacus eu viverra dapibus. "
    "Phasellus in adipiscing sem, in congue massa. Vestibulum ullamcorper "
    "orci nec semper pretium.");
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&submessage1, 8, &value));

  /* Align submessage to perform checks */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&submessage1));

  /* Assert submessage size and version */
  fail_if(pb_message_empty(&submessage1));
  ck_assert_uint_eq(440, pb_message_size(&submessage1));
  ck_assert_uint_eq(5, pb_message_version(&submessage1));

  /* Assert alignment */
  ck_assert_uint_eq(1, pb_message_version(&submessage2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_align(&submessage2));
  fail_unless(pb_message_equals(&submessage1, &submessage2));

  /* Free all allocated memory */
  pb_message_destroy(&submessage2);
  pb_message_destroy(&submessage1);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
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
  void *suite = suite_create("protobluff/message/message"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_empty);
  tcase_add_test(tcase, test_create_within);
  tcase_add_test(tcase, test_create_within_existing);
  tcase_add_test(tcase, test_create_within_siblings);
  tcase_add_test(tcase, test_create_within_siblings_empty);
  tcase_add_test(tcase, test_create_within_siblings_strings);
  tcase_add_test(tcase, test_create_within_nested);
  tcase_add_test(tcase, test_create_within_nested_empty);
  tcase_add_test(tcase, test_create_within_nested_strings);
  tcase_add_test(tcase, test_create_within_invalid);
  tcase_add_test(tcase, test_create_nested);
  tcase_add_test(tcase, test_create_nested_within);
  tcase_add_test(tcase, test_create_nested_repeated);
  tcase_add_test(tcase, test_create_nested_invalid);
  tcase_add_test(tcase, test_create_from_cursor);
  tcase_add_test(tcase, test_create_from_cursor_invalid);
  tcase_add_test(tcase, test_create_from_cursor_invalid_tag);
  tcase_add_test(tcase, test_create_from_cursor_invalid_wiretype);
  tcase_add_test(tcase, test_create_from_field);
  tcase_add_test(tcase, test_create_from_field_empty);
  tcase_add_test(tcase, test_create_from_field_invalid);
  tcase_add_test(tcase, test_create_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "copy" */
  tcase = tcase_create("copy");
  tcase_add_test(tcase, test_copy);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "has" */
  tcase = tcase_create("has");
  tcase_add_test(tcase, test_has);
  tcase_add_test(tcase, test_has_empty);
  tcase_add_test(tcase, test_has_repeated);
  tcase_add_test(tcase, test_has_unaligned);
  tcase_add_test(tcase, test_has_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "match" */
  tcase = tcase_create("match");
  tcase_add_test(tcase, test_match);
  tcase_add_test(tcase, test_match_empty);
  tcase_add_test(tcase, test_match_merged);
  tcase_add_test(tcase, test_match_unaligned);
  tcase_add_test(tcase, test_match_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "get" */
  tcase = tcase_create("get");
  tcase_add_test(tcase, test_get);
  tcase_add_test(tcase, test_get_merged);
  tcase_add_test(tcase, test_get_default_uint32);
  tcase_add_test(tcase, test_get_default_uint64);
  tcase_add_test(tcase, test_get_default_int32);
  tcase_add_test(tcase, test_get_default_int64);
  tcase_add_test(tcase, test_get_default_bool);
  tcase_add_test(tcase, test_get_default_float);
  tcase_add_test(tcase, test_get_default_double);
  tcase_add_test(tcase, test_get_default_string);
  tcase_add_test(tcase, test_get_absent);
  tcase_add_test(tcase, test_get_unaligned);
  tcase_add_test(tcase, test_get_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "put" */
  tcase = tcase_create("put");
  tcase_add_test(tcase, test_put);
  tcase_add_test(tcase, test_put_existing);
  tcase_add_test(tcase, test_put_message);
  tcase_add_test(tcase, test_put_message_existing);
  tcase_add_test(tcase, test_put_message_repeated);
  tcase_add_test(tcase, test_put_message_invalid);
  tcase_add_test(tcase, test_put_reverse);
  tcase_add_test(tcase, test_put_reverse_nested);
  tcase_add_test(tcase, test_put_unaligned);
  tcase_add_test(tcase, test_put_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "erase" */
  tcase = tcase_create("erase");
  tcase_add_test(tcase, test_erase);
  tcase_add_test(tcase, test_erase_empty);
  tcase_add_test(tcase, test_erase_message);
  tcase_add_test(tcase, test_erase_message_existing);
  tcase_add_test(tcase, test_erase_message_repeated);
  tcase_add_test(tcase, test_erase_message_nested);
  tcase_add_test(tcase, test_erase_unaligned);
  tcase_add_test(tcase, test_erase_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "clear" */
  tcase = tcase_create("clear");
  tcase_add_test(tcase, test_clear);
  tcase_add_test(tcase, test_clear_message);
  tcase_add_test(tcase, test_clear_message_siblings);
  tcase_add_test(tcase, test_clear_message_nested);
  tcase_add_test(tcase, test_clear_message_nested_inside_out);
  tcase_add_test(tcase, test_clear_unaligned);
  tcase_add_test(tcase, test_clear_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "raw" */
  tcase = tcase_create("raw");
  tcase_add_test(tcase, test_raw);
  tcase_add_test(tcase, test_raw_default);
  tcase_add_test(tcase, test_raw_unaligned);
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
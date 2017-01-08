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
#include <stdint.h>
#include <stdlib.h>

#include <protobluff/descriptor.h>

#include "message/common.h"
#include "message/cursor.h"
#include "message/journal.h"
#include "message/message.h"
#include "message/nested.h"

/* ----------------------------------------------------------------------------
 * Defaults
 * ------------------------------------------------------------------------- */

/* Unsigned 32-bit integer default */
static const uint32_t
default_uint32 = 1000000000U;

/* Unsigned 64-bit integer default */
static const uint64_t
default_uint64 = 1000000000000000000ULL;

/* ----------------------------------------------------------------------------
 * Descriptors
 * ------------------------------------------------------------------------- */

/* Descriptor */
static pb_descriptor_t
descriptor = { {
  (const pb_field_descriptor_t []){
    {  1, "F01", UINT32,  OPTIONAL, NULL, &default_uint32 },
    {  2, "F02", UINT64,  OPTIONAL, NULL, &default_uint64 },
    {  3, "F03", SINT32,  OPTIONAL },
    {  4, "F04", SINT64,  REPEATED },
    {  5, "F05", BOOL,    OPTIONAL },
    {  6, "F06", FLOAT,   OPTIONAL },
    {  7, "F07", DOUBLE,  OPTIONAL },
    {  8, "F08", STRING,  OPTIONAL },
    {  9, "F09", BYTES,   OPTIONAL },
    { 10, "F10", UINT32,  REPEATED, NULL, NULL, PACKED },
    { 11, "F11", MESSAGE, OPTIONAL, &descriptor },
    { 12, "F12", MESSAGE, REPEATED, &descriptor }
  }, 12 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Test whether a nested message contains at least one field.
 */
START_TEST(test_has) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Iterate over submessages */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 100; t++) {
    tags[t] = 11; tags[t + 1] = 2;

    /* Assert field existence */
    fail_unless(pb_message_nested_has(&(messages[0]), tags, t + 2));
  }

  /* Assert field non-existence */
  tags[100] = 11; tags[101] = 2;
  fail_if(pb_message_nested_has(&(messages[0]), tags, 101));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Test whether an empty nested message contains at least one field.
 */
START_TEST(test_has_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Assert field non-existence */
  fail_if(pb_message_nested_has(&message, tags, 101));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Test whether a nested message contains at least one repeated field.
 */
START_TEST(test_has_repeated) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 4, &value));
  }

  /* Iterate over submessages */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 100; t++) {
    tags[t] = 11; tags[t + 1] = 4;

    /* Assert field existence */
    fail_unless(pb_message_nested_has(&(messages[0]), tags, t + 2));
  }

  /* Assert field non-existence */
  tags[100] = 11; tags[101] = 4;
  fail_if(pb_message_nested_has(&(messages[0]), tags, 101));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Test whether an unaligned nested message contains at least one field.
 */
START_TEST(test_has_unaligned) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Write value to message */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_put(&(messages[0]), 1, &value));

  /* Iterate over submessages */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 100; t++) {
    tags[t] = 11; tags[t + 1] = 2;

    /* Assert field existence */
    fail_unless(pb_message_nested_has(&(messages[0]), tags, t + 2));
  }

  /* Assert field non-existence */
  tags[100] = 11; tags[101] = 2;
  fail_if(pb_message_nested_has(&(messages[0]), tags, 101));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Test whether an invalid nested message contains at least one field.
 */
START_TEST(test_has_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Assert field non-existence */
  fail_if(pb_message_nested_has(&message, tags, 102));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Compare the value from a nested message with the given value.
 */
START_TEST(test_match) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Iterate over submessages */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 100; t++) {
    tags[t] = 11; tags[t + 1] = 2;

    /* Compare value with value from message */
    uint64_t value = t + 1;
    fail_unless(pb_message_nested_match(&(messages[0]), tags, t + 2, &value));
  }

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Compare the value from an empty nested message with the given value.
 */
START_TEST(test_match_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Compare value with value from message */
  uint64_t value = 127;
  fail_if(pb_message_nested_match(&message, tags, 101, &value));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Compare the value from an nested message with the given value.
 */
START_TEST(test_match_repeated) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 4, &value));
  }

  /* Iterate over submessages */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 100; t++) {
    tags[t] = 11; tags[t + 1] = 4;

    /* Compare value with value from message */
    uint64_t value = t + 1;
    fail_unless(pb_message_nested_match(&(messages[0]), tags, t + 2, &value));
  }

  /* Assert field non-existence */
  tags[100] = 11; tags[101] = 4;
  fail_if(pb_message_nested_has(&(messages[0]), tags, 101));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Compare the value from an unaligned nested message with the given value.
 */
START_TEST(test_match_unaligned) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Write value to message */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_put(&(messages[0]), 1, &value));

  /* Iterate over submessages */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 100; t++) {
    tags[t] = 11; tags[t + 1] = 2;

    /* Compare value with value from message */
    uint64_t value = t + 1;
    fail_unless(pb_message_nested_match(&(messages[0]), tags, t + 2, &value));
  }

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Compare the value from an invalid nested message with the given value.
 */
START_TEST(test_match_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Compare value with value from message */
  uint64_t value = 127;
  fail_if(pb_message_nested_match(&message, tags, 102, &value));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Read the value from a nested message for a branch of tags.
 */
START_TEST(test_get) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Iterate over submessages */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 100; t++) {
    tags[t] = 11; tags[t + 1] = 2;

    /* Read values from nested submessages */
    uint64_t value = 0;
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_nested_get(&(messages[0]), tags, t + 2, &value));
    ck_assert_uint_eq(t, value - 1);
  }

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the default value from a nested message for a branch of tags.
 */
START_TEST(test_get_default) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++)
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);

  /* Create tags */
  pb_tag_t tags[101] = {};
  for (size_t t = 0; t < 100; t++)
    tags[t] = 11;
  tags[100] = 2;

  /* Read value from nested submessage */
  uint64_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_get(&(messages[0]), tags, 101, &value));
  ck_assert_uint_eq(default_uint64, value);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read an absent value from a nested message for a branch of tags.
 */
START_TEST(test_get_absent) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 10;

  /* Read value from nested submessage */
  uint32_t value = 0;
  ck_assert_uint_eq(PB_ERROR_ABSENT,
    pb_message_nested_get(&(messages[0]), tags, 102, &value));
  ck_assert_uint_eq(0, value);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the value from an unaligned nested message for a branch of tags.
 */
START_TEST(test_get_unaligned) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t messages[101] = { pb_message_create(&descriptor, &journal) };

  /* Create a hundred nested submessages and write values to them */
  for (size_t m = 1; m < 101; m++) {
    uint64_t value = m;
    messages[m] = pb_message_create_within(&(messages[m - 1]), 11);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_put(&(messages[m]), 2, &value));
  }

  /* Write value to message */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_put(&(messages[0]), 1, &value));

  /* Iterate over submessages */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 100; t++) {
    tags[t] = 11; tags[t + 1] = 2;

    /* Read values from nested submessages */
    uint64_t value = 0;
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_message_nested_get(&(messages[0]), tags, t + 2, &value));
    ck_assert_uint_eq(t, value - 1);
  }

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&(messages[0])));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&(messages[0])));

  /* Free all allocated memory */
  for (size_t m = 0; m < 101; m++)
    pb_message_destroy(&(messages[m]));
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Read the value from an invalid nested message for a branch of tags.
 */
START_TEST(test_get_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Read value from nested submessage */
  uint64_t value = 0;
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_message_nested_get(&message, tags, 102, &value));
  ck_assert_uint_eq(0, value);

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Write a value for a branch of tags to a nested message.
 */
START_TEST(test_put) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Write value to nested submessage */
  uint64_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Read value from nested submessage */
  value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_get(&message, tags, 102, &value));
  ck_assert_uint_eq(127, value);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a value for a branch of tags to an existing field in a nested message.
 */
START_TEST(test_put_existing) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Write value to nested submessage */
  uint64_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Write value to nested submessage again */
  value = 255;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Read value from nested submessage */
  value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_get(&message, tags, 102, &value));
  ck_assert_uint_eq(255, value);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a value for a branch of tags to a packed field in a nested message.
 */
START_TEST(test_put_packed) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 10;

  /* Write value to nested submessage */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Create nested cursor */
  pb_cursor_t cursor = pb_cursor_create_nested(&message, tags, 102);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Read value from nested submessage */
  value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_get(&cursor, &value));
  ck_assert_uint_eq(127, value);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a message for a branch of tags to a nested message.
 */
START_TEST(test_put_message) {
  pb_journal_t  journal1  = pb_journal_create_empty();
  pb_journal_t  journal2  = pb_journal_create_empty();
  pb_message_t message1 = pb_message_create(&descriptor, &journal1);
  pb_message_t message2 = pb_message_create(&descriptor, &journal2);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 1;

  /* Write value to first message and first message to second */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message1, 1, &value));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message2, tags, 101, &message1));

  /* Read value from nested submessage */
  value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_get(&message2, tags, 102, &value));
  ck_assert_uint_eq(127, value);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message2));

  /* Free all allocated memory */
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_journal_destroy(&journal2);
  pb_journal_destroy(&journal1);
} END_TEST

/*
 * Write a value for a branch of tags to an unaligned nested message.
 */
START_TEST(test_put_unaligned) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Write value to nested submessage */
  uint64_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Write value to message */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_put(&message, 2, &value));

  /* Read value from nested submessage */
  value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_get(&message, tags, 102, &value));
  ck_assert_uint_eq(127, value);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a value for a branch of tags to an invalid nested message.
 */
START_TEST(test_put_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Write value to nested submessage */
  uint64_t value = 127;
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Free all allocated memory */
  pb_message_destroy(&message);
} END_TEST

/*
 * Erase a field for a branch of tags from a nested message.
 */
START_TEST(test_erase) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 3;

  /* Write value to nested submessage */
  int32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Clear field from nested submessage */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message, tags, 102));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message, tags, 102));

  /* Read value from nested submessage */
  ck_assert_uint_eq(PB_ERROR_ABSENT,
    pb_message_nested_get(&message, tags, 102, &value));

  /* Read default value from nested submessage */
  tags[101] = 1;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_get(&message, tags, 102, &value));
  ck_assert_int_eq(default_uint32, value);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Erase a field for a branch of tags from an empty message.
 */
START_TEST(test_erase_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 3;

  /* Write value to nested submessage */
  int32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Clear field from nested submessage */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message, tags, 102));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message, tags, 102));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Erase a field for a branch of tags from a nested message.
 */
START_TEST(test_erase_packed) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 10;

  /* Write value to nested submessage */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Clear field from nested submessage */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message, tags, 102));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message, tags, 102));

  /* Create nested cursor */
  pb_cursor_t cursor = pb_cursor_create_nested(&message, tags, 102);

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_EOM, pb_cursor_error(&cursor));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Erase a submessage for a branch of tags from a nested message.
 */
START_TEST(test_erase_message) {
  pb_journal_t  journal1  = pb_journal_create_empty();
  pb_journal_t  journal2  = pb_journal_create_empty();
  pb_message_t message1 = pb_message_create(&descriptor, &journal1);
  pb_message_t message2 = pb_message_create(&descriptor, &journal2);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 1;

  /* Write value to first message and first message to second */
  uint32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_put(&message1, 1, &value));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message2, tags, 101, &message1));

  /* Clear field from nested submessage */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message2, tags, 102));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message2, tags, 102));

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message2));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message2));

  /* Free all allocated memory */
  pb_message_destroy(&message2);
  pb_message_destroy(&message1);
  pb_journal_destroy(&journal2);
  pb_journal_destroy(&journal1);
} END_TEST

/*
 * Erase a field for a branch of tags from an unaligned nested message.
 */
START_TEST(test_erase_unaligned) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 3;

  /* Write value to nested submessage */
  int32_t value = 127;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_put(&message, tags, 102, &value));

  /* Write value to message */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_put(&message, 1, &value));

  /* Clear field from nested submessage */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message, tags, 102));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_erase(&message, tags, 102));

  /* Read value from nested submessage */
  ck_assert_uint_eq(PB_ERROR_ABSENT,
    pb_message_nested_get(&message, tags, 102, &value));

  /* Read default value from nested submessage */
  tags[101] = 1;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_message_nested_get(&message, tags, 102, &value));
  ck_assert_int_eq(default_uint32, value);

  /* Assert message validity and error */
  fail_unless(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_message_error(&message));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Erase a field for a branch of tags from an invalid nested message.
 */
START_TEST(test_erase_invalid) {
  pb_message_t message = pb_message_create_invalid();

  /* Assert message validity and error */
  fail_if(pb_message_valid(&message));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_message_error(&message));

  /* Create tags */
  pb_tag_t tags[102] = {};
  for (size_t t = 0; t < 101; t++)
    tags[t] = 11;
  tags[101] = 2;

  /* Clear field from nested submessage */
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_message_nested_erase(&message, tags, 102));

  /* Free all allocated memory */
  pb_message_destroy(&message);
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
  void *suite = suite_create("protobluff/message/nested"),
       *tcase = NULL;

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
  tcase_add_test(tcase, test_match_repeated);
  tcase_add_test(tcase, test_match_unaligned);
  tcase_add_test(tcase, test_match_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "get" */
  tcase = tcase_create("get");
  tcase_add_test(tcase, test_get);
  tcase_add_test(tcase, test_get_default);
  tcase_add_test(tcase, test_get_absent);
  tcase_add_test(tcase, test_get_unaligned);
  tcase_add_test(tcase, test_get_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "put" */
  tcase = tcase_create("put");
  tcase_add_test(tcase, test_put);
  tcase_add_test(tcase, test_put_existing);
  tcase_add_test(tcase, test_put_packed);
  tcase_add_test(tcase, test_put_message);
  tcase_add_test(tcase, test_put_unaligned);
  tcase_add_test(tcase, test_put_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "erase" */
  tcase = tcase_create("erase");
  tcase_add_test(tcase, test_erase);
  tcase_add_test(tcase, test_erase_empty);
  tcase_add_test(tcase, test_erase_packed);
  tcase_add_test(tcase, test_erase_message);
  tcase_add_test(tcase, test_erase_unaligned);
  tcase_add_test(tcase, test_erase_invalid);
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

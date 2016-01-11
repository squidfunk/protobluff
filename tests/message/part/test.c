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

#include <protobluff/descriptor.h>

#include "core/allocator.h"
#include "message/common.h"
#include "message/cursor.h"
#include "message/journal.h"
#include "message/message.h"
#include "message/part.h"

/* ----------------------------------------------------------------------------
 * System-default allocator callback overrides
 * ------------------------------------------------------------------------- */

/*!
 * Allocator with failing reallocation.
 *
 * \param[in,out] data  Internal allocator data
 * \param[in,out] block Memory block to be resized
 * \param[in]     size  Bytes to be allocated
 * \return              Memory block
 */
static void *
allocator_resize_fail(void *data, void *block, size_t size) {
  assert(!data && size);
  return NULL;
}

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
      2, 3, 5, 11
    }, 4 } };

/* Descriptor */
static pb_descriptor_t
descriptor = { {
  (const pb_field_descriptor_t []){
    {  1, "F01", UINT32,  OPTIONAL },
    {  2, "F02", UINT64,  OPTIONAL },
    {  3, "F03", SINT32,  ONEOF, NULL, &oneof_descriptor },
    {  4, "F04", SINT64,  ONEOF, NULL, &oneof_descriptor },
    {  5, "F05", BOOL,    OPTIONAL },
    {  6, "F06", FLOAT,   ONEOF, NULL, &oneof_descriptor },
    {  7, "F07", DOUBLE,  OPTIONAL },
    {  8, "F08", STRING,  OPTIONAL },
    {  9, "F09", UINT32,  REPEATED },
    { 10, "F10", UINT64,  REPEATED },
    { 11, "F11", STRING,  REPEATED },
    { 12, "F12", MESSAGE, ONEOF, &descriptor, &oneof_descriptor }
  }, 12 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a part from a message for a specific tag.
 */
START_TEST(test_create) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 1);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(1, pb_part_size(&part));
  ck_assert_uint_eq(0, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(1, pb_part_start(&part));
  ck_assert_uint_eq(2, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a set of repeated parts within a message for a specific tag.
 */
START_TEST(test_create_repeated) {
  pb_journal_t journal  = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create a hundred parts and write values to them */
  for (size_t p = 1; p < 101; p++) {
    pb_part_t part = pb_part_create(&message, 10);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_part_write(&part, (uint8_t *)&p, 1));

    /* Assert part validity and error */
    fail_unless(pb_part_valid(&part));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

    /* Assert part size and version */
    fail_if(pb_part_empty(&part));
    ck_assert_uint_eq(1, pb_part_size(&part));
    ck_assert_uint_eq(p * 2, pb_part_version(&part));

    /* Assert part offsets */
    ck_assert_uint_eq(p * 2 - 1, pb_part_start(&part));
    ck_assert_uint_eq(p * 2, pb_part_end(&part));

    /* Assert journal size */
    fail_if(pb_journal_empty(&journal));
    ck_assert_uint_eq(p * 2, pb_journal_size(&journal));

    /* Free all allocated memory */
    pb_part_destroy(&part);
  }

  /* Assert journal contents */
  for (size_t p = 1; p < 101; p++)
    ck_assert_uint_eq(p, pb_journal_data_at(&journal, p * 2 - 1));

  /* Free all allocated memory */
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a part from an empty message for a specific tag.
 */
START_TEST(test_create_message_empty) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 1);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_unless(pb_part_empty(&part));
  ck_assert_uint_eq(0, pb_part_size(&part));
  ck_assert_uint_eq(1, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(1, pb_part_start(&part));
  ck_assert_uint_eq(1, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a part from a merged message for a specific tag.
 */
START_TEST(test_create_message_merged) {
  const uint8_t data[] = { 8, 127, 8, 127 };
  const size_t  size   = 4;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 1);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(1, pb_part_size(&part));
  ck_assert_uint_eq(0, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(3, pb_part_start(&part));
  ck_assert_uint_eq(4, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(4, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a part from an invalid message for a specific tag.
 */
START_TEST(test_create_message_invalid) {
  pb_message_t message = pb_message_create_invalid();
  pb_part_t    part    = pb_part_create(&message, 1);

  /* Assert part validity and error */
  fail_if(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_error(&part));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
} END_TEST

/*
 * Create a part from a message for a specific tag that is part of a oneof.
 */
START_TEST(test_create_oneof) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 3);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_unless(pb_part_empty(&part));
  ck_assert_uint_eq(0, pb_part_size(&part));
  ck_assert_uint_eq(1, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(1, pb_part_start(&part));
  ck_assert_uint_eq(1, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a part from a message for a specific tag that is part of a oneof.
 */
START_TEST(test_create_oneof_existing) {
  const uint8_t data[] = { 24, 127 };
  const size_t  size   = 2;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 3);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(1, pb_part_size(&part));
  ck_assert_uint_eq(0, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(1, pb_part_start(&part));
  ck_assert_uint_eq(2, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a part from a message for a specific tag that is part of a oneof.
 */
START_TEST(test_create_oneof_existing_before) {
  const uint8_t data[] = { 32, 127, 53, 0, 0, 0, 0, 98, 2, 8, 1 };
  const size_t  size   = 11;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 12);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(2, pb_part_size(&part));
  ck_assert_uint_eq(0, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(9, pb_part_start(&part));
  ck_assert_uint_eq(11, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(11, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a part from a message for a specific tag that is part of a oneof.
 */
START_TEST(test_create_oneof_existing_after) {
  const uint8_t data[] = { 32, 127, 53, 0, 0, 0, 0, 98, 0 };
  const size_t  size   = 9;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 3);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_unless(pb_part_empty(&part));
  ck_assert_uint_eq(0, pb_part_size(&part));
  ck_assert_uint_eq(4, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(1, pb_part_start(&part));
  ck_assert_uint_eq(1, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(1, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a part from a journal.
 */
START_TEST(test_create_from_journal) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal and part */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_part_t    part    = pb_part_create_from_journal(&journal);

  /* Assert same locations */
  ck_assert_ptr_eq(&journal, pb_part_journal(&part));

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(2, pb_part_size(&part));
  ck_assert_uint_eq(0, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(0, pb_part_start(&part));
  ck_assert_uint_eq(2, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a part from an invalid journal.
 */
START_TEST(test_create_from_journal_invalid) {
  pb_journal_t journal = pb_journal_create_invalid();
  pb_part_t    part    = pb_part_create_from_journal(&journal);

  /* Assert part validity and error */
  fail_if(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_error(&part));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create a part at the current position of a cursor.
 */
START_TEST(test_create_from_cursor) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);

  /* Create a hundred part siblings and write values to them */
  for (size_t p = 1; p < 101; p++) {
    pb_part_t part = pb_part_create(&message, 10);
    ck_assert_uint_eq(PB_ERROR_NONE,
      pb_part_write(&part, (uint8_t *)&p, 1));

    /* Free all allocated memory */
    pb_part_destroy(&part);
  }

  /* Create cursor */
  pb_cursor_t cursor = pb_cursor_create(&message, 10);

  /* Assert cursor validity and error */
  fail_unless(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_cursor_error(&cursor));

  /* Walk through parts */
  for (size_t p = 1; p < 101; p++, pb_cursor_next(&cursor)) {
    pb_part_t part = pb_part_create_from_cursor(&cursor);

    /* Assert part validity and error */
    fail_unless(pb_part_valid(&part));
    ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

    /* Assert part size and version */
    fail_if(pb_part_empty(&part));
    ck_assert_uint_eq(1, pb_part_size(&part));
    ck_assert_uint_eq(200, pb_part_version(&part));

    /* Assert part offsets */
    ck_assert_uint_eq(p * 2 - 1, pb_part_start(&part));
    ck_assert_uint_eq(p * 2, pb_part_end(&part));

    /* Free all allocated memory */
    pb_part_destroy(&part);
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
 * Create a part from an invalid cursor.
 */
START_TEST(test_create_from_cursor_invalid) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_cursor_t  cursor  = pb_cursor_create_invalid();

  /* Assert cursor validity and error */
  fail_if(pb_cursor_valid(&cursor));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_cursor_error(&cursor));

  /* Create part from cursor */
  pb_part_t part = pb_part_create_from_cursor(&cursor);

  /* Assert part validity and error */
  fail_if(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_error(&part));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_cursor_destroy(&cursor);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Create an invalid part.
 */
START_TEST(test_create_invalid) {
  pb_part_t part = pb_part_create_invalid();

  /* Assert part validity and error */
  fail_if(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_error(&part));

  /* Free all allocated memory */
  pb_part_destroy(&part);
} END_TEST

/*
 * Write a value to a part.
 */
START_TEST(test_write) {
  uint8_t data[] = { 8, 127 };
  size_t  size   = 2;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 1);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_unless(pb_part_empty(&part));
  ck_assert_uint_eq(0, pb_part_size(&part));
  ck_assert_uint_eq(1, pb_part_version(&part));

  /* Write value to part */
  uint8_t value[] = { 127 };
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_write(&part, value, 1));
  fail_if(memcmp(data, pb_journal_data(&journal), size));

  /* Assert part validity and error again */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(1, pb_part_size(&part));
  ck_assert_uint_eq(2, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(1, pb_part_start(&part));
  ck_assert_uint_eq(2, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a string value to a part.
 */
START_TEST(test_write_string) {
  const uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  const size_t  size   = 11;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 8);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_unless(pb_part_empty(&part));
  ck_assert_uint_eq(0, pb_part_size(&part));
  ck_assert_uint_eq(1, pb_part_version(&part));

  /* Write value to part */
  uint8_t value[] = "SOME DATA";
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_write(&part, value, 9));
  fail_if(memcmp(data, pb_journal_data(&journal), size));

  /* Assert part validity and error again */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(9, pb_part_size(&part));
  ck_assert_uint_eq(2, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(2,  pb_part_start(&part));
  ck_assert_uint_eq(11, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(11, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a long string value to a part.
 */
START_TEST(test_write_string_long) {
  pb_journal_t journal = pb_journal_create_empty();
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 8);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_unless(pb_part_empty(&part));
  ck_assert_uint_eq(0, pb_part_size(&part));
  ck_assert_uint_eq(1, pb_part_version(&part));

  /* Write value to part */
  pb_string_t value = pb_string_init_from_chars(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam euismod "
    "vehicula nibh, et egestas erat eleifend quis. Nam hendrerit egestas "
    "quam nec egestas. Donec lacinia vestibulum erat, ac suscipit nisi "
    "vehicula nec. Praesent ullamcorper vitae lorem vel euismod. Quisque "
    "fringilla lobortis convallis. Aliquam accumsan lacus eu viverra dapibus. "
    "Phasellus in adipiscing sem, in congue massa. Vestibulum ullamcorper "
    "orci nec semper pretium.");
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_write(&part,
    pb_string_data(&value), pb_string_size(&value)));

  /* Assert part validity and error again */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(437, pb_part_size(&part));
  ck_assert_uint_eq(3, pb_part_version(&part));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a value to an invalid part.
 */
START_TEST(test_write_invalid) {
  pb_part_t part = pb_part_create_invalid();

  /* Assert part validity and error */
  fail_if(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_error(&part));

  /* Write value to part */
  uint8_t value[] = { 0 };
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_write(&part, value, 1));

  /* Free all allocated memory */
  pb_part_destroy(&part);
} END_TEST

/*
 * Write a value to a part for which reallocation fails.
 */
START_TEST(test_write_invalid_resize) {
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_default.proc.resize,
      .free     = allocator_default.proc.free
    }
  };

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create_empty_with_allocator(&allocator);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 8);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_unless(pb_part_empty(&part));
  ck_assert_uint_eq(0, pb_part_size(&part));
  ck_assert_uint_eq(1, pb_part_version(&part));

  /* Patch allocator with faulty resize function */
  allocator.proc.resize = allocator_resize_fail;

  /* Write value to part */
  pb_string_t value = pb_string_init_from_chars(
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Nam euismod "
    "vehicula nibh, et egestas erat eleifend quis. Nam hendrerit egestas "
    "quam nec egestas. Donec lacinia vestibulum erat, ac suscipit nisi "
    "vehicula nec. Praesent ullamcorper vitae lorem vel euismod. Quisque "
    "fringilla lobortis convallis. Aliquam accumsan lacus eu viverra dapibus. "
    "Phasellus in adipiscing sem, in congue massa. Vestibulum ullamcorper "
    "orci nec semper pretium.");
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_part_write(&part,
    pb_string_data(&value), pb_string_size(&value)));

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_unless(pb_part_empty(&part));
  ck_assert_uint_eq(0, pb_part_size(&part));
  ck_assert_uint_eq(1, pb_part_version(&part));

  /* Patch allocator with working reallocation function and try again */
  allocator.proc.resize = allocator_default.proc.resize;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_part_write(&part, value.data, value.size));

  /* Assert part validity and error again */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(437, pb_part_size(&part));
  ck_assert_uint_eq(3, pb_part_version(&part));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Write a value to a part with an underlying zero-copy journal.
 */
START_TEST(test_write_invalid_zero_copy) {
  uint8_t data[] = { 8, 127 };
  size_t  size   = 2;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create_zero_copy(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 1);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(1, pb_part_size(&part));
  ck_assert_uint_eq(0, pb_part_version(&part));

  /* Write value to part */
  uint8_t value[] = { 127, 127 };
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_part_write(&part, value, 2));
  fail_if(memcmp(data, pb_journal_data(&journal), size));

  /* Assert part validity and error again */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(1, pb_part_size(&part));
  ck_assert_uint_eq(0, pb_part_version(&part));

  /* Assert part offsets */
  ck_assert_uint_eq(1, pb_part_start(&part));
  ck_assert_uint_eq(2, pb_part_end(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(2, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear data from a part and invalidate it.
 */
START_TEST(test_clear) {
  const uint8_t data[] = { 8, 127 };
  const size_t  size   = 2;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 1);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Clear part */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_clear(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_clear(&part));

  /* Assert part validity and error again */
  fail_if(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_error(&part));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear data from a string part and invalidate it.
 */
START_TEST(test_clear_string) {
  const uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  const size_t  size   = 11;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 8);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Clear part */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_clear(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_clear(&part));

  /* Assert part validity and error again */
  fail_if(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_error(&part));

  /* Assert journal size */
  fail_unless(pb_journal_empty(&journal));
  ck_assert_uint_eq(0, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear data from an invalid part.
 */
START_TEST(test_clear_invalid) {
  pb_part_t part = pb_part_create_invalid();

  /* Assert part validity and error */
  fail_if(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_error(&part));

  /* Clear part */
  ck_assert_uint_eq(PB_ERROR_INVALID, pb_part_clear(&part));

  /* Free all allocated memory */
  pb_part_destroy(&part);
} END_TEST

/*
 * Clear data from a a part for which reallocation fails.
 */
START_TEST(test_clear_invalid_resize) {
  const uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  const size_t  size   = 11;

  /* Patch allocator */
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_default.proc.resize,
      .free     = allocator_default.proc.free
    }
  };

  /* Create journal, message and part */
  pb_journal_t journal =
    pb_journal_create_with_allocator(&allocator, data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 8);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert part size and version */
  fail_if(pb_part_empty(&part));
  ck_assert_uint_eq(9, pb_part_size(&part));
  ck_assert_uint_eq(0, pb_part_version(&part));

  /* Patch allocator with faulty reallocation function */
  allocator.proc.resize = allocator_resize_fail;

  /* Clear part */
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_part_clear(&part));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_part_clear(&part));                      // SEMANTICS CHANGED: clearing a part

  /* Assert part validity and error again */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(11, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
  pb_message_destroy(&message);
  pb_journal_destroy(&journal);
} END_TEST

/*
 * Clear data from a string part and invalidate it.
 */
START_TEST(test_clear_invalid_zero_copy) {
  uint8_t data[] = { 66, 9, 83, 79, 77, 69, 32, 68, 65, 84, 65 };
  size_t  size   = 11;

  /* Create journal, message and part */
  pb_journal_t journal = pb_journal_create_zero_copy(data, size);
  pb_message_t message = pb_message_create(&descriptor, &journal);
  pb_part_t    part    = pb_part_create(&message, 8);

  /* Assert part validity and error */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Clear part */
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_part_clear(&part));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_part_clear(&part));

  /* Assert part validity and error again */
  fail_unless(pb_part_valid(&part));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_part_error(&part));

  /* Assert journal size */
  fail_if(pb_journal_empty(&journal));
  ck_assert_uint_eq(11, pb_journal_size(&journal));

  /* Free all allocated memory */
  pb_part_destroy(&part);
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
  void *suite = suite_create("protobluff/message/part"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_repeated);
  tcase_add_test(tcase, test_create_message_empty);
  tcase_add_test(tcase, test_create_message_merged);
  tcase_add_test(tcase, test_create_message_invalid);
  tcase_add_test(tcase, test_create_oneof);
  tcase_add_test(tcase, test_create_oneof_existing);
  tcase_add_test(tcase, test_create_oneof_existing_before);
  tcase_add_test(tcase, test_create_oneof_existing_after);
  tcase_add_test(tcase, test_create_from_journal);
  tcase_add_test(tcase, test_create_from_journal_invalid);
  tcase_add_test(tcase, test_create_from_cursor);
  tcase_add_test(tcase, test_create_from_cursor_invalid);
  tcase_add_test(tcase, test_create_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "write" */
  tcase = tcase_create("write");
  tcase_add_test(tcase, test_write);
  tcase_add_test(tcase, test_write_string);
  tcase_add_test(tcase, test_write_string_long);
  tcase_add_test(tcase, test_write_invalid);
  tcase_add_test(tcase, test_write_invalid_resize);
  tcase_add_test(tcase, test_write_invalid_zero_copy);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "clear" */
  tcase = tcase_create("clear");
  tcase_add_test(tcase, test_clear);
  tcase_add_test(tcase, test_clear_string);
  tcase_add_test(tcase, test_clear_invalid);
  tcase_add_test(tcase, test_clear_invalid_resize);
  tcase_add_test(tcase, test_clear_invalid_zero_copy);
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
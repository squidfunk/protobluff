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

#include "lib/binary.h"
#include "lib/binary/stream.h"
#include "lib/common.h"

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a stream over a binary.
 */
START_TEST(test_create) {
  const uint8_t data[] = { 11, 13, 17 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_left(&stream));

  /* Read next byte */
  uint8_t byte;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_stream_read(&stream, &byte));
  ck_assert_uint_eq(11, byte);

  /* Assert binary stream size and offset again */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(2, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a stream over a binary at a given offset.
 */
START_TEST(test_create_at) {
  const uint8_t data[] = { 11, 13, 17 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create_at(&binary, 1);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(2, pb_binary_stream_left(&stream));

  /* Read next byte */
  uint8_t byte;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_stream_read(&stream, &byte));
  ck_assert_uint_eq(13, byte);

  /* Assert binary stream size and offset again */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(2, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Create a copy of a binary stream.
 */
START_TEST(test_copy) {
  const uint8_t data[] = { 11, 13, 17 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);
  pb_binary_stream_t copy   = pb_binary_stream_copy(&stream);

  /* Assert binary stream copy size and offset */
  fail_if(pb_binary_stream_empty(&copy));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&copy);
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip a specific amount of bytes in a binary stream.
 */
START_TEST(test_skip) {
  const uint8_t data[] = { 11, 13, 17 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip bytes */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_stream_skip(&stream, 2));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(2, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_left(&stream));

  /* Read next byte */
  uint8_t byte;
  fail_if(pb_binary_stream_read(&stream, &byte));
  ck_assert_uint_eq(17, byte);

  /* Assert binary stream size and offset again */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip too many bytes in a binary stream.
 */
START_TEST(test_skip_underrun) {
  const uint8_t data[] = { 11, 13, 17 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip bytes */
  ck_assert_uint_eq(PB_ERROR_UNDERRUN, pb_binary_stream_skip(&stream, 10));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip a fixed-sized 32-bit value in a binary stream.
 */
START_TEST(test_skip_fixed32) {
  const uint8_t data[] = { 0, 0, 0, 0 };
  const size_t  size   = 4;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip 32-bit value (= 4 bytes) */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_stream_skip_fixed32(&stream));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip a fixed-sized 64-bit value in a binary stream.
 */
START_TEST(test_skip_fixed64) {
  const uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip 64-bit value (= 8 bytes) */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_stream_skip_fixed64(&stream));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip a variable-sized integer in a binary stream.
 */
START_TEST(test_skip_varint) {
  const uint8_t data[] = { 137, 138, 139, 13, 0, 0, 0, 0 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip variable-sized integer */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_stream_skip_varint(&stream));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip a variable-sized integer in an underruning binary stream.
 */
START_TEST(test_skip_varint_underrun) {
  const uint8_t data[] = { 137, 138, 139, 140 };
  const size_t  size   = 4;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip variable-sized integer */
  ck_assert_uint_eq(PB_ERROR_UNDERRUN, pb_binary_stream_skip_varint(&stream));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip a length-prefixed byte sequence in a binary stream.
 */
START_TEST(test_skip_length) {
  const uint8_t data[] = { 3, 0, 0, 0, 0, 0, 0, 0 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip length-prefixed byte sequence */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_stream_skip_length(&stream));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip a length-prefixed byte sequence with zero bytes in a binary stream.
 */
START_TEST(test_skip_length_zero) {
  const uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip length-prefixed byte sequence */
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_stream_skip_length(&stream));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(7, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip a length-prefixed byte sequence in an underruning binary stream.
 */
START_TEST(test_skip_length_underrun) {
  const uint8_t data[] = { 127, 0, 0, 0, 0, 0, 0, 0 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip length-prefixed byte sequence */
  ck_assert_uint_eq(PB_ERROR_UNDERRUN, pb_binary_stream_skip_length(&stream));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Skip a length-prefixed byte sequence in an underruning binary stream.
 */
START_TEST(test_skip_length_prefix_underrun) {
  const uint8_t data[] = { 137, 138, 139, 140 };
  const size_t  size   = 4;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Skip length-prefixed byte sequence */
  ck_assert_uint_eq(PB_ERROR_UNDERRUN, pb_binary_stream_skip_length(&stream));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a byte from a binary stream.
 */
START_TEST(test_read) {
  const uint8_t data[] = { 11, 13, 17 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create_at(&binary, 2);

  /* Read next byte */
  uint8_t byte;
  ck_assert_uint_eq(PB_ERROR_NONE, pb_binary_stream_read(&stream, &byte));
  ck_assert_uint_eq(17, byte);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a byte from an underruning binary stream.
 */
START_TEST(test_read_underrun) {
  const uint8_t data[] = { 11, 13, 17 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create_at(&binary, 3);

  /* Read next byte */
  uint8_t byte;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN, pb_binary_stream_read(&stream, &byte));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a fixed-sized 32-bit value from a binary stream.
 */
START_TEST(test_read_fixed32) {
  const uint8_t data[] = { 0, 202, 154, 59 };
  const size_t  size   = 4;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 32-bit fixed-sized integer */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_stream_read_fixed32(&stream, &value));
  ck_assert_uint_eq(1000000000, value);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a fixed-sized 32-bit value from an underruning binary stream.
 */
START_TEST(test_read_fixed32_underrun) {
  const uint8_t data[] = { 0, 202, 154 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 32-bit fixed-sized integer */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN,
    pb_binary_stream_read_fixed32(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a fixed-sized 64-bit value from a binary stream.
 */
START_TEST(test_read_fixed64) {
  const uint8_t data[] = { 0, 0, 100, 167, 179, 182, 224, 13 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 64-bit fixed-sized integer */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_stream_read_fixed64(&stream, &value));
  ck_assert_uint_eq(1000000000000000000, value);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a fixed-sized 64-bit value from an underruning binary stream.
 */
START_TEST(test_read_fixed64_underrun) {
  const uint8_t data[] = { 0, 0, 100, 167, 179, 182, 224 };
  const size_t  size   = 7;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 32-bit fixed-sized integer */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN,
    pb_binary_stream_read_fixed64(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(7, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(7, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a 8-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_varint8) {
  const uint8_t data[] = { 127, 0, 0 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 8-bit variabled-sized integer */
  uint8_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_stream_read_varint8(&stream, &value));
  ck_assert_uint_eq(127, value);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(2, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a 8-bit variable-sized integer from an underruning binary stream.
 */
START_TEST(test_read_varint8_underrun) {
  const uint8_t data[] = { 11, 13, 17 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create_at(&binary, 3);

  /* Read 8-bit variabled-sized integer */
  uint8_t value;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN,
    pb_binary_stream_read_varint8(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read an overflowing 8-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_varint8_overflow) {
  const uint8_t data[] = { 137, 0, 0 };
  const size_t  size   = 3;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 8-bit variabled-sized integer */
  uint8_t value;
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_stream_read_varint8(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(2, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a 32-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_varint32) {
  const uint8_t data[] = { 128, 148, 235, 220, 3 };
  const size_t  size   = 5;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 32-bit variabled-sized integer */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_stream_read_varint32(&stream, &value));
  ck_assert_uint_eq(1000000000, value);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(5, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(5, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a 32-bit variable-sized integer from an underruning binary stream.
 */
START_TEST(test_read_varint32_underrun) {
  const uint8_t data[] = { 128, 148, 235, 220 };
  const size_t  size   = 4;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 32-bit variabled-sized integer */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN,
    pb_binary_stream_read_varint32(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read an overflowing 32-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_varint32_overflow) {
  const uint8_t data[] = { 137, 138, 139, 140, 141, 142 };
  const size_t  size   = 6;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 32-bit variabled-sized integer */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_stream_read_varint32(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(6, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(5, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a 64-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_varint64) {
  const uint8_t data[] = { 128, 128, 144, 187, 186, 214, 173, 240, 13, 0 };
  const size_t  size   = 10;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 64-bit variabled-sized integer */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_stream_read_varint64(&stream, &value));
  ck_assert_uint_eq(1000000000000000000, value);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(9,  pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(1,  pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a 64-bit variable-sized integer from an underruning binary stream.
 */
START_TEST(test_read_varint64_underrun) {
  const uint8_t data[] = { 128, 128, 144, 187, 186, 214, 173, 240 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 64-bit variabled-sized integer */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN,
    pb_binary_stream_read_varint64(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read an overflowing 64-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_varint64_overflow) {
  const uint8_t data[] = { 137, 138, 139, 140, 141, 142, 143, 144, 145, 146 };
  const size_t  size   = 10;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read 64-bit variabled-sized integer */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_stream_read_varint64(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(10, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0,  pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a zig-zag encoded 32-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_svarint32) {
  const uint8_t data[] = { 128, 182, 202, 145, 6 };
  const size_t  size   = 5;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read signed 32-bit variabled-sized integer */
  int32_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_stream_read_svarint32(&stream, &value));
  ck_assert_int_eq(-1000000000, value);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(5, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(5, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a 32-bit variable-sized integer from an underruning binary stream.
 */
START_TEST(test_read_svarint32_underrun) {
  const uint8_t data[] = { 128, 182, 202, 145 };
  const size_t  size   = 4;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read signed 32-bit variabled-sized integer */
  int32_t value;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN,
    pb_binary_stream_read_svarint32(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read an overflowing 32-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_svarint32_overflow) {
  const uint8_t data[] = { 137, 138, 139, 140, 141, 142 };
  const size_t  size   = 6;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read signed 32-bit variabled-sized integer */
  int32_t value;
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_stream_read_svarint32(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(6, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(5, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a zig-zag encoded 64-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_svarint64) {
  const uint8_t data[] = { 128, 128, 184, 226, 226, 148, 233, 135, 121, 0 };
  const size_t  size   = 10;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read signed 64-bit variabled-sized integer */
  int64_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_stream_read_svarint64(&stream, &value));
  ck_assert_int_eq(-1000000000000000000, value);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(9,  pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(1,  pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a 64-bit variable-sized integer from an underruning binary stream.
 */
START_TEST(test_read_svarint64_underrun) {
  const uint8_t data[] = { 128, 128, 184, 226, 226, 148, 233, 135 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read signed 64-bit variabled-sized integer */
  int64_t value;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN,
    pb_binary_stream_read_svarint64(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read an overflowing 64-bit variable-sized integer from a binary stream.
 */
START_TEST(test_read_svarint64_overflow) {
  const uint8_t data[] = { 137, 138, 139, 140, 141, 142, 143, 144, 145, 146 };
  const size_t  size   = 10;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read signed 64-bit variabled-sized integer */
  int64_t value;
  ck_assert_uint_eq(PB_ERROR_OVERFLOW,
    pb_binary_stream_read_svarint64(&stream, &value));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(10, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0,  pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a length-prefixed byte sequence from a binary stream.
 */
START_TEST(test_read_length) {
  const uint8_t data[] = { 3, 0, 0, 0, 0, 0, 0, 0 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read length-prefixed byte sequence */
  pb_string_t string;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_stream_read_length(&stream, &string));
  fail_if(memcmp(&(data[1]), string.data, string.size));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a length-prefixed byte sequence with zero bytes from a binary stream.
 */
START_TEST(test_read_length_zero) {
  const uint8_t data[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read length-prefixed byte sequence */
  pb_string_t string;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_binary_stream_read_length(&stream, &string));
  ck_assert_uint_eq(0, string.size);

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(1, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(7, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a length-prefixed byte sequence in an underruning binary stream.
 */
START_TEST(test_read_length_underrun) {
  const uint8_t data[] = { 127, 0, 0, 0, 0, 0, 0, 0 };
  const size_t  size   = 8;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read length-prefixed byte sequence */
  pb_string_t string;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN,
    pb_binary_stream_read_length(&stream, &string));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(8, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
  pb_binary_destroy(&binary);
} END_TEST

/*
 * Read a length-prefixed byte sequence in an underruning binary stream.
 */
START_TEST(test_read_length_prefix_underrun) {
  const uint8_t data[] = { 137, 138, 139, 140 };
  const size_t  size   = 4;

  /* Create binary and stream */
  pb_binary_t binary = pb_binary_create(data, size);
  pb_binary_stream_t stream = pb_binary_stream_create(&binary);

  /* Read length-prefixed byte sequence */
  pb_string_t string;
  ck_assert_uint_eq(PB_ERROR_UNDERRUN,
    pb_binary_stream_read_length(&stream, &string));

  /* Assert binary stream size and offset */
  fail_if(pb_binary_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_size(&stream));
  ck_assert_uint_eq(4, pb_binary_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_binary_stream_left(&stream));

  /* Free all allocated memory */
  pb_binary_stream_destroy(&stream);
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
  void *suite = suite_create("protobluff/binary/stream"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_at);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "copy" */
  tcase = tcase_create("copy");
  tcase_add_test(tcase, test_copy);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "skip" */
  tcase = tcase_create("skip");
  tcase_add_test(tcase, test_skip);
  tcase_add_test(tcase, test_skip_underrun);
  tcase_add_test(tcase, test_skip_fixed32);
  tcase_add_test(tcase, test_skip_fixed64);
  tcase_add_test(tcase, test_skip_varint);
  tcase_add_test(tcase, test_skip_varint_underrun);
  tcase_add_test(tcase, test_skip_length);
  tcase_add_test(tcase, test_skip_length_zero);
  tcase_add_test(tcase, test_skip_length_underrun);
  tcase_add_test(tcase, test_skip_length_prefix_underrun);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "read" */
  tcase = tcase_create("read");
  tcase_add_test(tcase, test_read);
  tcase_add_test(tcase, test_read_underrun);
  tcase_add_test(tcase, test_read_fixed32);
  tcase_add_test(tcase, test_read_fixed32_underrun);
  tcase_add_test(tcase, test_read_fixed64);
  tcase_add_test(tcase, test_read_fixed64_underrun);
  tcase_add_test(tcase, test_read_varint8);
  tcase_add_test(tcase, test_read_varint8_underrun);
  tcase_add_test(tcase, test_read_varint8_overflow);
  tcase_add_test(tcase, test_read_varint32);
  tcase_add_test(tcase, test_read_varint32_underrun);
  tcase_add_test(tcase, test_read_varint32_overflow);
  tcase_add_test(tcase, test_read_varint64);
  tcase_add_test(tcase, test_read_varint64_underrun);
  tcase_add_test(tcase, test_read_varint64_overflow);
  tcase_add_test(tcase, test_read_svarint32);
  tcase_add_test(tcase, test_read_svarint32_underrun);
  tcase_add_test(tcase, test_read_svarint32_overflow);
  tcase_add_test(tcase, test_read_svarint64);
  tcase_add_test(tcase, test_read_svarint64_underrun);
  tcase_add_test(tcase, test_read_svarint64_overflow);
  tcase_add_test(tcase, test_read_length);
  tcase_add_test(tcase, test_read_length_zero);
  tcase_add_test(tcase, test_read_length_underrun);
  tcase_add_test(tcase, test_read_length_prefix_underrun);
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
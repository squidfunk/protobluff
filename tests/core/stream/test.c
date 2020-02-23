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

#include "core/buffer.h"
#include "core/common.h"
#include "core/stream.h"

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create a stream over a buffer.
 */
START_TEST(test_create) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 1 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(10, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Create a stream over a buffer at a given offset.
 */
START_TEST(test_create_at) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 1 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create_at(&buffer, 5);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(5, pb_stream_offset(&stream));
  ck_assert_uint_eq(5, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Advance a stream by the given number of bytes.
 */
START_TEST(test_advance) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 1 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(10, pb_stream_left(&stream));

  /* Advance a stream by the given number of bytes */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_advance(&stream, 5));

  /* Assert stream offset again */
  ck_assert_uint_eq(5, pb_stream_offset(&stream));
  ck_assert_uint_eq(5, pb_stream_left(&stream));

  /* Advance a stream by the given number of bytes */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_advance(&stream, 5));

  /* Assert stream offset again */
  ck_assert_uint_eq(10, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Advance a stream by zero bytes.
 */
START_TEST(test_advance_zero) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 1 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(10, pb_stream_left(&stream));

  /* Advance a stream by the given number of bytes */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_advance(&stream, 0));

  /* Assert stream offset again */
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(10, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Advance a stream by too many bytes.
 */
START_TEST(test_advance_underrun) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 1 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(10, pb_stream_left(&stream));

  /* Advance a stream by the given number of bytes */
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_stream_advance(&stream, 11));

  /* Assert stream offset again */
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(10, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a value of given type.
 */
START_TEST(test_read) {
  const uint8_t data[] = { 128, 128, 144, 187, 186, 214, 173, 240, 13 };
  const size_t  size   = 9;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a variable-sized integer */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_UINT64, &value));
  ck_assert_uint_eq(1000000000000000000ULL, value);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(9, pb_stream_size(&stream));
  ck_assert_uint_eq(9, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a variable-sized integer.
 */
START_TEST(test_read_varint) {
  const uint8_t data[] = { 128, 128, 144, 187, 186, 214, 173, 240, 13 };
  const size_t  size   = 9;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a variable-sized integer */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_UINT64, &value));
  ck_assert_uint_eq(1000000000000000000ULL, value);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(9, pb_stream_size(&stream));
  ck_assert_uint_eq(9, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a repeated variable-sized integer.
 */
START_TEST(test_read_varint_repeated) {
  const uint8_t data[] = { 128, 148, 235, 220, 3, 128, 148, 235, 220, 3 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a variable-sized integer */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_UINT32, &value));
  ck_assert_uint_eq(1000000000U, value);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(5, pb_stream_offset(&stream));
  ck_assert_uint_eq(5, pb_stream_left(&stream));

  /* Read another variable-sized integer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_UINT32, &value));
  ck_assert_uint_eq(1000000000U, value);

  /* Assert stream size and offset again */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(10, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read an invalid variable-sized integer.
 */
START_TEST(test_read_varint_invalid) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a variable-sized integer */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_VARINT,
    pb_stream_read(&stream, PB_TYPE_UINT64, &value));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(10, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a fixed-sized 64-bit value.
 */
START_TEST(test_read_64bit) {
  const uint8_t data[] = { 0, 0, 100, 167, 179, 182, 224, 13 };
  const size_t  size   = 8;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a fixed-sized 64-bit value */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_FIXED64, &value));
  ck_assert_uint_eq(1000000000000000000ULL, value);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_stream_size(&stream));
  ck_assert_uint_eq(8, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a repeated fixed-sized 64-bit value.
 */
START_TEST(test_read_64bit_repeated) {
  const uint8_t data[] = { 0, 0, 100, 167, 179, 182, 224, 13,
                           0, 0, 100, 167, 179, 182, 224, 13 };
  const size_t  size   = 16;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a fixed-sized 64-bit value */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_FIXED64, &value));
  ck_assert_uint_eq(1000000000000000000ULL, value);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(16, pb_stream_size(&stream));
  ck_assert_uint_eq(8, pb_stream_offset(&stream));
  ck_assert_uint_eq(8, pb_stream_left(&stream));

  /* Read another fixed-sized 64-bit value */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_FIXED64, &value));
  ck_assert_uint_eq(1000000000000000000ULL, value);

  /* Assert stream size and offset again */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(16, pb_stream_size(&stream));
  ck_assert_uint_eq(16, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read an underrunning fixed-sized 64-bit value.
 */
START_TEST(test_read_64bit_underrun) {
  const uint8_t data[] = { 0, 0, 100, 167, 179, 182, 224 };
  const size_t  size   = 7;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a fixed-sized 64-bit value */
  uint64_t value;
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_stream_read(&stream, PB_TYPE_FIXED64, &value));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(7, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(7, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a length-prefixed value.
 */
START_TEST(test_read_length) {
  const uint8_t data[] = { 4, 0, 0, 0, 0 };
  const size_t  size   = 5;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a length-prefixed value */
  pb_string_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_BYTES, &value));
  fail_if(memcmp(&(data[1]),
    pb_string_data(&value), pb_string_size(&value)));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(5, pb_stream_size(&stream));
  ck_assert_uint_eq(5, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a length-prefixed value of zero length.
 */
START_TEST(test_read_length_zero) {
  const uint8_t data[] = { 0 };
  const size_t  size   = 1;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a length-prefixed value */
  pb_string_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_BYTES, &value));
  ck_assert_uint_eq(0, pb_string_size(&value));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(1, pb_stream_size(&stream));
  ck_assert_uint_eq(1, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a repeated length-prefixed value.
 */
START_TEST(test_read_length_repeated) {
  const uint8_t data[] = { 4, 0, 0, 0, 0, 4, 0, 0, 0, 0 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a length-prefixed value */
  pb_string_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_BYTES, &value));
  fail_if(memcmp(&(data[1]),
    pb_string_data(&value), pb_string_size(&value)));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(5, pb_stream_offset(&stream));
  ck_assert_uint_eq(5, pb_stream_left(&stream));

  /* Read another length-prefixed value */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_BYTES, &value));
  fail_if(memcmp(&(data[6]),
    pb_string_data(&value), pb_string_size(&value)));

  /* Assert stream size and offset again */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(10, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read an underrunning length-prefixed value.
 */
START_TEST(test_read_length_underrun) {
  const uint8_t data[] = { 4, 0, 0, 0 };
  const size_t  size   = 4;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a length-prefixed value */
  pb_string_t value;
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_stream_read(&stream, PB_TYPE_BYTES, &value));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(4, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a length-prefixed value with an underruning length prefix.
 */
START_TEST(test_read_length_prefix_underrun) {
  const uint8_t data[] = { 255 };
  const size_t  size   = 1;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a length-prefixed value */
  pb_string_t value;
  ck_assert_uint_eq(PB_ERROR_VARINT,
    pb_stream_read(&stream, PB_TYPE_BYTES, &value));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(1, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(1, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a fixed-sized 32-bit value.
 */
START_TEST(test_read_32bit) {
  const uint8_t data[] = { 0, 202, 154, 59 };
  const size_t  size   = 4;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a fixed-sized 32-bit value */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_FIXED32, &value));
  ck_assert_uint_eq(1000000000U, value);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_stream_size(&stream));
  ck_assert_uint_eq(4, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read a repeated fixed-sized 32-bit value.
 */
START_TEST(test_read_32bit_repeated) {
  const uint8_t data[] = { 0, 202, 154, 59, 0, 202, 154, 59 };
  const size_t  size   = 8;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a fixed-sized 32-bit value */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_FIXED32, &value));
  ck_assert_uint_eq(1000000000U, value);

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_stream_size(&stream));
  ck_assert_uint_eq(4, pb_stream_offset(&stream));
  ck_assert_uint_eq(4, pb_stream_left(&stream));

  /* Read another fixed-sized 32-bit value */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_read(&stream, PB_TYPE_FIXED32, &value));
  ck_assert_uint_eq(1000000000U, value);

  /* Assert stream size and offset again */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_stream_size(&stream));
  ck_assert_uint_eq(8, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Read an underrunning fixed-sized 32-bit value.
 */
START_TEST(test_read_32bit_underrun) {
  const uint8_t data[] = { 0, 202, 154 };
  const size_t  size   = 3;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Read a fixed-sized 32-bit value */
  uint32_t value;
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_stream_read(&stream, PB_TYPE_FIXED32, &value));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(3, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip a value of given wiretype.
 */
START_TEST(test_skip) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 1 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a variable-sized integer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_skip(&stream, PB_WIRETYPE_VARINT));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(10, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip a variable-sized integer.
 */
START_TEST(test_skip_varint) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 1 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a variable-sized integer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_skip(&stream, PB_WIRETYPE_VARINT));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(10, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip an invalid variable-sized integer.
 */
START_TEST(test_skip_varint_invalid) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
  const size_t  size   = 10;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a variable-sized integer */
  ck_assert_uint_eq(PB_ERROR_VARINT,
    pb_stream_skip(&stream, PB_WIRETYPE_VARINT));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(10, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(10, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip a fixed-sized 64-bit value.
 */
START_TEST(test_skip_64bit) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255 };
  const size_t  size   = 8;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a fixed-sized 64-bit integer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_skip(&stream, PB_WIRETYPE_64BIT));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(8, pb_stream_size(&stream));
  ck_assert_uint_eq(8, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip an underrunning fixed-sized 64-bit value.
 */
START_TEST(test_skip_64bit_underrun) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255 };
  const size_t  size   = 7;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a fixed-sized 64-bit integer */
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_stream_skip(&stream, PB_WIRETYPE_64BIT));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(7, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(7, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip a length-prefixed value.
 */
START_TEST(test_skip_length) {
  const uint8_t data[] = { 4, 0, 0, 0, 0 };
  const size_t  size   = 5;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a length-prefixed value */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_skip(&stream, PB_WIRETYPE_LENGTH));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(5, pb_stream_size(&stream));
  ck_assert_uint_eq(5, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip a length-prefixed value of zero length.
 */
START_TEST(test_skip_length_zero) {
  const uint8_t data[] = { 0 };
  const size_t  size   = 1;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a length-prefixed value */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_skip(&stream, PB_WIRETYPE_LENGTH));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(1, pb_stream_size(&stream));
  ck_assert_uint_eq(1, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip an underrunning length-prefixed value.
 */
START_TEST(test_skip_length_underrun) {
  const uint8_t data[] = { 4, 0, 0, 0 };
  const size_t  size   = 4;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a length-prefixed value */
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_stream_skip(&stream, PB_WIRETYPE_LENGTH));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(4, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip a length-prefixed value with an underruning length prefix.
 */
START_TEST(test_skip_length_prefix_underrun) {
  const uint8_t data[] = { 255 };
  const size_t  size   = 1;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a length-prefixed value */
  ck_assert_uint_eq(PB_ERROR_VARINT,
    pb_stream_skip(&stream, PB_WIRETYPE_LENGTH));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(1, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(1, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip a fixed-sized 32-bit value.
 */
START_TEST(test_skip_32bit) {
  const uint8_t data[] = { 255, 255, 255, 255 };
  const size_t  size   = 4;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a fixed-sized 32-bit integer */
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_stream_skip(&stream, PB_WIRETYPE_32BIT));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(4, pb_stream_size(&stream));
  ck_assert_uint_eq(4, pb_stream_offset(&stream));
  ck_assert_uint_eq(0, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
  pb_buffer_destroy(&buffer);
} END_TEST

/*
 * Skip an underrunning fixed-sized 32-bit value.
 */
START_TEST(test_skip_32bit_underrun) {
  const uint8_t data[] = { 255, 255, 255 };
  const size_t  size   = 3;

  /* Create buffer and stream */
  pb_buffer_t buffer = pb_buffer_create(data, size);
  pb_stream_t stream = pb_stream_create(&buffer);

  /* Skip a fixed-sized 32-bit integer */
  ck_assert_uint_eq(PB_ERROR_OFFSET,
    pb_stream_skip(&stream, PB_WIRETYPE_32BIT));

  /* Assert stream size and offset */
  fail_if(pb_stream_empty(&stream));
  ck_assert_uint_eq(3, pb_stream_size(&stream));
  ck_assert_uint_eq(0, pb_stream_offset(&stream));
  ck_assert_uint_eq(3, pb_stream_left(&stream));

  /* Free all allocated memory */
  pb_stream_destroy(&stream);
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
  void *suite = suite_create("protobluff/core/stream"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_at);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "advance" */
  tcase = tcase_create("advance");
  tcase_add_test(tcase, test_advance);
  tcase_add_test(tcase, test_advance_zero);
  tcase_add_test(tcase, test_advance_underrun);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "read" */
  tcase = tcase_create("read");
  tcase_add_test(tcase, test_read);
  tcase_add_test(tcase, test_read_varint);
  tcase_add_test(tcase, test_read_varint_repeated);
  tcase_add_test(tcase, test_read_varint_invalid);
  tcase_add_test(tcase, test_read_64bit);
  tcase_add_test(tcase, test_read_64bit_repeated);
  tcase_add_test(tcase, test_read_64bit_underrun);
  tcase_add_test(tcase, test_read_length);
  tcase_add_test(tcase, test_read_length_zero);
  tcase_add_test(tcase, test_read_length_repeated);
  tcase_add_test(tcase, test_read_length_underrun);
  tcase_add_test(tcase, test_read_length_prefix_underrun);
  tcase_add_test(tcase, test_read_32bit);
  tcase_add_test(tcase, test_read_32bit_repeated);
  tcase_add_test(tcase, test_read_32bit_underrun);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "skip" */
  tcase = tcase_create("skip");
  tcase_add_test(tcase, test_skip);
  tcase_add_test(tcase, test_skip_varint);
  tcase_add_test(tcase, test_skip_varint_invalid);
  tcase_add_test(tcase, test_skip_64bit);
  tcase_add_test(tcase, test_skip_64bit_underrun);
  tcase_add_test(tcase, test_skip_length);
  tcase_add_test(tcase, test_skip_length_zero);
  tcase_add_test(tcase, test_skip_length_underrun);
  tcase_add_test(tcase, test_skip_length_prefix_underrun);
  tcase_add_test(tcase, test_skip_32bit);
  tcase_add_test(tcase, test_skip_32bit_underrun);
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

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

#include "core/common.h"
#include "core/varint.h"

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Retrieve the packed size of a signed 32-bit variable-sized integer.
 */
START_TEST(test_size_int32) {
  int32_t value = 0;
  ck_assert_uint_eq(1, pb_varint_size_int32(&value));

  /* Assert size of 1-byte upper boundary */
  value = (1 << 7) - 1;
  ck_assert_uint_eq(1, pb_varint_size_int32(&value));

  /* Assert size of 2-byte lower boundary */
  value = (1 << 7);
  ck_assert_uint_eq(2, pb_varint_size_int32(&value));

  /* Assert size of 2-byte upper boundary */
  value = (1 << 14) - 1;
  ck_assert_uint_eq(2, pb_varint_size_int32(&value));

  /* Assert size of 3-byte lower boundary */
  value = (1 << 14);
  ck_assert_uint_eq(3, pb_varint_size_int32(&value));

  /* Assert size of 3-byte upper boundary */
  value = (1 << 21) - 1;
  ck_assert_uint_eq(3, pb_varint_size_int32(&value));

  /* Assert size of 4-byte lower boundary */
  value = (1 << 21);
  ck_assert_uint_eq(4, pb_varint_size_int32(&value));

  /* Assert size of 4-byte upper boundary */
  value = (1 << 28) - 1;
  ck_assert_uint_eq(4, pb_varint_size_int32(&value));

  /* Assert size of 5-byte lower boundary */
  value = (1 << 28);
  ck_assert_uint_eq(5, pb_varint_size_int32(&value));

  /* Assert size of maximum positive value */
  value = INT32_MAX;
  ck_assert_uint_eq(5, pb_varint_size_int32(&value));

  /* Assert size of negative values */
  value = -1;
  ck_assert_uint_eq(10, pb_varint_size_int32(&value));

  /* Assert size of minimum negative value */
  value = INT32_MIN;
  ck_assert_uint_eq(10, pb_varint_size_int32(&value));
} END_TEST

/*
 * Retrieve the packed size of a signed 64-bit variable-sized integer.
 */
START_TEST(test_size_int64) {
  int64_t value = 0;
  ck_assert_uint_eq(1, pb_varint_size_int64(&value));

  /* Assert size of 5-byte lower boundary */
  value = (1LL << 28);
  ck_assert_uint_eq(5, pb_varint_size_int64(&value));

  /* Assert size of 5-byte upper boundary */
  value = (1LL << 35) - 1;
  ck_assert_uint_eq(5, pb_varint_size_int64(&value));

  /* Assert size of 6-byte lower boundary */
  value = (1LL << 35);
  ck_assert_uint_eq(6, pb_varint_size_int64(&value));

  /* Assert size of 6-byte upper boundary */
  value = (1LL << 42) - 1;
  ck_assert_uint_eq(6, pb_varint_size_int64(&value));

  /* Assert size of 7-byte lower boundary */
  value = (1LL << 42);
  ck_assert_uint_eq(7, pb_varint_size_int64(&value));

  /* Assert size of 7-byte upper boundary */
  value = (1LL << 49) - 1;
  ck_assert_uint_eq(7, pb_varint_size_int64(&value));

  /* Assert size of 8-byte lower boundary */
  value = (1LL << 49);
  ck_assert_uint_eq(8, pb_varint_size_int64(&value));

  /* Assert size of 8-byte upper boundary */
  value = (1LL << 56) - 1;
  ck_assert_uint_eq(8, pb_varint_size_int64(&value));

  /* Assert size of 9-byte lower boundary */
  value = (1LL << 56);
  ck_assert_uint_eq(9, pb_varint_size_int64(&value));

  /* Assert size of maximum positive value */
  value = INT64_MAX;
  ck_assert_uint_eq(9, pb_varint_size_int64(&value));

  /* Assert size of negative values */
  value = -1;
  ck_assert_uint_eq(10, pb_varint_size_int64(&value));

  /* Assert size of minimum negative value */
  value = INT64_MIN;
  ck_assert_uint_eq(10, pb_varint_size_int64(&value));
} END_TEST

/*
 * Retrieve the packed size of an unsigned 8-bit variable-sized integer.
 */
START_TEST(test_size_uint8) {
  uint32_t value = 0;
  ck_assert_uint_eq(1, pb_varint_size_uint8(&value));

  /* Assert size of 1-byte upper boundary */
  value = (1U << 7) - 1;
  ck_assert_uint_eq(1, pb_varint_size_uint8(&value));

  /* Assert size of 2-byte lower boundary */
  value = (1U << 8) - 1;
  ck_assert_uint_eq(2, pb_varint_size_uint8(&value));

  /* Assert size of maximum positive value */
  value = UINT8_MAX;
  ck_assert_uint_eq(2, pb_varint_size_uint8(&value));
} END_TEST

/*
 * Retrieve the packed size of an unsigned 32-bit variable-sized integer.
 */
START_TEST(test_size_uint32) {
  uint32_t value = 0;
  ck_assert_uint_eq(1, pb_varint_size_uint32(&value));

  /* Assert size of 1-byte upper boundary */
  value = (1U << 7) - 1;
  ck_assert_uint_eq(1, pb_varint_size_uint32(&value));

  /* Assert size of 2-byte lower boundary */
  value = (1U << 7);
  ck_assert_uint_eq(2, pb_varint_size_uint32(&value));

  /* Assert size of 2-byte upper boundary */
  value = (1U << 14) - 1;
  ck_assert_uint_eq(2, pb_varint_size_uint32(&value));

  /* Assert size of 3-byte lower boundary */
  value = (1U << 14);
  ck_assert_uint_eq(3, pb_varint_size_uint32(&value));

  /* Assert size of 3-byte upper boundary */
  value = (1U << 21) - 1;
  ck_assert_uint_eq(3, pb_varint_size_uint32(&value));

  /* Assert size of 4-byte lower boundary */
  value = (1U << 21);
  ck_assert_uint_eq(4, pb_varint_size_uint32(&value));

  /* Assert size of 4-byte upper boundary */
  value = (1U << 28) - 1;
  ck_assert_uint_eq(4, pb_varint_size_uint32(&value));

  /* Assert size of 5-byte lower boundary */
  value = (1U << 28);
  ck_assert_uint_eq(5, pb_varint_size_uint32(&value));

  /* Assert size of maximum positive value */
  value = UINT32_MAX;
  ck_assert_uint_eq(5, pb_varint_size_uint32(&value));
} END_TEST

/*
 * Retrieve the packed size of an unsigned 64-bit variable-sized integer.
 */
START_TEST(test_size_uint64) {
  uint64_t value = 0;
  ck_assert_uint_eq(1, pb_varint_size_uint64(&value));

  /* Assert size of 5-byte lower boundary */
  value = (1ULL << 28);
  ck_assert_uint_eq(5, pb_varint_size_uint64(&value));

  /* Assert size of 5-byte upper boundary */
  value = (1ULL << 35) - 1;
  ck_assert_uint_eq(5, pb_varint_size_uint64(&value));

  /* Assert size of 6-byte lower boundary */
  value = (1ULL << 35);
  ck_assert_uint_eq(6, pb_varint_size_uint64(&value));

  /* Assert size of 6-byte upper boundary */
  value = (1ULL << 42) - 1;
  ck_assert_uint_eq(6, pb_varint_size_uint64(&value));

  /* Assert size of 7-byte lower boundary */
  value = (1ULL << 42);
  ck_assert_uint_eq(7, pb_varint_size_uint64(&value));

  /* Assert size of 7-byte upper boundary */
  value = (1ULL << 49) - 1;
  ck_assert_uint_eq(7, pb_varint_size_uint64(&value));

  /* Assert size of 8-byte lower boundary */
  value = (1ULL << 49);
  ck_assert_uint_eq(8, pb_varint_size_uint64(&value));

  /* Assert size of 8-byte upper boundary */
  value = (1ULL << 56) - 1;
  ck_assert_uint_eq(8, pb_varint_size_uint64(&value));

  /* Assert size of 9-byte lower boundary */
  value = (1ULL << 56);
  ck_assert_uint_eq(9, pb_varint_size_uint64(&value));

  /* Assert size of 9-byte upper boundary */
  value = (1ULL << 63) - 1;
  ck_assert_uint_eq(9, pb_varint_size_uint64(&value));

  /* Assert size of maximum positive value */
  value = UINT64_MAX;
  ck_assert_uint_eq(10, pb_varint_size_uint64(&value));
} END_TEST

/*
 * Retrieve the packed size of a signed 32-bit variable-sized integer.
 */
START_TEST(test_size_sint32) {
  int32_t value = 0;
  ck_assert_uint_eq(1, pb_varint_size_sint32(&value));

  /* Assert size of 1-byte upper boundary */
  value = (1 << 6) - 1;
  ck_assert_uint_eq(1, pb_varint_size_sint32(&value));

  /* Assert size of 2-byte lower boundary */
  value = (1 << 6);
  ck_assert_uint_eq(2, pb_varint_size_sint32(&value));

  /* Assert size of 2-byte upper boundary */
  value = (1 << 13) - 1;
  ck_assert_uint_eq(2, pb_varint_size_sint32(&value));

  /* Assert size of 3-byte lower boundary */
  value = (1 << 13);
  ck_assert_uint_eq(3, pb_varint_size_sint32(&value));

  /* Assert size of 3-byte upper boundary */
  value = (1 << 20) - 1;
  ck_assert_uint_eq(3, pb_varint_size_sint32(&value));

  /* Assert size of 4-byte lower boundary */
  value = (1 << 20);
  ck_assert_uint_eq(4, pb_varint_size_sint32(&value));

  /* Assert size of 4-byte upper boundary */
  value = (1 << 27) - 1;
  ck_assert_uint_eq(4, pb_varint_size_sint32(&value));

  /* Assert size of 5-byte lower boundary */
  value = (1 << 27);
  ck_assert_uint_eq(5, pb_varint_size_sint32(&value));

  /* Assert size of maximum positive value */
  value = INT32_MAX;
  ck_assert_uint_eq(5, pb_varint_size_sint32(&value));

  /* Assert size of 1-byte upper boundary */
  value = -(1 << 6);
  ck_assert_uint_eq(1, pb_varint_size_sint32(&value));

  /* Assert size of 2-byte lower boundary */
  value = -(1 << 6) - 1;
  ck_assert_uint_eq(2, pb_varint_size_sint32(&value));

  /* Assert size of 2-byte upper boundary */
  value = -(1 << 13);
  ck_assert_uint_eq(2, pb_varint_size_sint32(&value));

  /* Assert size of 2-byte lower boundary */
  value = -(1 << 13) - 1;
  ck_assert_uint_eq(3, pb_varint_size_sint32(&value));

  /* Assert size of 3-byte upper boundary */
  value = -(1 << 20);
  ck_assert_uint_eq(3, pb_varint_size_sint32(&value));

  /* Assert size of 4-byte lower boundary */
  value = -(1 << 20) - 1;
  ck_assert_uint_eq(4, pb_varint_size_sint32(&value));

  /* Assert size of 4-byte upper boundary */
  value = -(1 << 27);
  ck_assert_uint_eq(4, pb_varint_size_sint32(&value));

  /* Assert size of 5-byte lower boundary */
  value = -(1 << 27) - 1;
  ck_assert_uint_eq(5, pb_varint_size_sint32(&value));

  /* Assert size of maximum negative value */
  value = INT32_MIN;
  ck_assert_uint_eq(5, pb_varint_size_sint32(&value));
} END_TEST

/*
 * Retrieve the packed size of a signed 64-bit variable-sized integer.
 */
START_TEST(test_size_sint64) {
  int64_t value = 0;
  ck_assert_uint_eq(1, pb_varint_size_sint64(&value));

  /* Assert size of 5-byte lower boundary */
  value = (1LL << 27);
  ck_assert_uint_eq(5, pb_varint_size_sint64(&value));

  /* Assert size of 5-byte upper boundary */
  value = (1LL << 34) - 1;
  ck_assert_uint_eq(5, pb_varint_size_sint64(&value));

  /* Assert size of 6-byte lower boundary */
  value = (1LL << 34);
  ck_assert_uint_eq(6, pb_varint_size_sint64(&value));

  /* Assert size of 6-byte upper boundary */
  value = (1LL << 41) - 1;
  ck_assert_uint_eq(6, pb_varint_size_sint64(&value));

  /* Assert size of 7-byte lower boundary */
  value = (1LL << 41);
  ck_assert_uint_eq(7, pb_varint_size_sint64(&value));

  /* Assert size of 7-byte upper boundary */
  value = (1LL << 48) - 1;
  ck_assert_uint_eq(7, pb_varint_size_sint64(&value));

  /* Assert size of 8-byte lower boundary */
  value = (1LL << 48);
  ck_assert_uint_eq(8, pb_varint_size_sint64(&value));

  /* Assert size of 8-byte upper boundary */
  value = (1LL << 55) - 1;
  ck_assert_uint_eq(8, pb_varint_size_sint64(&value));

  /* Assert size of 9-byte lower boundary */
  value = (1LL << 55);
  ck_assert_uint_eq(9, pb_varint_size_sint64(&value));

  /* Assert size of 9-byte upper boundary */
  value = (1LL << 62) - 1;
  ck_assert_uint_eq(9, pb_varint_size_sint64(&value));

  /* Assert size of maximum positive value */
  value = INT64_MAX;
  ck_assert_uint_eq(10, pb_varint_size_sint64(&value));

  /* Assert size of 5-byte lower boundary */
  value = -(1LL << 27) - 1;
  ck_assert_uint_eq(5, pb_varint_size_sint64(&value));

  /* Assert size of 5-byte upper boundary */
  value = -(1LL << 34);
  ck_assert_uint_eq(5, pb_varint_size_sint64(&value));

  /* Assert size of 6-byte lower boundary */
  value = -(1LL << 34) - 1;
  ck_assert_uint_eq(6, pb_varint_size_sint64(&value));

  /* Assert size of 6-byte upper boundary */
  value = -(1LL << 41);
  ck_assert_uint_eq(6, pb_varint_size_sint64(&value));

  /* Assert size of 7-byte lower boundary */
  value = -(1LL << 41) - 1;
  ck_assert_uint_eq(7, pb_varint_size_sint64(&value));

  /* Assert size of 7-byte upper boundary */
  value = -(1LL << 48);
  ck_assert_uint_eq(7, pb_varint_size_sint64(&value));

  /* Assert size of 8-byte lower boundary */
  value = -(1LL << 48) - 1;
  ck_assert_uint_eq(8, pb_varint_size_sint64(&value));

  /* Assert size of 8-byte upper boundary */
  value = -(1LL << 55);
  ck_assert_uint_eq(8, pb_varint_size_sint64(&value));

  /* Assert size of 9-byte lower boundary */
  value = -(1LL << 55) - 1;
  ck_assert_uint_eq(9, pb_varint_size_sint64(&value));

  /* Assert size of 9-byte upper boundary */
  value = -(1LL << 62);
  ck_assert_uint_eq(9, pb_varint_size_sint64(&value));

  /* Assert size of maximum negative value */
  value = INT64_MIN;
  ck_assert_uint_eq(10, pb_varint_size_sint64(&value));
} END_TEST

/*
 * Retrieve the packed size of a variable-sized integer of given type.
 */
START_TEST(test_size) {
  uint32_t value = 1;
  ck_assert_uint_eq(1, pb_varint_size(PB_TYPE_UINT32, &value));
} END_TEST

/* ------------------------------------------------------------------------- */

/*
 * Pack a signed 32-bit variable-sized integer.
 */
START_TEST(test_pack_int32) {
  uint8_t data[10];

  /* Pack value into buffer */
  int32_t value = -1000000000;
  ck_assert_uint_eq(10, pb_varint_pack_int32(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; int64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_int32(&value); b++) {
    check |= (int64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_int_eq(value, check);
} END_TEST

/*
 * Pack the minimum signed 32-bit variable-sized integer.
 */
START_TEST(test_pack_int32_min) {
  uint8_t data[10];

  /* Pack value into buffer */
  int32_t value = INT32_MIN;
  ck_assert_uint_eq(10, pb_varint_pack_int32(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; int64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_int32(&value); b++) {
    check |= (int64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_int_eq(value, check);
} END_TEST

/*
 * Pack the maximum signed 32-bit variable-sized integer.
 */
START_TEST(test_pack_int32_max) {
  uint8_t data[10];

  /* Pack value into buffer */
  int32_t value = INT32_MAX;
  ck_assert_uint_eq(5, pb_varint_pack_int32(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; int64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_int32(&value); b++) {
    check |= (int64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_int_eq(value, check);
} END_TEST

/*
 * Pack a signed 64-bit variable-sized integer.
 */
START_TEST(test_pack_int64) {
  uint8_t data[10];

  /* Pack value into buffer */
  int64_t value = -1000000000000000000LL;
  ck_assert_uint_eq(10, pb_varint_pack_int64(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; int64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint64(&value); b++) {
    check |= (int64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_int_eq(value, check);
} END_TEST

/*
 * Pack the minimum signed 64-bit variable-sized integer.
 */
START_TEST(test_pack_int64_min) {
  uint8_t data[10];

  /* Pack value into buffer */
  int64_t value = INT64_MIN;
  ck_assert_uint_eq(10, pb_varint_pack_int64(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; int64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_int64(&value); b++) {
    check |= (int64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_int_eq(value, check);
} END_TEST

/*
 * Pack the maximum signed 64-bit variable-sized integer.
 */
START_TEST(test_pack_int64_max) {
  uint8_t data[10];

  /* Pack value into buffer */
  int64_t value = INT64_MAX;
  ck_assert_uint_eq(9, pb_varint_pack_int64(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; int64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_int64(&value); b++) {
    check |= (int64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_int_eq(value, check);
} END_TEST


/*
 * Pack an unsigned 8-bit variable-sized integer.
 */
START_TEST(test_pack_uint8) {
  uint8_t data[2];

  /* Pack value into buffer */
  uint8_t value = 127;
  ck_assert_uint_eq(1, pb_varint_pack_uint8(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint8_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint8(&value); b++) {
    check |= (uint8_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 8 && (byte & 0x80));
  }
  ck_assert_uint_eq(value, check);
} END_TEST

/*
 * Pack the minimum unsigned 8-bit variable-sized integer.
 */
START_TEST(test_pack_uint8_min) {
  uint8_t data[2];

  /* Pack value into buffer */
  uint8_t value = 0;
  ck_assert_uint_eq(1, pb_varint_pack_uint8(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint8_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint8(&value); b++) {
    check |= (uint8_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 8 && (byte & 0x80));
  }
  ck_assert_uint_eq(value, check);
} END_TEST

/*
 * Pack the maximum unsigned 8-bit variable-sized integer.
 */
START_TEST(test_pack_uint8_max) {
  uint8_t data[2];

  /* Pack value into buffer */
  uint8_t value = UINT8_MAX;
  ck_assert_uint_eq(2, pb_varint_pack_uint8(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint8_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint8(&value); b++) {
    check |= (uint8_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 8 && (byte & 0x80));
  }
  ck_assert_uint_eq(value, check);
} END_TEST

/*
 * Pack an unsigned 32-bit variable-sized integer.
 */
START_TEST(test_pack_uint32) {
  uint8_t data[5];

  /* Pack value into buffer */
  uint32_t value = 1000000000U;
  ck_assert_uint_eq(5, pb_varint_pack_uint32(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint32_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint32(&value); b++) {
    check |= (uint32_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 31 && (byte & 0x80));
  }
  ck_assert_uint_eq(value, check);
} END_TEST

/*
 * Pack the minimum unsigned 32-bit variable-sized integer.
 */
START_TEST(test_pack_uint32_min) {
  uint8_t data[5];

  /* Pack value into buffer */
  uint32_t value = 0;
  ck_assert_uint_eq(1, pb_varint_pack_uint32(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint32_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint32(&value); b++) {
    check |= (uint32_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 31 && (byte & 0x80));
  }
  ck_assert_uint_eq(value, check);
} END_TEST

/*
 * Pack the maximum unsigned 32-bit variable-sized integer.
 */
START_TEST(test_pack_uint32_max) {
  uint8_t data[5];

  /* Pack value into buffer */
  uint32_t value = UINT32_MAX;
  ck_assert_uint_eq(5, pb_varint_pack_uint32(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint32_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint32(&value); b++) {
    check |= (uint32_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 31 && (byte & 0x80));
  }
  ck_assert_uint_eq(value, check);
} END_TEST

/*
 * Pack an unsigned 64-bit variable-sized integer.
 */
START_TEST(test_pack_uint64) {
  uint8_t data[10];

  /* Pack value into buffer */
  uint64_t value = 1000000000000000000ULL;
  ck_assert_uint_eq(9, pb_varint_pack_uint64(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint64(&value); b++) {
    check |= (uint64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_uint_eq(value, check);
} END_TEST

/*
 * Pack the minimum unsigned 64-bit variable-sized integer.
 */
START_TEST(test_pack_uint64_min) {
  uint8_t data[10];

  /* Pack value into buffer */
  uint64_t value = 0;
  ck_assert_uint_eq(1, pb_varint_pack_uint64(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint64(&value); b++) {
    check |= (uint64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_uint_eq(value, check);
} END_TEST

/*
 * Pack the maximum unsigned 64-bit variable-sized integer.
 */
START_TEST(test_pack_uint64_max) {
  uint8_t data[10];

  /* Pack value into buffer */
  uint64_t value = UINT64_MAX;
  ck_assert_uint_eq(10, pb_varint_pack_uint64(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_uint64(&value); b++) {
    check |= (uint64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_uint_eq(value, check);
} END_TEST

/*
 * Pack a signed 32-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_pack_sint32) {
  uint8_t data[5];

  /* Pack value into buffer */
  int32_t value = -1000000000;
  ck_assert_uint_eq(5, pb_varint_pack_sint32(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint32_t check = 0;
  for (size_t b = 0; b < pb_varint_size_sint32(&value); b++) {
    check |= (uint32_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 31 && (byte & 0x80));
  }
  ck_assert_int_eq(value, (int32_t)((check >> 1) ^ -(check & 1)));

  /* Encode negative to positive */
  for (int32_t v = -128; v < 128; v++) {
    pb_varint_pack_sint32(data, &v);

    /* Assert buffer contents */
    uint8_t byte, offset = 0; uint32_t check = 0;
    for (size_t b = 0; b < pb_varint_size_sint32(&v); b++) {
      check |= (uint32_t)((byte = data[b]) & 0x7F) << offset;
      fail_if((offset += 7) > 31 && (byte & 0x80));
    }
    ck_assert_int_eq(v, (int32_t)((check >> 1) ^ -(check & 1)));
  }
} END_TEST

/*
 * Pack the minimum signed 32-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_pack_sint32_min) {
  uint8_t data[5];

  /* Pack value into buffer */
  int32_t value = INT32_MIN;
  ck_assert_uint_eq(5, pb_varint_pack_sint32(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint32_t check = 0;
  for (size_t b = 0; b < pb_varint_size_sint32(&value); b++) {
    check |= (uint32_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 31 && (byte & 0x80));
  }
  ck_assert_int_eq(value, (int32_t)((check >> 1) ^ -(check & 1)));
} END_TEST

/*
 * Pack the maximum signed 32-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_pack_sint32_max) {
  uint8_t data[5];

  /* Pack value into buffer */
  int32_t value = INT32_MAX;
  ck_assert_uint_eq(5, pb_varint_pack_sint32(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint32_t check = 0;
  for (size_t b = 0; b < pb_varint_size_sint32(&value); b++) {
    check |= (uint32_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 31 && (byte & 0x80));
  }
  ck_assert_int_eq(value, (int32_t)((check >> 1) ^ -(check & 1)));
} END_TEST

/*
 * Pack a signed 64-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_pack_sint64) {
  uint8_t data[10];

  /* Pack value into buffer */
  int64_t value = -1000000000000000000LL;
  ck_assert_uint_eq(9, pb_varint_pack_sint64(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_sint64(&value); b++) {
    check |= (uint64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_int_eq(value, (int64_t)((check >> 1) ^ -(check & 1)));

  /* Encode negative to positive */
  for (int64_t v = -128; v < 128; v++) {
    pb_varint_pack_sint64(data, &v);

    /* Assert buffer contents */
    uint8_t byte, offset = 0; uint64_t check = 0;
    for (size_t b = 0; b < pb_varint_size_sint64(&v); b++) {
      check |= (uint64_t)((byte = data[b]) & 0x7F) << offset;
      fail_if((offset += 7) > 31 && (byte & 0x80));
    }
    ck_assert_int_eq(v, (int64_t)((check >> 1) ^ -(check & 1)));
  }
} END_TEST

/*
 * Pack the minimum signed 64-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_pack_sint64_min) {
  uint8_t data[10];

  /* Pack value into buffer */
  int64_t value = INT64_MIN;
  ck_assert_uint_eq(10, pb_varint_pack_sint64(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_sint64(&value); b++) {
    check |= (uint64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_int_eq(value, (int64_t)((check >> 1) ^ -(check & 1)));
} END_TEST

/*
 * Pack the maximum signed 64-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_pack_sint64_max) {
  uint8_t data[10];

  /* Pack value into buffer */
  int64_t value = INT64_MAX;
  ck_assert_uint_eq(10, pb_varint_pack_sint64(data, &value));

  /* Assert buffer contents */
  uint8_t byte, offset = 0; uint64_t check = 0;
  for (size_t b = 0; b < pb_varint_size_sint64(&value); b++) {
    check |= (uint64_t)((byte = data[b]) & 0x7F) << offset;
    fail_if((offset += 7) > 63 && (byte & 0x80));
  }
  ck_assert_int_eq(value, (int64_t)((check >> 1) ^ -(check & 1)));
} END_TEST

/*
 * Pack a variable-sized integer of given type.
 */
START_TEST(test_pack) {
  uint8_t data[5]; uint32_t value = 1;
  ck_assert_uint_eq(1, pb_varint_pack(PB_TYPE_UINT32, data, &value));
} END_TEST

/* ------------------------------------------------------------------------- */

/*
 * Scan a buffer for a valid variable-sized integer.
 */
START_TEST(test_scan) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 127 };
  const size_t  size   = 10;

  /* Assert that buffer contains a valid variabled-sized integer */
  fail_unless(pb_varint_scan(data, size));
} END_TEST

/*
 * Scan a buffer for an invalid variable-sized integer.
 */
START_TEST(test_scan_invalid) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };
  const size_t  size   = 10;

  /* Assert that buffer doesn't contain a valid variabled-sized integer */
  fail_if(pb_varint_scan(data, size));
} END_TEST

/* ------------------------------------------------------------------------- */

/*
 * Unpack a signed 32-bit variable-sized integer.
 */
START_TEST(test_unpack_int32) {
  const uint8_t data[] = { 128, 236, 148, 163, 252, 255, 255, 255, 255, 1 };

  /* Unpack value from buffer */
  int32_t check = 0;
  ck_assert_uint_eq(10, pb_varint_unpack_int32(data, 10, &check));
  ck_assert_int_eq(-1000000000, check);
} END_TEST

/*
 * Unpack the minimum signed 32-bit variable-sized integer.
 */
START_TEST(test_unpack_int32_min) {
  const uint8_t data[] = { 128, 128, 128, 128, 248, 255, 255, 255, 255, 1 };

  /* Unpack value from buffer */
  int32_t check = 0;
  ck_assert_uint_eq(10, pb_varint_unpack_int32(data, 10, &check));
  ck_assert_int_eq(INT32_MIN, check);
} END_TEST

/*
 * Unpack the maximum signed 32-bit variable-sized integer.
 */
START_TEST(test_unpack_int32_max) {
  const uint8_t data[] = { 255, 255, 255, 255, 7 };

  /* Unpack value from buffer */
  int32_t check = 0;
  ck_assert_uint_eq(5, pb_varint_unpack_int32(data, 5, &check));
  ck_assert_int_eq(INT32_MAX, check);
} END_TEST

/*
 * Unpack an invalid signed 32-bit variable-sized integer.
 */
START_TEST(test_unpack_int32_invalid) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 0 };

  /* Unpack value from buffer */
  int32_t check = 0;
  ck_assert_uint_eq(0, pb_varint_unpack_int32(data, 6, &check));
} END_TEST

/*
 * Unpack a signed 64-bit variable-sized integer.
 */
START_TEST(test_unpack_int64) {
  const uint8_t data[] = { 128, 128, 240, 196, 197, 169, 210, 143, 242, 1 };

  /* Unpack value from buffer */
  int64_t check = 0;
  ck_assert_uint_eq(10, pb_varint_unpack_int64(data, 10, &check));
  ck_assert_int_eq(-1000000000000000000LL, check);
} END_TEST

/*
 * Unpack the minimum signed 64-bit variable-sized integer.
 */
START_TEST(test_unpack_int64_min) {
  const uint8_t data[] = { 128, 128, 128, 128, 128, 128, 128, 128, 128, 1 };

  /* Unpack value from buffer */
  int64_t check = 0;
  ck_assert_uint_eq(10, pb_varint_unpack_int64(data, 10, &check));
  ck_assert_int_eq(INT64_MIN, check);
} END_TEST

/*
 * Unpack the maximum signed 64-bit variable-sized integer.
 */
START_TEST(test_unpack_int64_max) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 127 };

  /* Unpack value from buffer */
  int64_t check = 0;
  ck_assert_uint_eq(9, pb_varint_unpack_int64(data, 9, &check));
  ck_assert_int_eq(INT64_MAX, check);
} END_TEST

/*
 * Unpack an invalid signed 64-bit variable-sized integer.
 */
START_TEST(test_unpack_int64_invalid) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };

  /* Unpack value from buffer */
  int64_t check = 0;
  ck_assert_uint_eq(0, pb_varint_unpack_int64(data, 10, &check));
} END_TEST

/*
 * Unpack an unsigned 8-bit variable-sized integer.
 */
START_TEST(test_unpack_uint8) {
  const uint8_t data[] = { 128, 1 };

  /* Unpack value from buffer */
  uint8_t check = 0;
  ck_assert_uint_eq(2, pb_varint_unpack_uint8(data, 2, &check));
  ck_assert_uint_eq(128, check);
} END_TEST

/*
 * Unpack the minimum unsigned 8-bit variable-sized integer.
 */
START_TEST(test_unpack_uint8_min) {
  const uint8_t data[] = { 0 };

  /* Unpack value from buffer */
  uint8_t check = 0;
  ck_assert_uint_eq(1, pb_varint_unpack_uint8(data, 1, &check));
  ck_assert_uint_eq(0, check);
} END_TEST

/*
 * Unpack the maximum unsigned 8-bit variable-sized integer.
 */
START_TEST(test_unpack_uint8_max) {
  const uint8_t data[] = { 255, 1 };

  /* Unpack value from buffer */
  uint8_t check = 0;
  ck_assert_uint_eq(2, pb_varint_unpack_uint8(data, 2, &check));
  ck_assert_uint_eq(255, check);
} END_TEST

/*
 * Unpack an invalid unsigned 8-bit variable-sized integer.
 */
START_TEST(test_unpack_uint8_invalid) {
  const uint8_t data[] = { 255, 255 };

  /* Unpack value from buffer */
  uint8_t check = 0;
  ck_assert_uint_eq(0, pb_varint_unpack_uint8(data, 2, &check));
} END_TEST

/*
 * Unpack an unsigned 32-bit variable-sized integer.
 */
START_TEST(test_unpack_uint32) {
  const uint8_t data[] = { 128, 148, 235, 220, 3 };

  /* Unpack value from buffer */
  uint32_t check = 0;
  ck_assert_uint_eq(5, pb_varint_unpack_uint32(data, 5, &check));
  ck_assert_uint_eq(1000000000U, check);
} END_TEST

/*
 * Unpack the minimum unsigned 32-bit variable-sized integer.
 */
START_TEST(test_unpack_uint32_min) {
  const uint8_t data[] = { 0 };

  /* Unpack value from buffer */
  uint32_t check = 0;
  ck_assert_uint_eq(1, pb_varint_unpack_uint32(data, 1, &check));
  ck_assert_uint_eq(0, check);
} END_TEST

/*
 * Unpack the maximum unsigned 32-bit variable-sized integer.
 */
START_TEST(test_unpack_uint32_max) {
  const uint8_t data[] = { 255, 255, 255, 255, 15 };

  /* Unpack value from buffer */
  uint32_t check = 0;
  ck_assert_uint_eq(5, pb_varint_unpack_uint32(data, 5, &check));
  ck_assert_uint_eq(UINT32_MAX, check);
} END_TEST

/*
 * Unpack an invalid unsigned 32-bit variable-sized integer.
 */
START_TEST(test_unpack_uint32_invalid) {
  const uint8_t data[] = { 255, 255, 255, 255, 255 };

  /* Unpack value from buffer */
  uint32_t check = 0;
  ck_assert_uint_eq(0, pb_varint_unpack_uint32(data, 5, &check));
} END_TEST

/*
 * Unpack an unsigned 64-bit variable-sized integer.
 */
START_TEST(test_unpack_uint64) {
  const uint8_t data[] = { 128, 128, 144, 187, 186, 214, 173, 240, 13 };

  /* Unpack value from buffer */
  uint64_t check = 0;
  ck_assert_uint_eq(9, pb_varint_unpack_uint64(data, 10, &check));
  ck_assert_uint_eq(1000000000000000000ULL, check);
} END_TEST

/*
 * Unpack the minimum unsigned 64-bit variable-sized integer.
 */
START_TEST(test_unpack_uint64_min) {
  const uint8_t data[] = { 0 };

  /* Unpack value from buffer */
  uint64_t check = 0;
  ck_assert_uint_eq(1, pb_varint_unpack_uint64(data, 1, &check));
  ck_assert_uint_eq(0, check);
} END_TEST

/*
 * Unpack the maximum unsigned 64-bit variable-sized integer.
 */
START_TEST(test_unpack_uint64_max) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 127 };

  /* Unpack value from buffer */
  uint64_t check = 0;
  ck_assert_uint_eq(9, pb_varint_unpack_uint64(data, 9, &check));
  ck_assert_uint_eq(INT64_MAX, check);
} END_TEST

/*
 * Unpack an invalid unsigned 64-bit variable-sized integer.
 */
START_TEST(test_unpack_uint64_invalid) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };

  /* Unpack value from buffer */
  uint64_t check = 0;
  ck_assert_uint_eq(0, pb_varint_unpack_uint64(data, 10, &check));
} END_TEST

/*
 * Unpack a signed 32-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_unpack_sint32) {
  const uint8_t data[] = { 255, 167, 214, 185, 7 };

  /* Unpack value from buffer */
  int32_t check = 0;
  ck_assert_uint_eq(5, pb_varint_unpack_sint32(data, 5, &check));
  ck_assert_int_eq(-1000000000, check);
} END_TEST

/*
 * Unpack the minimum signed 32-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_unpack_sint32_min) {
  const uint8_t data[] = { 255, 255, 255, 255, 15 };

  /* Unpack value from buffer */
  int32_t check = 0;
  ck_assert_uint_eq(5, pb_varint_unpack_sint32(data, 5, &check));
  ck_assert_int_eq(INT32_MIN, check);
} END_TEST

/*
 * Unpack the maximum signed 32-bit variable-sized integer in zig-zag-encoding.
 */
START_TEST(test_unpack_sint32_max) {
  const uint8_t data[] = { 254, 255, 255, 255, 15 };

  /* Unpack value from buffer */
  int32_t check = 0;
  ck_assert_uint_eq(5, pb_varint_unpack_sint32(data, 5, &check));
  ck_assert_int_eq(INT32_MAX, check);
} END_TEST

/*
 * Unpack an invalid signed 32-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_unpack_sint32_invalid) {
  const uint8_t data[] = { 255, 255, 255, 255, 255 };

  /* Unpack value from buffer */
  int32_t check = 0;
  ck_assert_uint_eq(0, pb_varint_unpack_sint32(data, 5, &check));
} END_TEST

/*
 * Unpack a signed 64-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_unpack_sint64) {
  const uint8_t data[] = { 255, 255, 159, 246, 244, 172, 219, 224, 27 };

  /* Unpack value from buffer */
  int64_t check = 0;
  ck_assert_uint_eq(9, pb_varint_unpack_sint64(data, 10, &check));
  ck_assert_int_eq(-1000000000000000000LL, check);
} END_TEST

/*
 * Unpack the minimum signed 64-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_unpack_sint64_min) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 1 };

  /* Unpack value from buffer */
  int64_t check = 0;
  ck_assert_uint_eq(10, pb_varint_unpack_sint64(data, 10, &check));
  ck_assert_int_eq(INT64_MIN, check);
} END_TEST

/*
 * Unpack the maximum signed 64-bit variable-sized integer in zig-zag-encoding.
 */
START_TEST(test_unpack_sint64_max) {
  const uint8_t data[] = { 254, 255, 255, 255, 255, 255, 255, 255, 255, 1 };

  /* Unpack value from buffer */
  int64_t check = 0;
  ck_assert_uint_eq(10, pb_varint_unpack_sint64(data, 10, &check));
  ck_assert_int_eq(INT64_MAX, check);
} END_TEST

/*
 * Unpack an invalid signed 64-bit variable-sized integer in zig-zag encoding.
 */
START_TEST(test_unpack_sint64_invalid) {
  const uint8_t data[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };

  /* Unpack value from buffer */
  int64_t check = 0;
  ck_assert_uint_eq(0, pb_varint_unpack_sint64(data, 10, &check));
} END_TEST

/*
 * Unpack a variable-sized integer of given type.
 */
START_TEST(test_unpack) {
  const uint8_t data[] = { 1 }; uint32_t value = 0;
  ck_assert_uint_eq(1, pb_varint_unpack(PB_TYPE_UINT32, data, 1, &value));
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
  void *suite = suite_create("protobluff/core/varint"),
       *tcase = NULL;

  /* Add tests to test case "size" */
  tcase = tcase_create("size");
  tcase_add_test(tcase, test_size_int32);
  tcase_add_test(tcase, test_size_int64);
  tcase_add_test(tcase, test_size_uint8);
  tcase_add_test(tcase, test_size_uint32);
  tcase_add_test(tcase, test_size_uint64);
  tcase_add_test(tcase, test_size_sint32);
  tcase_add_test(tcase, test_size_sint64);
  tcase_add_test(tcase, test_size);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "pack" */
  tcase = tcase_create("pack");
  tcase_add_test(tcase, test_pack_int32);
  tcase_add_test(tcase, test_pack_int32_min);
  tcase_add_test(tcase, test_pack_int32_max);
  tcase_add_test(tcase, test_pack_int64);
  tcase_add_test(tcase, test_pack_int64_min);
  tcase_add_test(tcase, test_pack_int64_max);
  tcase_add_test(tcase, test_pack_uint8);
  tcase_add_test(tcase, test_pack_uint8_min);
  tcase_add_test(tcase, test_pack_uint8_max);
  tcase_add_test(tcase, test_pack_uint32);
  tcase_add_test(tcase, test_pack_uint32_min);
  tcase_add_test(tcase, test_pack_uint32_max);
  tcase_add_test(tcase, test_pack_uint64);
  tcase_add_test(tcase, test_pack_uint64_min);
  tcase_add_test(tcase, test_pack_uint64_max);
  tcase_add_test(tcase, test_pack_sint32);
  tcase_add_test(tcase, test_pack_sint32_min);
  tcase_add_test(tcase, test_pack_sint32_max);
  tcase_add_test(tcase, test_pack_sint64);
  tcase_add_test(tcase, test_pack_sint64_min);
  tcase_add_test(tcase, test_pack_sint64_max);
  tcase_add_test(tcase, test_pack);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "scan" */
  tcase = tcase_create("scan");
  tcase_add_test(tcase, test_scan);
  tcase_add_test(tcase, test_scan_invalid);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "unpack" */
  tcase = tcase_create("unpack");
  tcase_add_test(tcase, test_unpack_int32);
  tcase_add_test(tcase, test_unpack_int32_min);
  tcase_add_test(tcase, test_unpack_int32_max);
  tcase_add_test(tcase, test_unpack_int32_invalid);
  tcase_add_test(tcase, test_unpack_int64);
  tcase_add_test(tcase, test_unpack_int64_min);
  tcase_add_test(tcase, test_unpack_int64_max);
  tcase_add_test(tcase, test_unpack_int64_invalid);
  tcase_add_test(tcase, test_unpack_uint8);
  tcase_add_test(tcase, test_unpack_uint8_min);
  tcase_add_test(tcase, test_unpack_uint8_max);
  tcase_add_test(tcase, test_unpack_uint8_invalid);
  tcase_add_test(tcase, test_unpack_uint32);
  tcase_add_test(tcase, test_unpack_uint32_min);
  tcase_add_test(tcase, test_unpack_uint32_max);
  tcase_add_test(tcase, test_unpack_uint32_invalid);
  tcase_add_test(tcase, test_unpack_uint64);
  tcase_add_test(tcase, test_unpack_uint64_min);
  tcase_add_test(tcase, test_unpack_uint64_max);
  tcase_add_test(tcase, test_unpack_uint64_invalid);
  tcase_add_test(tcase, test_unpack_sint32);
  tcase_add_test(tcase, test_unpack_sint32_min);
  tcase_add_test(tcase, test_unpack_sint32_max);
  tcase_add_test(tcase, test_unpack_sint32_invalid);
  tcase_add_test(tcase, test_unpack_sint64);
  tcase_add_test(tcase, test_unpack_sint64_min);
  tcase_add_test(tcase, test_unpack_sint64_max);
  tcase_add_test(tcase, test_unpack_sint64_invalid);
  tcase_add_test(tcase, test_unpack);
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
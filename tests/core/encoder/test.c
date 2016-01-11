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

#include "core/allocator.h"
#include "core/buffer.h"
#include "core/common.h"
#include "core/descriptor.h"
#include "core/encoder.h"

/* ----------------------------------------------------------------------------
 * System-default allocator callback overrides
 * ------------------------------------------------------------------------- */

/*!
 * Allocator with failing allocation.
 *
 * \param[in,out] data Internal allocator data
 * \param[in]     size Bytes to be allocated
 * \return             Memory block
 */
static void *
allocator_allocate_fail(void *data, size_t size) {
  assert(!data && size);
  return NULL;
}

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

/* Enum descriptor */
static pb_enum_descriptor_t
enum_descriptor = { {
  (const pb_enum_value_descriptor_t []){
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
    {  6, "F06", FLOAT,   OPTIONAL },
    {  7, "F07", DOUBLE,  OPTIONAL },
    {  8, "F08", STRING,  OPTIONAL },
    {  9, "F09", BYTES,   OPTIONAL },
    { 10, "F10", ENUM,    OPTIONAL, &enum_descriptor },
    { 11, "F11", MESSAGE, OPTIONAL, &descriptor },
    { 12, "F12", MESSAGE, REPEATED, &descriptor }
  }, 12 } };

/* Descriptor with packed fields */
static pb_descriptor_t
descriptor_packed = { {
  (const pb_field_descriptor_t []){
    {  1, "F01", UINT64,  REPEATED, NULL, NULL, PACKED },
    {  2, "F02", ENUM,    REPEATED, &enum_descriptor, NULL, PACKED },
    {  3, "F03", FLOAT,   REPEATED, NULL, NULL, PACKED },
    {  4, "F04", DOUBLE,  REPEATED, NULL, NULL, PACKED }
  }, 4 } };

/* ----------------------------------------------------------------------------
 * Tests
 * ------------------------------------------------------------------------- */

/*
 * Create an encoder.
 */
START_TEST(test_create) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Assert encoder validity and error */
  fail_unless(pb_encoder_valid(&encoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_encoder_error(&encoder));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Create an invalid encoder.
 */
START_TEST(test_create_invalid) {
  pb_encoder_t encoder = pb_encoder_create_invalid();
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Assert encoder validity and error */
  fail_if(pb_encoder_valid(&encoder));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_encoder_error(&encoder));

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Create an encoder for which allocation fails.
 */
START_TEST(test_create_invalid_allocate) {
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_allocate_fail,
      .resize   = allocator_default.proc.resize,
      .free     = allocator_default.proc.free
    }
  };

  /* Create encoder */
  pb_encoder_t encoder =
    pb_encoder_create_with_allocator(&allocator, &descriptor);

  /* Assert encoder validity and error */
  fail_unless(pb_encoder_valid(&encoder));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_encoder_error(&encoder));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a value.
 */
START_TEST(test_encode) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  uint32_t value1 = 1000000000;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 1, &value1, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(6, pb_buffer_size(buffer));

  /* Encode another value */
  pb_string_t value2 = pb_string_init_from_chars("SOME DATA");
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 8, &value2, 1));

  /* Assert buffer validity and error again */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size again */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(17, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode values in packed encoding.
 */
START_TEST(test_encode_packed) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor_packed);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode values */
  pb_enum_t values[] = { 0, 1, 2 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 2, values, 3));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(5, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode values in packed encoding with an invalid encoder.
 */
START_TEST(test_encode_packed_invalid) {
  pb_encoder_t encoder = pb_encoder_create_invalid();
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode values */
  uint64_t values[] = { 10, 100, 1000, 10000, 100000, 1000000 };
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_encoder_encode(&encoder, 1, &values, 6));

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode values with an encoder for which reallocation fails.
 */
START_TEST(test_encode_packed_invalid_resize) {
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_resize_fail,
      .free     = allocator_default.proc.free
    }
  };

  /* Create encoder */
  pb_encoder_t encoder =
    pb_encoder_create_with_allocator(&allocator, &descriptor_packed);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode values */
  uint64_t values[] = { 10, 100, 1000, 10000, 100000, 1000000 };
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_encoder_encode(&encoder, 1, &values, 6));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a variable-sized integer.
 */
START_TEST(test_encode_varint) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  pb_enum_t value = 0;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 10, &value, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(2, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode variable-sized integers in packed encoding.
 */
START_TEST(test_encode_varint_packed) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor_packed);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode values */
  uint64_t values[] = { 10, 100, 1000, 10000, 100000, 1000000 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 1, values, 6));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(14, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode variable-sized integers in packed encoding in merged mode.
 */
START_TEST(test_encode_varint_packed_merged) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor_packed);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode values */
  uint64_t values[] = { 10, 100, 1000, 10000, 100000, 1000000 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 1, values, 6));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 1, values, 6));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(28, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a variable-sized integer with an invalid encoder.
 */
START_TEST(test_encode_varint_invalid) {
  pb_encoder_t encoder = pb_encoder_create_invalid();
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  pb_enum_t value = 0;
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_encoder_encode(&encoder, 10, &value, 1));

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a variable-sized integer with failing reallocation.
 */
START_TEST(test_encode_varint_invalid_resize) {
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_resize_fail,
      .free     = allocator_default.proc.free
    }
  };

  /* Create encoder */
  pb_encoder_t encoder =
    pb_encoder_create_with_allocator(&allocator, &descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  pb_enum_t value = 0;
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_encoder_encode(&encoder, 10, &value, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a fixed-sized 64-bit value.
 */
START_TEST(test_encode_64bit) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  double value = 0.00000001;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 7, &value, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(9, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode fixed-sized 64-bit values in packed encoding.
 */
START_TEST(test_encode_64bit_packed) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor_packed);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode values */
  double values[] = { 0.00000001, 0.000001, 0.0001, 0.01, 1.0, 100.0 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 4, &values, 6));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(50, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode fixed-sized 64-bit values in packed encoding in merged mode.
 */
START_TEST(test_encode_64bit_packed_merged) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor_packed);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode values */
  double values[] = { 0.00000001, 0.000001, 0.0001, 0.01, 1.0, 100.0 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 4, &values, 6));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 4, &values, 6));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(100, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a fixed-sized 64-bit value with an invalid encoder.
 */
START_TEST(test_encode_64bit_invalid) {
  pb_encoder_t encoder = pb_encoder_create_invalid();
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  double value = 0.00000001;
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_encoder_encode(&encoder, 7, &value, 1));

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a fixed-sized 64-bit value with failing reallocation.
 */
START_TEST(test_encode_64bit_invalid_resize) {
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_resize_fail,
      .free     = allocator_default.proc.free
    }
  };

  /* Create encoder */
  pb_encoder_t encoder =
    pb_encoder_create_with_allocator(&allocator, &descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  double value = 0.00000001;
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_encoder_encode(&encoder, 7, &value, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a length-prefixed string.
 */
START_TEST(test_encode_length) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  pb_string_t value = pb_string_init_from_chars("SOME DATA");
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 8, &value, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(11, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a length-prefixed message.
 */
START_TEST(test_encode_length_message) {
  pb_encoder_t encoder1 = pb_encoder_create(&descriptor);
  pb_encoder_t encoder2 = pb_encoder_create(&descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder1);

  /* Encode a value and a message */
  double value = 0.00000001;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder2, 7, &value, 1));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder1, 11, &encoder2, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(11, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder2);
  pb_encoder_destroy(&encoder1);
} END_TEST

/*
 * Encode a length-prefixed string with an invalid encoder.
 */
START_TEST(test_encode_length_invalid) {
  pb_encoder_t encoder = pb_encoder_create_invalid();
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  pb_string_t value = pb_string_init_from_chars("SOME DATA");
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_encoder_encode(&encoder, 8, &value, 1));

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a length-prefixed string with failing reallocation.
 */
START_TEST(test_encode_length_invalid_resize) {
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_resize_fail,
      .free     = allocator_default.proc.free
    }
  };

  /* Create encoder */
  pb_encoder_t encoder =
    pb_encoder_create_with_allocator(&allocator, &descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  pb_string_t value = pb_string_init_from_chars("SOME DATA");
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_encoder_encode(&encoder, 8, &value, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a fixed-sized 32-bit value.
 */
START_TEST(test_encode_32bit) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  float value = 0.0001;
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 6, &value, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(5, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode fixed-sized 32-bit values in packed encoding.
 */
START_TEST(test_encode_32bit_packed) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor_packed);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode values */
  float values[] = { 0.0001, 0.01, 1.0, 100.0, 10000.0, 1000000.0 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 3, &values, 6));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(26, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode fixed-sized 32-bit values in packed encoding in merged mode.
 */
START_TEST(test_encode_32bit_packed_merged) {
  pb_encoder_t encoder = pb_encoder_create(&descriptor_packed);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode values */
  float values[] = { 0.0001, 0.01, 1.0, 100.0, 10000.0, 1000000.0 };
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 3, &values, 6));
  ck_assert_uint_eq(PB_ERROR_NONE,
    pb_encoder_encode(&encoder, 3, &values, 6));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_if(pb_buffer_empty(buffer));
  ck_assert_uint_eq(52, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a fixed-sized 32-bit value with an invalid encoder.
 */
START_TEST(test_encode_32bit_invalid) {
  pb_encoder_t encoder = pb_encoder_create_invalid();
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  float value = 0.0001;
  ck_assert_uint_eq(PB_ERROR_INVALID,
    pb_encoder_encode(&encoder, 6, &value, 1));

  /* Assert buffer validity and error */
  fail_if(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_ALLOC, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
} END_TEST

/*
 * Encode a fixed-sized 32-bit value with failing reallocation.
 */
START_TEST(test_encode_32bit_invalid_resize) {
  pb_allocator_t allocator = {
    .proc = {
      .allocate = allocator_default.proc.allocate,
      .resize   = allocator_resize_fail,
      .free     = allocator_default.proc.free
    }
  };

  /* Create encoder */
  pb_encoder_t encoder =
    pb_encoder_create_with_allocator(&allocator, &descriptor);
  const pb_buffer_t *buffer = pb_encoder_buffer(&encoder);

  /* Encode a value */
  float value = 0.0001;
  ck_assert_uint_eq(PB_ERROR_ALLOC,
    pb_encoder_encode(&encoder, 6, &value, 1));

  /* Assert buffer validity and error */
  fail_unless(pb_buffer_valid(buffer));
  ck_assert_uint_eq(PB_ERROR_NONE, pb_buffer_error(buffer));

  /* Assert buffer size */
  fail_unless(pb_buffer_empty(buffer));
  ck_assert_uint_eq(0, pb_buffer_size(buffer));

  /* Free all allocated memory */
  pb_encoder_destroy(&encoder);
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
  void *suite = suite_create("protobluff/core/encoder"),
       *tcase = NULL;

  /* Add tests to test case "create" */
  tcase = tcase_create("create");
  tcase_add_test(tcase, test_create);
  tcase_add_test(tcase, test_create_invalid);
  tcase_add_test(tcase, test_create_invalid_allocate);
  suite_add_tcase(suite, tcase);

  /* Add tests to test case "encode" */
  tcase = tcase_create("encode");
  tcase_add_test(tcase, test_encode);
  tcase_add_test(tcase, test_encode_packed);
  tcase_add_test(tcase, test_encode_packed_invalid);
  tcase_add_test(tcase, test_encode_packed_invalid_resize);
  tcase_add_test(tcase, test_encode_varint);
  tcase_add_test(tcase, test_encode_varint_packed);
  tcase_add_test(tcase, test_encode_varint_packed_merged);
  tcase_add_test(tcase, test_encode_varint_invalid);
  tcase_add_test(tcase, test_encode_varint_invalid_resize);
  tcase_add_test(tcase, test_encode_64bit);
  tcase_add_test(tcase, test_encode_64bit_packed);
  tcase_add_test(tcase, test_encode_64bit_packed_merged);
  tcase_add_test(tcase, test_encode_64bit_invalid);
  tcase_add_test(tcase, test_encode_64bit_invalid_resize);
  tcase_add_test(tcase, test_encode_length);
  tcase_add_test(tcase, test_encode_length_message);
  tcase_add_test(tcase, test_encode_length_invalid);
  tcase_add_test(tcase, test_encode_length_invalid_resize);
  tcase_add_test(tcase, test_encode_32bit);
  tcase_add_test(tcase, test_encode_32bit_packed);
  tcase_add_test(tcase, test_encode_32bit_packed_merged);
  tcase_add_test(tcase, test_encode_32bit_invalid);
  tcase_add_test(tcase, test_encode_32bit_invalid_resize);
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
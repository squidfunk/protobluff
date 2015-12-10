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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "core/allocator.h"
#include "core/buffer.h"
#include "core/common.h"
#include "core/descriptor.h"
#include "core/encoder.h"
#include "core/varint.h"

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef pb_error_t
(*pb_encoder_encode_f)(
  pb_buffer_t *buffer,                 /*!< Buffer */
  const pb_field_descriptor_t
    *descriptor,                       /*!< Field descriptor */
  const void *value);                  /*!< Pointer holding value */

/* ----------------------------------------------------------------------------
 * Encoder callbacks
 * ------------------------------------------------------------------------- */

/*!
 * Encode a variable-sized integer.
 *
 * \param[in,out] buffer     Buffer
 * \param[in]     descriptor Field descriptor
 * \param[in]     value      Pointer holding value
 * \return                   Error code
 */
static pb_error_t
encode_varint(
    pb_buffer_t *buffer, const pb_field_descriptor_t *descriptor,
    const void *value) {
  assert(buffer && descriptor && value);
  assert(pb_buffer_valid(buffer));
  pb_type_t type = pb_field_descriptor_type(descriptor);

#ifndef NDEBUG

  /* Assert valid value for enum field */
  if (type == PB_TYPE_ENUM)
    assert(pb_enum_descriptor_value_by_number(
      pb_field_descriptor_reference(descriptor),
        *(const pb_enum_t *)value));

#endif /* NDEBUG */

  /* Encode variable-sized integer according to type */
  uint8_t *data = pb_buffer_grow(
    buffer, pb_varint_size(type, value));
  return likely_(data && pb_varint_pack(type, data, value))
    ? PB_ERROR_NONE
    : PB_ERROR_ALLOC;
}

/*!
 * Encode a fixed-sized 64-bit value.
 *
 * \param[in,out] buffer     Buffer
 * \param[in]     descriptor Field descriptor
 * \param[in]     value      Pointer holding value
 * \return                   Error code
 */
static pb_error_t
encode_64bit(
    pb_buffer_t *buffer, const pb_field_descriptor_t *descriptor,
    const void *value) {
  assert(buffer && descriptor && value);
  assert(pb_buffer_valid(buffer));
  uint8_t *data = pb_buffer_grow(buffer, 8);
  return likely_(data && memcpy(data, value, 8))
    ? PB_ERROR_NONE
    : PB_ERROR_ALLOC;
}

/*!
 * Encode a length-prefixed value.
 *
 * \warning The lines excluded from code coverage cannot be triggered within
 * the tests, as they are masked through pb_encoder_encode().
 *
 * \param[in,out] buffer     Buffer
 * \param[in]     descriptor Field descriptor
 * \param[in]     value      Pointer holding value
 * \return                   Error code
 */
static pb_error_t
encode_length(
    pb_buffer_t *buffer, const pb_field_descriptor_t *descriptor,
    const void *value) {
  assert(buffer && descriptor && value);
  assert(pb_buffer_valid(buffer));

  /* Extract data and size according to type */
  const uint8_t *string; uint32_t length;
  if (pb_field_descriptor_type(descriptor) == PB_TYPE_MESSAGE) {
    string = pb_buffer_data(pb_encoder_buffer(value));
    length = pb_buffer_size(pb_encoder_buffer(value));
  } else {
    string = pb_string_data(value);
    length = pb_string_size(value);
  }

  /* Encode length prefix */
  uint8_t *data = pb_buffer_grow(buffer,
    pb_varint_size_uint32(&length) + length);
  if (data) {
    data += pb_varint_pack_uint32(data, &length);

    /* Encode message or string */
    memcpy(data, string, length);
    return PB_ERROR_NONE;
  }
  return PB_ERROR_ALLOC;                                   /* LCOV_EXCL_LINE */
}

/*!
 * Encode a fixed-sized 32-bit value.
 *
 * \param[in,out] buffer     Buffer
 * \param[in]     descriptor Field descriptor
 * \param[in]     value      Pointer holding value
 * \return                   Error code
 */
static pb_error_t
encode_32bit(
    pb_buffer_t *buffer, const pb_field_descriptor_t *descriptor,
    const void *value) {
  assert(buffer && descriptor && value);
  assert(pb_buffer_valid(buffer));
  uint8_t *data = pb_buffer_grow(buffer, 4);
  return likely_(data && memcpy(data, value, 4))
    ? PB_ERROR_NONE
    : PB_ERROR_ALLOC;
}

/* ----------------------------------------------------------------------------
 * Jump tables
 * ------------------------------------------------------------------------- */

/*! Jump table: wiretype ==> encode method */
static const pb_encoder_encode_f
encode_jump[] = {
  [PB_WIRETYPE_VARINT] = encode_varint,
  [PB_WIRETYPE_64BIT]  = encode_64bit,
  [PB_WIRETYPE_LENGTH] = encode_length,
  [PB_WIRETYPE_32BIT]  = encode_32bit
};

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create an encoder.
 *
 * \param[in] descriptor Descriptor
 * \return               Encoder
 */
extern pb_encoder_t
pb_encoder_create(const pb_descriptor_t *descriptor) {
  return pb_encoder_create_with_allocator(&allocator_default, descriptor);
}

/*!
 * Create an encoder using a custom allocator.
 *
 * \warning An encoder does not take ownership of the provided allocator, so
 * the caller must ensure that the allocator is not freed during operations.
 *
 * \param[in,out] allocator  Allocator
 * \param[in]     descriptor Descriptor
 * \return                   Encoder
 */
extern pb_encoder_t
pb_encoder_create_with_allocator(
    pb_allocator_t *allocator, const pb_descriptor_t *descriptor) {
  assert(allocator && descriptor);
  pb_encoder_t encoder = {
    .descriptor = descriptor,
    .buffer     = pb_buffer_create_empty_with_allocator(allocator)
  };
  return encoder;
}

/*!
 * Destroy an encoder.
 *
 * \param[in,out] encoder Encoder
 */
extern void
pb_encoder_destroy(pb_encoder_t *encoder) {
  assert(encoder);
  if (pb_encoder_valid(encoder))
    pb_buffer_destroy(&(encoder->buffer));
}

/*!
 * Encode a value.
 *
 * The encoder has a few limitations due to its simple and slim design that
 * may impact message validity:
 *
 * -# The encoder encodes a field regardless of its label (optional, required
 *    or repeated). The Protocol Buffers standard demands that only the last
 *    occurrence of an optional or required field is interpreted.
 *
 * -# The encoder does not check that all required fields are set. This can
 *    be done afterwards by creating a validator with pb_validator_create()
 *    and checking it with pb_validator_check().
 *
 * -# The encoder does not enforce a natual ordering of fields (according to
 *    their tag numbers) when encoding a message. If order matters, the caller
 *    is responsible to ensure and enforce it.
 *
 * -# The encoder does not automatically encode default values for optional
 *    fields. Some third-party Protocol Buffers libraries implement this, but
 *    it is not correct in respect to the specification and therefore omitted.
 *
 * \param[in,out] encoder Encoder
 * \param[in]     tag     Tag
 * \param[in]     value   Pointer holding value
 * \return                Error code
 */
extern pb_error_t
pb_encoder_encode(pb_encoder_t *encoder, pb_tag_t tag, const void *value) {
  assert(encoder && tag && value);
  if (unlikely_(!pb_encoder_valid(encoder)) || encoder == value)
    return PB_ERROR_INVALID;

  /* Assert descriptor */
  const pb_field_descriptor_t *descriptor =
    pb_descriptor_field_by_tag(encoder->descriptor, tag);
  assert(descriptor);

  /* Pack wiretype into tag */
  pb_wiretype_t wiretype = pb_field_descriptor_wiretype(descriptor);
  tag = (tag << 3) | wiretype;

  /* Encode tag and value */
  uint8_t *data = pb_buffer_grow(
    &(encoder->buffer), pb_varint_size_uint32(&tag));
  if (data) {
    pb_varint_pack_uint32(data, &tag);

    /* Encode value */
    assert(encode_jump[wiretype]);
    return encode_jump[wiretype](&(encoder->buffer), descriptor, value);
  }
  return PB_ERROR_ALLOC;
}
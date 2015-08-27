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

#include <alloca.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lib/binary.h"
#include "lib/binary/buffer.h"
#include "lib/binary/stream.h"
#include "lib/common.h"
#include "lib/cursor.h"
#include "lib/enum/descriptor.h"
#include "lib/field.h"
#include "lib/field/descriptor.h"
#include "lib/message.h"
#include "lib/message/descriptor.h"
#include "lib/part.h"

/* ----------------------------------------------------------------------------
 * Jump tables
 * ------------------------------------------------------------------------- */

/* Jump table: field type ==> stream read method */
static const pb_binary_stream_read_f read_jump[] = {
  [PB_TYPE_INT32]    = pb_binary_stream_read_varint32,
  [PB_TYPE_INT64]    = pb_binary_stream_read_varint64,
  [PB_TYPE_UINT32]   = pb_binary_stream_read_varint32,
  [PB_TYPE_UINT64]   = pb_binary_stream_read_varint64,
  [PB_TYPE_SINT32]   = pb_binary_stream_read_svarint32,
  [PB_TYPE_SINT64]   = pb_binary_stream_read_svarint64,
  [PB_TYPE_FIXED32]  = pb_binary_stream_read_fixed32,
  [PB_TYPE_FIXED64]  = pb_binary_stream_read_fixed64,
  [PB_TYPE_SFIXED32] = pb_binary_stream_read_fixed32,
  [PB_TYPE_SFIXED64] = pb_binary_stream_read_fixed64,
  [PB_TYPE_BOOL]     = pb_binary_stream_read_varint8,
  [PB_TYPE_ENUM]     = pb_binary_stream_read_varint32,
  [PB_TYPE_FLOAT]    = pb_binary_stream_read_fixed32,
  [PB_TYPE_DOUBLE]   = pb_binary_stream_read_fixed64,
  [PB_TYPE_STRING]   = pb_binary_stream_read_length,
  [PB_TYPE_BYTES]    = pb_binary_stream_read_length,
  [PB_TYPE_MESSAGE]  = NULL
};

/* Jump table: field type ==> buffer write method */
static const pb_binary_buffer_write_f write_jump[] = {
  [PB_TYPE_INT32]    = pb_binary_buffer_write_varint32,
  [PB_TYPE_INT64]    = pb_binary_buffer_write_varint64,
  [PB_TYPE_UINT32]   = pb_binary_buffer_write_varint32,
  [PB_TYPE_UINT64]   = pb_binary_buffer_write_varint64,
  [PB_TYPE_SINT32]   = pb_binary_buffer_write_svarint32,
  [PB_TYPE_SINT64]   = pb_binary_buffer_write_svarint64,
  [PB_TYPE_FIXED32]  = NULL,
  [PB_TYPE_FIXED64]  = NULL,
  [PB_TYPE_SFIXED32] = NULL,
  [PB_TYPE_SFIXED64] = NULL,
  [PB_TYPE_BOOL]     = pb_binary_buffer_write_varint8,
  [PB_TYPE_ENUM]     = pb_binary_buffer_write_varint32,
  [PB_TYPE_FLOAT]    = NULL,
  [PB_TYPE_DOUBLE]   = NULL,
  [PB_TYPE_STRING]   = NULL,
  [PB_TYPE_BYTES]    = NULL,
  [PB_TYPE_MESSAGE]  = NULL
};

/* ----------------------------------------------------------------------------
 * Mappings
 * ------------------------------------------------------------------------- */

/* Mapping: wiretype ==> default value */
static const void *default_map[7] = {
  [PB_WIRETYPE_VARINT] = (const uint8_t  []){ 0 },
  [PB_WIRETYPE_64BIT]  = (const uint64_t []){ 0 },
  [PB_WIRETYPE_32BIT]  = (const uint32_t []){ 0 }
};

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a field within a message for a specific tag.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Field
 */
extern pb_field_t
pb_field_create(pb_message_t *message, pb_tag_t tag) {
  pb_field_t field = pb_field_create_without_default(message, tag);
  if (pb_field_valid(&field) && pb_field_empty(&field)) {
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_field_by_tag(pb_message_descriptor(message), tag);

    /* Write explicit or implicit default value to field */
    const void *value = pb_field_descriptor_default(descriptor)
      ? pb_field_descriptor_default(descriptor)
      : default_map[pb_field_descriptor_wiretype(descriptor)];
    if (value && pb_field_put(&field, value)) {
      pb_field_destroy(&field);                            /* LCOV_EXCL_LINE */
      return pb_field_create_invalid();                    /* LCOV_EXCL_LINE */
    }
  }
  return field;
};

/*!
 * Create a field within a message for a specific tag without setting defaults.
 *
 * \warning This function returns a field in an inconsistent state, with a tag
 * set but no value, in a way which is not recognized by the parser. Immediate
 * writes should always be performed through pb_message_put(), which calls
 * this method internally.
 *
 * \param[in,out] message Message
 * \param[in]     tag     Tag
 * \return                Field
 */
extern pb_field_t
pb_field_create_without_default(pb_message_t *message, pb_tag_t tag) {
  assert(message && tag);
  if (pb_message_valid(message)) {
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_field_by_tag(pb_message_descriptor(message), tag);
    assert(descriptor &&
      pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE);
    pb_field_t field = {
      .descriptor = descriptor,
      .part       = pb_part_create(message, tag)
    };
    return field;
  }
  return pb_field_create_invalid();
};

/*!
 * Create a field within a nested message for a branch of tags.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \return                Field
 */
extern pb_field_t
pb_field_create_nested(
    pb_message_t *message, const pb_tag_t tags[], size_t size) {
  assert(message && tags && size > 1);
  pb_message_t submessage =
    pb_message_create_nested(message, tags, --size);
  pb_field_t field = pb_field_create(&submessage, tags[size]);
  pb_message_destroy(&submessage);
  return field;
}

/*!
 * Create a field at the current position of a cursor.
 *
 * \param[in,out] cursor Cursor
 * \return               Field
 */
extern pb_field_t
pb_field_create_from_cursor(pb_cursor_t *cursor) {
  assert(cursor);
  if (pb_cursor_valid(cursor)) {
    const pb_message_t *message = pb_cursor_message(cursor);
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_field_by_tag(
        pb_message_descriptor(message), pb_cursor_tag(cursor));
    if (descriptor)
      if (pb_field_descriptor_type(descriptor) != PB_TYPE_MESSAGE) {
        pb_field_t field = {
          .descriptor = descriptor,
          .part       = pb_part_create_from_cursor(cursor)
        };
        return field;
      }
  }
  return pb_field_create_invalid();
}

/*!
 * Destroy a field.
 *
 * \param[in,out] field Field
 */
extern void
pb_field_destroy(pb_field_t *field) {
  assert(field);
  pb_part_destroy(&(field->part));
}

/*!
 * Compare the value of a field with the given value.
 *
 * 32-bit and 64-bit fixed-size fields can be compared by raw pointer, all
 * other types must be read explicitly.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] field Field
 * \param[in]     value Pointer receiving value
 * \return              Test result
 */
extern int
pb_field_match(pb_field_t *field, const void *value) {
  assert(field && value);
  if (unlikely_(!pb_field_valid(field) || pb_field_align(field)))
    return 0;

  /* Directly compare fixed-size fields */
  size_t size = pb_field_descriptor_type_size(field->descriptor);
  if (pb_field_descriptor_wiretype(field->descriptor) & PB_WIRETYPE_64BIT)
    return !memcmp(value, pb_field_raw(field), size);

  /* Allocate enough temporary space for field-agnostic compare */
  void *temp = alloca(size);
  if (unlikely_(pb_field_get(field, temp)))
    return 0;

  /* Compare field value according to type */
  return pb_field_descriptor_wiretype(field->descriptor) != PB_WIRETYPE_LENGTH
    ? !memcmp(value, temp, size)
    : !memcmp(((const pb_string_t *)value)->data,
        ((const pb_string_t *)temp)->data,
        ((const pb_string_t *)temp)->size);
}

/*!
 * Read the value from a field.
 *
 * This method has to ensure the alignment of the part, since it directly
 * creates the binary stream from the underlying part.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] field Field
 * \param[out]    value Pointer receiving value
 * \return              Error code
 */
extern pb_error_t
pb_field_get(pb_field_t *field, void *value) {
  assert(field && value);
  if (unlikely_(!pb_field_valid(field) || pb_field_align(field)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Create a stream to read the field's value */
  pb_binary_stream_t stream =
    pb_binary_stream_create_from_part(&(field->part));
  pb_type_t type = pb_field_descriptor_type(field->descriptor);
  if (!read_jump[type] || (error = read_jump[type](&stream, value)))
    error = error ? error : PB_ERROR_WIRETYPE;
  pb_binary_stream_destroy(&stream);
  return error;
}

/*!
 * Write a value to a field.
 *
 * \warning The caller has to ensure that the space pointed to by the value
 * pointer is appropriately sized for the type of field.
 *
 * \param[in,out] field Field
 * \param[in]     value Pointer holding value
 * \return              Error code
 */
extern pb_error_t
pb_field_put(pb_field_t *field, const void *value) {
  assert(field && value);
  if (unlikely_(!pb_field_valid(field)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Write new value according to wiretype */

  pb_type_t type = pb_field_descriptor_type(field->descriptor);
  switch (pb_field_descriptor_wiretype(field->descriptor)) {

    /* Write a variable-sized integer through a buffer */
    case PB_WIRETYPE_VARINT: {

      #ifndef NDEBUG

          /* Assert valid value for enum field */
          if (type == PB_TYPE_ENUM)
            assert(pb_enum_descriptor_value_by_number(
              pb_field_descriptor_reference(field->descriptor),
                *(const pb_enum_t *)value));

      #endif /* NDEBUG */

      /* Create buffer and write variable-sized integer */
      pb_binary_buffer_t buffer = pb_binary_buffer_create();
      if (write_jump[type] && !(error = write_jump[type](&buffer, value)))
        error = pb_part_write(&(field->part),
          pb_binary_buffer_data(&buffer),
          pb_binary_buffer_size(&buffer));
      pb_binary_buffer_destroy(&buffer);
      break;
    }

    /* Write a fixed-sized 64-bit value */
    case PB_WIRETYPE_64BIT:
      error = pb_part_write(&(field->part), value, 8);
      break;

    /* Write a length-prefixed value */
    case PB_WIRETYPE_LENGTH:
      error = pb_part_write(&(field->part),
        ((const pb_string_t *)value)->data,
        ((const pb_string_t *)value)->size);
      break;

    /* Write a fixed-sized 32-bit value */
    case PB_WIRETYPE_32BIT:
      error = pb_part_write(&(field->part), value, 4);
      break;
  }
  return error;
}

/*!
 * Clear a field entirely.
 *
 * \warning Clearing a field will also invalidate it, as the underlying part
 * cannot restore the associated header (tag and length prefix).
 *
 * \param[in,out] field Field
 * \return              Error code
 */
extern pb_error_t
pb_field_clear(pb_field_t *field) {
  assert(field);
  return pb_part_clear(&(field->part));
}

/*!
 * Retrieve a pointer to the raw data of a field.
 *
 * \warning Operating on the raw data of a binary is insanely dangerous and
 * only recommended for 32- and 64-bit fixed-size fields on zero-copy binaries.
 * If the underlying binary is altered, the pointer silently loses its
 * validity. All following operations will quietly corrupt the binary.
 *
 * \param[in,out] field Field
 * \return              Raw data
 */
extern void *
pb_field_raw(pb_field_t *field) {
  assert(field);
  return pb_field_valid(field) && !pb_field_align(field)
    ? pb_binary_data_from(pb_field_binary(field), pb_field_start(field))
    : NULL;
}

/* LCOV_EXCL_START >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> */

/*!
 * Dump a field.
 *
 * Though this is just a wrapper for pb_part_dump(), it is implemented as a
 * proper function so it can be easily called from a debugger.
 *
 * \warning Don't use this in production, only for debugging.
 *
 * \param[in] field Field
 */
extern void
pb_field_dump(const pb_field_t *field) {
  assert(field);
  pb_part_dump(&(field->part));
}

/* LCOV_EXCL_STOP <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< */
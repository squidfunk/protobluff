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

#include "core/common.h"
#include "core/descriptor.h"

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Return the smaller of two values.
 *
 * \param[in] x Value to compare
 * \param[in] y Value to compare
 * \return      Smaller value
 */
#define min(x, y) \
  ((x) < (y) ? (x) : (y))

/* ----------------------------------------------------------------------------
 * Mappings
 * ------------------------------------------------------------------------- */

/*! Mapping: type ==> native size */
const size_t
pb_field_descriptor_type_size_map[] = {
  [PB_TYPE_INT32]    = sizeof(int32_t),
  [PB_TYPE_INT64]    = sizeof(int64_t),
  [PB_TYPE_UINT32]   = sizeof(uint32_t),
  [PB_TYPE_UINT64]   = sizeof(uint64_t),
  [PB_TYPE_SINT32]   = sizeof(int32_t),
  [PB_TYPE_SINT64]   = sizeof(int64_t),
  [PB_TYPE_FIXED32]  = sizeof(uint32_t),
  [PB_TYPE_FIXED64]  = sizeof(uint64_t),
  [PB_TYPE_SFIXED32] = sizeof(int32_t),
  [PB_TYPE_SFIXED64] = sizeof(int64_t),
  [PB_TYPE_BOOL]     = sizeof(uint8_t),
  [PB_TYPE_ENUM]     = sizeof(pb_enum_t),
  [PB_TYPE_FLOAT]    = sizeof(float),
  [PB_TYPE_DOUBLE]   = sizeof(double),
  [PB_TYPE_STRING]   = sizeof(pb_string_t),
  [PB_TYPE_BYTES]    = sizeof(pb_string_t),
  [PB_TYPE_MESSAGE]  = sizeof(pb_string_t)
};

/*! Mapping: type ==> wiretype */
const pb_wiretype_t
pb_field_descriptor_wiretype_map[] = {
  [PB_TYPE_INT32]    = PB_WIRETYPE_VARINT,
  [PB_TYPE_INT64]    = PB_WIRETYPE_VARINT,
  [PB_TYPE_UINT32]   = PB_WIRETYPE_VARINT,
  [PB_TYPE_UINT64]   = PB_WIRETYPE_VARINT,
  [PB_TYPE_SINT32]   = PB_WIRETYPE_VARINT,
  [PB_TYPE_SINT64]   = PB_WIRETYPE_VARINT,
  [PB_TYPE_FIXED32]  = PB_WIRETYPE_32BIT,
  [PB_TYPE_FIXED64]  = PB_WIRETYPE_64BIT,
  [PB_TYPE_SFIXED32] = PB_WIRETYPE_32BIT,
  [PB_TYPE_SFIXED64] = PB_WIRETYPE_64BIT,
  [PB_TYPE_BOOL]     = PB_WIRETYPE_VARINT,
  [PB_TYPE_ENUM]     = PB_WIRETYPE_VARINT,
  [PB_TYPE_FLOAT]    = PB_WIRETYPE_32BIT,
  [PB_TYPE_DOUBLE]   = PB_WIRETYPE_64BIT,
  [PB_TYPE_STRING]   = PB_WIRETYPE_LENGTH,
  [PB_TYPE_BYTES]    = PB_WIRETYPE_LENGTH,
  [PB_TYPE_MESSAGE]  = PB_WIRETYPE_LENGTH
};

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the field descriptor for a given tag from a descriptor.
 *
 * We can leverage the fact that the fields are always in ascending order, as
 * the generator will generate it in this exact way, so using the tag number as
 * an array index, there are two scenarios:
 *
 * -# The tag number is smaller than the number of fields, so we can limit the
 *    fields we have to check to those left to the tag number array index. The
 *    respective field may then be at or left to the exact position.
 *
 * -# The tag number found at the array index is larger than the tag number
 *    we're looking for. In this case, we can abort the search.
 *
 * \warning The fields actually need to be in ascending order, so you better
 * ensure that or be pleasantly surprised by undefined behaviour.
 *
 * \param[in] descriptor Descriptor
 * \param[in] tag        Tag
 * \return               Field descriptor
 */
extern const pb_field_descriptor_t *
pb_descriptor_field_by_tag(
    const pb_descriptor_t *descriptor, pb_tag_t tag) {
  assert(descriptor && tag);
  for (size_t f = min(tag, descriptor->field.size); f > 0; ) {
    if (pb_field_descriptor_tag(
        &(descriptor->field.data[--f])) == tag) {
      return &(descriptor->field.data[f]);
    } else if (pb_field_descriptor_tag(
        &(descriptor->field.data[f])) < tag) {
      break;
    }
  }
  return pb_descriptor_extension(descriptor) ?
    pb_descriptor_field_by_tag(
      pb_descriptor_extension(descriptor), tag) : NULL;
}

/*!
 * Register an extension for the given descriptor.
 *
 * Extensions are implemented as a linked list of descriptors - if multiple
 * extensions are registered for a descriptor, they are chained together.
 * Before registering an extension, it is checked that the extension is not
 * already registered.
 *
 * \param[in,out] descriptor Descriptor
 * \param[in,out] extension  Descriptor extension
 */
extern void
pb_descriptor_extend(
    pb_descriptor_t *descriptor, pb_descriptor_t *extension) {
  assert(descriptor && extension);
  while (descriptor->extension) {
    if (descriptor->extension == extension)
      return;
    descriptor = descriptor->extension;
  }
  descriptor->extension = extension;
}

/* ------------------------------------------------------------------------- */

/*!
 * Retrieve the value for a given number from an enum descriptor.
 *
 * \warning To find the matching enum descriptor value quickly if any, the same
 * approach like in pb_descriptor_field_by_tag() is used.
 *
 * \param[in] descriptor Enum descriptor
 * \param[in] number     Number
 * \return               Enum descriptor value
 */
extern const struct pb_enum_descriptor_value_t *
pb_enum_descriptor_value_by_number(
    const pb_enum_descriptor_t *descriptor, pb_enum_t number) {
  assert(descriptor);
  if (descriptor->value.size) {
    size_t v = min(number, descriptor->value.size - 1);
    do {
      if (pb_enum_descriptor_value_number(
          &(descriptor->value.data[v])) == number) {
        return &(descriptor->value.data[v]);
      } else if (pb_enum_descriptor_value_number(
          &(descriptor->value.data[v])) < number) {
        break;
      }
    } while (v--);
  }
  return NULL;
}
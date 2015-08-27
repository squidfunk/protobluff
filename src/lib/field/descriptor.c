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

#include "lib/common.h"
#include "lib/field/descriptor.h"

/* ----------------------------------------------------------------------------
 * Mappings
 * ------------------------------------------------------------------------- */

/* Mapping: field type ==> size */
static const size_t size_map[] = {
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
  [PB_TYPE_MESSAGE]  = 0
};

/* Mapping: field type ==> wiretype */
static const pb_wiretype_t wiretype_map[] = {
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
 * Retrieve the native type size for a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Type size
 */
extern size_t
pb_field_descriptor_type_size(const pb_field_descriptor_t *descriptor) {
  assert(descriptor);
  return size_map[descriptor->type];
}

/*!
 * Retrieve the necessary wiretype for a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Wiretype
 */
extern pb_wiretype_t
pb_field_descriptor_wiretype(const pb_field_descriptor_t *descriptor) {
  assert(descriptor);
  return wiretype_map[descriptor->type];
}
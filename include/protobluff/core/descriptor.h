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

#ifndef PB_INCLUDE_CORE_DESCRIPTOR_H
#define PB_INCLUDE_CORE_DESCRIPTOR_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <protobluff/core/common.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_field_descriptor_t {
  const pb_tag_t tag;                  /*!< Tag */
  const char *const name;              /*!< Name */
  const pb_type_t type;                /*!< Type */
  const pb_label_t label;              /*!< Label */
  const void *const refer;             /*!< Referenced descriptor */
  const void *const value;             /*!< Default */
  const uint8_t flags;                 /*!< Flags */
} pb_field_descriptor_t;

/* ------------------------------------------------------------------------- */

typedef struct pb_descriptor_t {
  struct {
    const struct pb_field_descriptor_t
      *const data;                     /*!< Field descriptors */
    const size_t size;                 /*!< Field descriptor count */
  } field;
  struct pb_descriptor_t
    *extension;                        /*!< Descriptor extension */
} pb_descriptor_t;

/* ------------------------------------------------------------------------- */

typedef struct pb_enum_descriptor_value_t {
  const pb_enum_t number;              /*!< Number */
  const char *const name;              /*!< Name */
} pb_enum_descriptor_value_t;

typedef struct pb_enum_descriptor_t {
  struct {
    const pb_enum_descriptor_value_t
      *const data;                     /*!< Enum descriptor values */
    const size_t size;                 /*!< Enum descriptor value count */
  } value;
} pb_enum_descriptor_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT const pb_field_descriptor_t *
pb_descriptor_field_by_tag(
  const pb_descriptor_t *descriptor,   /* Descriptor */
  pb_tag_t tag);                       /* Tag */

PB_EXPORT void
pb_descriptor_extend(
  pb_descriptor_t *descriptor,         /* Descriptor */
  pb_descriptor_t *extension);         /* Descriptor extension */

/* ------------------------------------------------------------------------- */

PB_EXPORT const pb_enum_descriptor_value_t *
pb_enum_descriptor_value_by_number(
  const pb_enum_descriptor_t
    *descriptor,                       /* Enum descriptor */
  pb_enum_t number);                   /* Number */

/* ----------------------------------------------------------------------------
 * Mappings
 * ------------------------------------------------------------------------- */

/*! Mapping: type ==> native size */
extern const size_t
pb_field_descriptor_type_size_map[];

/*! Mapping: type ==> wiretype */
extern const pb_wiretype_t
pb_field_descriptor_wiretype_map[];

/* ----------------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------------- */

#define PB_FLAG_PACKED 1               /*!< Flag: packed field */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the tag of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Tag
 */
PB_INLINE pb_tag_t
pb_field_descriptor_tag(const pb_field_descriptor_t *descriptor) {
  assert(descriptor);
  return descriptor->tag;
}

/*!
 * Retrieve the name of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Name
 */
PB_INLINE const char *
pb_field_descriptor_name(const pb_field_descriptor_t *descriptor) {
  assert(descriptor);
  return descriptor->name;
}

/*!
 * Retrieve the type of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Type
 */
PB_INLINE pb_type_t
pb_field_descriptor_type(const pb_field_descriptor_t *descriptor) {
  assert(descriptor);
  return descriptor->type;
}

/*!
 * Retrieve the label of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Label
 */
PB_INLINE pb_label_t
pb_field_descriptor_label(const pb_field_descriptor_t *descriptor) {
  assert(descriptor);
  return descriptor->label;
}

/*!
 * Retrieve the referenced descriptor of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Referenced descriptor
 */
PB_INLINE const void *
pb_field_descriptor_reference(const pb_field_descriptor_t *descriptor) {
  assert(descriptor && (
    descriptor->type == PB_TYPE_MESSAGE ||
    descriptor->type == PB_TYPE_ENUM));
  return descriptor->refer;
}

/*!
 * Retrieve the default value of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Default value
 */
PB_INLINE const void *
pb_field_descriptor_default(const pb_field_descriptor_t *descriptor) {
  assert(descriptor &&
    descriptor->type != PB_TYPE_MESSAGE);
  return descriptor->value;
}

/*!
 * Test whether a field descriptor specifies the packed option.
 *
 * \param[in] descriptor Field descriptor
 * \return               Test result
 */
PB_INLINE int
pb_field_descriptor_packed(const pb_field_descriptor_t *descriptor) {
  assert(descriptor);
  return descriptor->flags & PB_FLAG_PACKED;
}

/*!
 * Retrieve the native type size of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Type size
 */
PB_INLINE size_t
pb_field_descriptor_type_size(const pb_field_descriptor_t *descriptor) {
  assert(descriptor);
  return pb_field_descriptor_type_size_map[descriptor->type];
}

/*!
 * Retrieve the wiretype of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Wiretype
 */
PB_INLINE pb_wiretype_t
pb_field_descriptor_wiretype(const pb_field_descriptor_t *descriptor) {
  assert(descriptor);
  return pb_field_descriptor_wiretype_map[descriptor->type];
}

/* ------------------------------------------------------------------------- */

/*!
 * Retrieve the size of a descriptor.
 *
 * \param[in] descriptor Descriptor
 * \return               Descriptor size
 */
PB_INLINE size_t
pb_descriptor_size(const pb_descriptor_t *descriptor) {
  assert(descriptor);
  return descriptor->field.size;
}

/*!
 * Test whether a descriptor is empty.
 *
 * \param[in] descriptor Descriptor
 * \return               Test result
 */
PB_INLINE int
pb_descriptor_empty(const pb_descriptor_t *descriptor) {
  assert(descriptor);
  return !pb_descriptor_size(descriptor);
}

/*!
 * Retrieve the (first) extension of a descriptor.
 *
 * \param[in] descriptor Descriptor
 * \return               Descriptor extension
 */
PB_INLINE pb_descriptor_t *
pb_descriptor_extension(const pb_descriptor_t *descriptor) {
  assert(descriptor);
  return descriptor->extension;
}

/* ------------------------------------------------------------------------- */

/*!
 * Retrieve the value of an enum descriptor value.
 *
 * \param[in] value Enum descriptor value
 * \return          Value
 */
PB_INLINE pb_enum_t
pb_enum_descriptor_value_number(const pb_enum_descriptor_value_t *value) {
  assert(value);
  return value->number;
}

/*!
 * Retrieve the name of an enum descriptor value.
 *
 * \param[in] value Enum descriptor value
 * \return          Name
 */
PB_INLINE const char *
pb_enum_descriptor_value_name(const pb_enum_descriptor_value_t *value) {
  assert(value);
  return value->name;
}

/* ------------------------------------------------------------------------- */

/*!
 * Retrieve the size of an enum descriptor.
 *
 * \param[in] descriptor Enum descriptor
 * \return               Enum descriptor size
 */
PB_INLINE size_t
pb_enum_descriptor_size(const pb_enum_descriptor_t *descriptor) {
  assert(descriptor);
  return descriptor->value.size;
}

/*!
 * Test whether an enum descriptor is empty.
 *
 * \param[in] descriptor Enum descriptor
 * \return               Test result
 */
PB_INLINE int
pb_enum_descriptor_empty(const pb_enum_descriptor_t *descriptor) {
  assert(descriptor);
  return !pb_enum_descriptor_size(descriptor);
}

#endif /* PB_INCLUDE_CORE_DESCRIPTOR_H */
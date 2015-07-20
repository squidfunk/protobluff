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

#ifndef PB_INCLUDE_FIELD_DESCRIPTOR_H
#define PB_INCLUDE_FIELD_DESCRIPTOR_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <protobluff/common.h>

/* ----------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------- */

struct pb_message_descriptor_t;

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
  const uint8_t flags;                 /*!< Flags (reserved) */
} pb_field_descriptor_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT size_t
pb_field_descriptor_type_size(
  const pb_field_descriptor_t
    *descriptor);                      /* Field descriptor */

PB_EXPORT pb_wiretype_t
pb_field_descriptor_wiretype(
  const pb_field_descriptor_t
    *descriptor);                      /* Field descriptor */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the tag of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Tag
 */
#define pb_field_descriptor_tag(descriptor) \
  (assert(descriptor), (descriptor)->tag)

/*!
 * Retrieve the name of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Name
 */
#define pb_field_descriptor_name(descriptor) \
  (assert(descriptor), (descriptor)->name)

/*!
 * Retrieve the type of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Type
 */
#define pb_field_descriptor_type(descriptor) \
  (assert(descriptor), (descriptor)->type)

/*!
 * Retrieve the label of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Label
 */
#define pb_field_descriptor_label(descriptor) \
  (assert(descriptor), (descriptor)->label)

/*!
 * Retrieve the referenced descriptor of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Referenced descriptor
 */
#define pb_field_descriptor_reference(descriptor) \
  (assert((descriptor) && (descriptor)->type == PB_TYPE_MESSAGE), \
    (descriptor)->refer)

/*!
 * Retrieve the default value of a field descriptor.
 *
 * \param[in] descriptor Field descriptor
 * \return               Default value
 */
#define pb_field_descriptor_default(descriptor) \
  (assert((descriptor) && (descriptor)->type != PB_TYPE_MESSAGE), \
    (descriptor)->value)

#endif /* PB_INCLUDE_FIELD_DESCRIPTOR_H */
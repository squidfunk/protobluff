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

#ifndef PB_INCLUDE_ENUM_DESCRIPTOR_H
#define PB_INCLUDE_ENUM_DESCRIPTOR_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <protobluff/common.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

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

typedef struct pb_enum_descriptor_iter_t {
  const pb_enum_descriptor_t
    *const descriptor;                 /*!< Enum descriptor */
  size_t pos;                          /*!< Current position */
} pb_enum_descriptor_iter_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT const struct pb_enum_descriptor_value_t *
pb_enum_descriptor_value_by_number(
  const pb_enum_descriptor_t
    *descriptor,                       /* Enum descriptor */
  pb_enum_t number);                   /* Number */

PB_EXPORT const struct pb_enum_descriptor_value_t *
pb_enum_descriptor_value_by_name(
  const pb_enum_descriptor_t
    *descriptor,                       /* Enum descriptor */
  const char name[]);                  /* Name */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the value of an enum descriptor value.
 *
 * \param[in] value Enum descriptor value
 * \return          Value
 */
#define pb_enum_descriptor_value_number(value) \
  (assert(value), (value)->number)

/*!
 * Retrieve the name of an enum descriptor value.
 *
 * \param[in] value Enum descriptor value
 * \return          Name
 */
#define pb_enum_descriptor_value_name(value) \
  (assert(value), (value)->name)

/* ------------------------------------------------------------------------- */

/*!
 * Retrieve the size of an enum descriptor.
 *
 * \param[in] descriptor Enum descriptor
 * \return               Enum descriptor size
 */
#define pb_enum_descriptor_size(descriptor) \
  (assert(descriptor), (descriptor)->value.size)

/*!
 * Test whether an enum descriptor is empty.
 *
 * \param[in] descriptor Enum descriptor
 * \return               Test result
 */
#define pb_enum_descriptor_empty(descriptor) \
  (assert(descriptor), !pb_enum_descriptor_size(descriptor))

/* ------------------------------------------------------------------------- */

/*!
 * Create a const-iterator over an enum descriptor.
 *
 * \param[in] descriptor Enum descriptor
 * \return               Enum descriptor iterator
 */
#define pb_enum_descriptor_iter_create(descriptor) \
  (assert(descriptor), \
    (pb_enum_descriptor_iter_t){ (descriptor), SIZE_MAX })

/*!
 * Free all allocated memory.
 *
 * \param[in,out] it Enum descriptor iterator
 */
#define pb_enum_descriptor_iter_destroy(it) \
  (assert(it)) /* Nothing to be done */

/*!
 * Set the position of an enum descriptor iterator to the first item.
 *
 * \param[in,out] it Enum descriptor iterator
 * \return           Test result
 */
#define pb_enum_descriptor_iter_begin(it) \
  (assert(it), !pb_enum_descriptor_empty((it)->descriptor) ? \
    !((it)->pos = 0) : 0)

/*!
 * Set the position of an enum descriptor iterator to the last item.
 *
 * \param[in,out] it Enum descriptor iterator
 * \return           Test result
 */
#define pb_enum_descriptor_iter_end(it) \
  (assert(it), !pb_enum_descriptor_empty((it)->descriptor) ? \
    !!((it)->pos = pb_enum_descriptor_size((it)->descriptor) - 1) : 0)

/*!
 * Decrement the position of an enum descriptor iterator.
 *
 * \param[in,out] it Enum descriptor iterator
 * \return           Test result
 */
#define pb_enum_descriptor_iter_prev(it) \
  (assert((it) && (it)->pos != SIZE_MAX), \
    (!pb_enum_descriptor_empty((it)->descriptor) && (it)->pos > 0) ? \
      (it)->pos-- : 0)

/*!
 * Increment the position of an enum descriptor iterator.
 *
 * \param[in,out] it Enum descriptor iterator
 * \return           Test result
 */
#define pb_enum_descriptor_iter_next(it) \
  (assert((it) && (it)->pos != SIZE_MAX), \
    (!pb_enum_descriptor_empty((it)->descriptor) && \
      (it)->pos < (pb_enum_descriptor_size((it)->descriptor) - 1)) ? \
        ++(it)->pos : 0)

/*!
 * Retrieve the current position of an enum descriptor iterator.
 *
 * \param[in] it Enum descriptor iterator
 * \return       Current position
 */
#define pb_enum_descriptor_iter_pos(it) \
  (assert((it) && (it)->pos != SIZE_MAX), \
    (const size_t)(it)->pos)

/*!
 * Retrieve the enum descriptor value at the current position.
 *
 * \param[in] it Enum descriptor iterator
 * \return       Enum descriptor value
 */
#define pb_enum_descriptor_iter_current(it) \
  (assert((it) && (it)->pos != SIZE_MAX), \
    &((it)->descriptor->value.data[(it)->pos]))

#endif /* PB_INCLUDE_ENUM_DESCRIPTOR_H */
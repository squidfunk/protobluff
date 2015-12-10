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

#ifndef PB_INCLUDE_UTIL_DESCRIPTOR_H
#define PB_INCLUDE_UTIL_DESCRIPTOR_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <protobluff/core/common.h>
#include <protobluff/core/descriptor.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_descriptor_iter_t {
  const pb_descriptor_t
    *const descriptor;                 /*!< Descriptor */
  size_t pos;                          /*!< Current position */
} pb_descriptor_iter_t;

/* ------------------------------------------------------------------------- */

typedef struct pb_enum_descriptor_iter_t {
  const pb_enum_descriptor_t
    *const descriptor;                 /*!< Enum descriptor */
  size_t pos;                          /*!< Current position */
} pb_enum_descriptor_iter_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT const pb_field_descriptor_t *
pb_descriptor_field_by_name(
  const pb_descriptor_t *descriptor,   /* Descriptor */
  const char name[]);                  /* Name */

/* ------------------------------------------------------------------------- */

PB_EXPORT const pb_enum_descriptor_value_t *
pb_enum_descriptor_value_by_name(
  const pb_enum_descriptor_t
    *descriptor,                       /* Enum descriptor */
  const char name[]);                  /* Name */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Create a const-iterator over a descriptor.
 *
 * \param[in] descriptor Descriptor
 * \return               Descriptor iterator
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_descriptor_iter_t
pb_descriptor_iter_create(const pb_descriptor_t *descriptor) {
  assert(descriptor);
  pb_descriptor_iter_t it = {
    .descriptor = descriptor,
    .pos        = SIZE_MAX
  };
  return it;
}

/*!
 * Free all allocated memory.
 *
 * \param[in,out] it Descriptor iterator
 */
PB_INLINE void
pb_descriptor_iter_destroy(pb_descriptor_iter_t *it) {
  assert(it); /* Nothing to be done */
}

/*!
 * Set the position of a descriptor iterator to the first item.
 *
 * \param[in,out] it Descriptor iterator
 * \return           Test result
 */
PB_WARN_UNUSED_RESULT
PB_INLINE int
pb_descriptor_iter_begin(pb_descriptor_iter_t *it) {
  assert(it);
  return !pb_descriptor_empty(it->descriptor)
    ? !(it->pos = 0)
    : 0;
}

/*!
 * Set the position of a descriptor iterator to the last item.
 *
 * \param[in,out] it Descriptor iterator
 * \return           Test result
 */
PB_WARN_UNUSED_RESULT
PB_INLINE int
pb_descriptor_iter_end(pb_descriptor_iter_t *it) {
  assert(it);
  return !pb_descriptor_empty(it->descriptor)
    ? !!(it->pos = pb_descriptor_size(it->descriptor) - 1)
    : 0;
}

/*!
 * Decrement the position of a descriptor iterator.
 *
 * \param[in,out] it Descriptor iterator
 * \return           Test result
 */
PB_WARN_UNUSED_RESULT
PB_INLINE int
pb_descriptor_iter_prev(pb_descriptor_iter_t *it) {
  assert(it && it->pos != SIZE_MAX);
  assert(!pb_descriptor_empty(it->descriptor));
  return it->pos > 0
    ? it->pos--
    : 0;
}

/*!
 * Increment the position of a descriptor iterator.
 *
 * \param[in,out] it Descriptor iterator
 * \return           Test result
 */
PB_WARN_UNUSED_RESULT
PB_INLINE int
pb_descriptor_iter_next(pb_descriptor_iter_t *it) {
  assert(it && it->pos != SIZE_MAX);
  assert(!pb_descriptor_empty(it->descriptor));
  return it->pos < pb_descriptor_size(it->descriptor) - 1
    ? ++it->pos
    : 0;
}

/*!
 * Retrieve the current position of a descriptor iterator.
 *
 * \param[in] it Descriptor iterator
 * \return       Current position
 */
PB_INLINE size_t
pb_descriptor_iter_pos(const pb_descriptor_iter_t *it) {
  assert(it && it->pos != SIZE_MAX);
  return it->pos;
}

/*!
 * Retrieve the field descriptor at the current position.
 *
 * \param[in] it Descriptor iterator
 * \return       Field descriptor
 */
PB_INLINE const pb_field_descriptor_t *
pb_descriptor_iter_current(const pb_descriptor_iter_t *it) {
  assert(it && it->pos != SIZE_MAX);
  return &(it->descriptor->field.data[it->pos]);
}

/* ------------------------------------------------------------------------- */

/*!
 * Create a const-iterator over an enum descriptor.
 *
 * \param[in] descriptor Enum descriptor
 * \return               Enum descriptor iterator
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_enum_descriptor_iter_t
pb_enum_descriptor_iter_create(const pb_enum_descriptor_t *descriptor) {
  assert(descriptor);
  pb_enum_descriptor_iter_t it = {
    .descriptor = descriptor,
    .pos        = SIZE_MAX
  };
  return it;
}

/*!
 * Free all allocated memory.
 *
 * \param[in,out] it Enum descriptor iterator
 */
PB_INLINE void
pb_enum_descriptor_iter_destroy(pb_enum_descriptor_iter_t *it) {
  assert(it); /* Nothing to be done */
}

/*!
 * Set the position of an enum descriptor iterator to the first item.
 *
 * \param[in,out] it Enum descriptor iterator
 * \return           Test result
 */
PB_WARN_UNUSED_RESULT
PB_INLINE int
pb_enum_descriptor_iter_begin(pb_enum_descriptor_iter_t *it) {
  assert(it);
  return !pb_enum_descriptor_empty(it->descriptor)
    ? !(it->pos = 0)
    : 0;
}

/*!
 * Set the position of an enum descriptor iterator to the last item.
 *
 * \param[in,out] it Enum descriptor iterator
 * \return           Test result
 */
PB_WARN_UNUSED_RESULT
PB_INLINE int
pb_enum_descriptor_iter_end(pb_enum_descriptor_iter_t *it) {
  assert(it);
  return !pb_enum_descriptor_empty(it->descriptor)
    ? !!(it->pos = pb_enum_descriptor_size(it->descriptor) - 1)
    : 0;
}

/*!
 * Decrement the position of an enum descriptor iterator.
 *
 * \param[in,out] it Enum descriptor iterator
 * \return           Test result
 */
PB_WARN_UNUSED_RESULT
PB_INLINE int
pb_enum_descriptor_iter_prev(pb_enum_descriptor_iter_t *it) {
  assert(it && it->pos != SIZE_MAX);
  assert(!pb_enum_descriptor_empty(it->descriptor));
  return it->pos > 0
    ? it->pos--
    : 0;
}

/*!
 * Increment the position of an enum descriptor iterator.
 *
 * \param[in,out] it Enum descriptor iterator
 * \return           Test result
 */
PB_WARN_UNUSED_RESULT
PB_INLINE int
pb_enum_descriptor_iter_next(pb_enum_descriptor_iter_t *it) {
  assert(it && it->pos != SIZE_MAX);
  assert(!pb_enum_descriptor_empty(it->descriptor));
  return it->pos < pb_enum_descriptor_size(it->descriptor) - 1
    ? ++it->pos
    : 0;
}

/*!
 * Retrieve the current position of an enum descriptor iterator.
 *
 * \param[in] it Enum descriptor iterator
 * \return       Current position
 */
PB_INLINE size_t
pb_enum_descriptor_iter_pos(const pb_enum_descriptor_iter_t *it) {
  assert(it && it->pos != SIZE_MAX);
  return it->pos;
}

/*!
 * Retrieve the enum descriptor value at the current position.
 *
 * \param[in] it Enum descriptor iterator
 * \return       Enum descriptor value
 */
PB_INLINE const pb_enum_descriptor_value_t *
pb_enum_descriptor_iter_current(const pb_enum_descriptor_iter_t *it) {
  assert(it && it->pos != SIZE_MAX);
  return &(it->descriptor->value.data[it->pos]);
}

#endif /* PB_INCLUDE_UTIL_DESCRIPTOR_H */
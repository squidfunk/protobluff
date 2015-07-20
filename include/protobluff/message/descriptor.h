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

#ifndef PB_INCLUDE_MESSAGE_DESCRIPTOR_H
#define PB_INCLUDE_MESSAGE_DESCRIPTOR_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <protobluff/common.h>

/* ----------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------- */

struct pb_field_descriptor_t;

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_message_descriptor_t {
  struct {
    const struct pb_field_descriptor_t
      *const data;                     /*!< Field descriptors */
    const size_t size;                 /*!< Field descriptor count */
  } field;
} pb_message_descriptor_t;

typedef struct pb_message_descriptor_iter_t {
  const pb_message_descriptor_t
    *const descriptor;                 /*!< Message descriptor */
  size_t pos;                          /*!< Current position */
} pb_message_descriptor_iter_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT const struct pb_field_descriptor_t *
pb_message_descriptor_field_by_tag(
  const pb_message_descriptor_t
    *descriptor,                       /* Message descriptor */
  pb_tag_t tag);                       /* Tag */

PB_EXPORT const struct pb_field_descriptor_t *
pb_message_descriptor_field_by_name(
  const pb_message_descriptor_t
    *descriptor,                       /* Message descriptor */
  const char name[]);                  /* Name */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the size of a message descriptor.
 *
 * \param[in] descriptor Message descriptor
 * \return               Message descriptor size
 */
#define pb_message_descriptor_size(descriptor) \
  (assert(descriptor), (descriptor)->field.size)

/*!
 * Test whether a message descriptor is empty.
 *
 * \param[in] descriptor Message descriptor
 * \return               Test result
 */
#define pb_message_descriptor_empty(descriptor) \
  (assert(descriptor), !pb_message_descriptor_size(descriptor))

/* ------------------------------------------------------------------------- */

/*!
 * Create a const-iterator over a message descriptor.
 *
 * \param[in] descriptor Message descriptor
 * \return               Message descriptor iterator
 */
#define pb_message_descriptor_iter_create(descriptor) \
  (assert(descriptor), \
    (pb_message_descriptor_iter_t){ (descriptor), SIZE_MAX })

/*!
 * Free all allocated memory.
 *
 * \param[in,out] it Message descriptor iterator
 */
#define pb_message_descriptor_iter_destroy(it) \
  (assert(it)) /* Nothing to be done */

/*!
 * Set the position of a message descriptor iterator to the first item.
 *
 * \param[in,out] it Message descriptor iterator
 * \return           Test result
 */
#define pb_message_descriptor_iter_begin(it) \
  (assert(it), !pb_message_descriptor_empty((it)->descriptor) ? \
    !((it)->pos = 0) : 0)

/*!
 * Set the position of a message descriptor iterator to the last item.
 *
 * \param[in,out] it Message descriptor iterator
 * \return           Test result
 */
#define pb_message_descriptor_iter_end(it) \
  (assert(it), !pb_message_descriptor_empty((it)->descriptor) ? \
    !!((it)->pos = pb_message_descriptor_size((it)->descriptor) - 1) : 0)

/*!
 * Decrement the position of a message descriptor iterator.
 *
 * \param[in,out] it Message descriptor iterator
 * \return           Test result
 */
#define pb_message_descriptor_iter_prev(it) \
  (assert((it) && (it)->pos != SIZE_MAX), \
    (!pb_message_descriptor_empty((it)->descriptor) && (it)->pos > 0) ? \
      (it)->pos-- : 0)

/*!
 * Increment the position of a message descriptor iterator.
 *
 * \param[in,out] it Message descriptor iterator
 * \return           Test result
 */
#define pb_message_descriptor_iter_next(it) \
  (assert((it) && (it)->pos != SIZE_MAX), \
    (!pb_message_descriptor_empty((it)->descriptor) && \
      (it)->pos < (pb_message_descriptor_size((it)->descriptor) - 1)) ? \
        ++(it)->pos : 0)

/*!
 * Retrieve the current position of a message descriptor iterator.
 *
 * \param[in] it Message descriptor iterator
 * \return       Current position
 */
#define pb_message_descriptor_iter_pos(it) \
  (assert((it) && (it)->pos != SIZE_MAX), \
    (const size_t)(it)->pos)

/*!
 * Retrieve the field descriptor at the current position.
 *
 * \param[in] it Message descriptor iterator
 * \return       Field descriptor
 */
#define pb_message_descriptor_iter_current(it) \
  (assert((it) && (it)->pos != SIZE_MAX), \
    &((it)->descriptor->field.data[(it)->pos]))

#endif /* PB_INCLUDE_MESSAGE_DESCRIPTOR_H */
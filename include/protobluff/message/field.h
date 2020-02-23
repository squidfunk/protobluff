/*
 * Copyright (c) 2013-2020 Martin Donath <martin.donath@squidfunk.com>
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

#ifndef PB_INCLUDE_MESSAGE_FIELD_H
#define PB_INCLUDE_MESSAGE_FIELD_H

#include <assert.h>
#include <stddef.h>

#include <protobluff/core/descriptor.h>
#include <protobluff/message/common.h>
#include <protobluff/message/part.h>

/* ----------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------- */

struct pb_cursor_t;
struct pb_message_t;

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_field_t {
  const pb_field_descriptor_t
    *descriptor;                       /*!< Field descriptor */
  pb_part_t part;                      /*!< Part */
} pb_field_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_field_t
pb_field_create(
  struct pb_message_t *message,        /* Message */
  pb_tag_t tag);                       /* Tag */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_field_t
pb_field_create_nested(
  struct pb_message_t *message,        /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size);                        /* Tag count */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_field_t
pb_field_create_from_cursor(
  struct pb_cursor_t *cursor);         /* Cursor */

PB_EXPORT void
pb_field_destroy(
  pb_field_t *field);                  /* Field */

PB_EXPORT int
pb_field_match(
  pb_field_t *field,                   /* Field */
  const void *value);                  /* Pointer holding value */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_error_t
pb_field_get(
  pb_field_t *field,                   /* Field */
  void *value);                        /* Pointer receiving value */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_error_t
pb_field_put(
  pb_field_t *field,                   /* Field */
  const void *value);                  /* Pointer holding value */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_error_t
pb_field_clear(
  pb_field_t *field);                  /* Field */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the descriptor of a field.
 *
 * \param[in] field Field
 * \return          Field descriptor
 */
PB_INLINE const pb_field_descriptor_t *
pb_field_descriptor(const pb_field_t *field) {
  assert(field);
  return field->descriptor;
}

/*!
 * Retrieve the part of a field.
 *
 * \param[in] field Field
 * \return          Part
 */
PB_INLINE const pb_part_t *
pb_field_part(const pb_field_t *field) {
  assert(field);
  return &(field->part);
}

/*!
 * Dump a field.
 *
 * \warning Don't use this in production, only for debugging.
 *
 * \param[in] field Field
 */
PB_INLINE void
pb_field_dump(const pb_field_t *field) {
  assert(field);
  pb_part_dump(&(field->part));
}

/*!
 * Retrieve the internal error state of a field.
 *
 * \param[in] field Field
 * \return          Error code
 */
PB_INLINE pb_error_t
pb_field_error(const pb_field_t *field) {
  assert(field);
  return pb_part_error(&(field->part));
}

/*!
 * Test whether a field is valid.
 *
 * \param[in] field Field
 * \return          Test result
 */
PB_INLINE int
pb_field_valid(const pb_field_t *field) {
  assert(field);
  return !pb_field_error(field);
}

#endif /* PB_INCLUDE_MESSAGE_FIELD_H */

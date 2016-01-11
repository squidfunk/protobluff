/*
 * Copyright (c) 2013-2016 Martin Donath <martin.donath@squidfunk.com>
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

#ifndef PB_MESSAGE_FIELD_H
#define PB_MESSAGE_FIELD_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <protobluff/message/field.h>

#include "message/common.h"
#include "message/journal.h"
#include "message/part.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_field_t
pb_field_create_without_default(
  struct pb_message_t *message,        /* Message */
  pb_tag_t tag);                       /* Tag */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Create an invalid field.
 *
 * \return Field
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_field_t
pb_field_create_invalid(void) {
  pb_field_t field = {
    .descriptor = NULL,
    .part       = pb_part_create_invalid()
  };
  return field;
}

/*!
 * Create a shallow copy of a field.
 *
 * \param[in] field Field
 * \return          Field copy
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_field_t
pb_field_copy(const pb_field_t *field) {
  assert(field);
  return *field;
}

/*!
 * Retrieve the underlying journal of a field.
 *
 * \param[in] field Field
 * \return          Journal
 */
PB_INLINE pb_journal_t *
pb_field_journal(pb_field_t *field) {
  assert(field);
  return pb_part_journal(&(field->part));
}

/*!
 * Retrieve the version of a field.
 *
 * \param[in] field Field
 * \return            Version
 */
PB_INLINE pb_version_t
pb_field_version(const pb_field_t *field) {
  assert(field);
  return pb_part_version(&(field->part));
}

/*!
 * Test whether a field is properly aligned.
 *
 * \param[in] field Field
 * \return          Test result
 */
PB_INLINE int
pb_field_aligned(const pb_field_t *field) {
  assert(field);
  return pb_part_aligned(&(field->part));
}

/*!
 * Ensure that a field is properly aligned.
 *
 * \param[in,out] field Field
 * \return              Error code
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_error_t
pb_field_align(pb_field_t *field) {
  assert(field);
  return !pb_part_aligned(&(field->part))
    ? pb_part_align(&(field->part))
    : PB_ERROR_NONE;
}

/*!
 * Retrieve the start offset of a field within its underlying journal.
 *
 * \param[in] field Field
 * \return          Start offset
 */
PB_INLINE size_t
pb_field_start(const pb_field_t *field) {
  assert(field);
  return pb_part_start(&(field->part));
}

/*!
 * Retrieve the end offset of a field within its underlying journal.
 *
 * \param[in] field Field
 * \return          End offset
 */
PB_INLINE size_t
pb_field_end(const pb_field_t *field) {
  assert(field);
  return pb_part_end(&(field->part));
}

/*!
 * Retrieve the size of a field.
 *
 * \param[in] field Field
 * \return          Field size
 */
PB_INLINE size_t
pb_field_size(const pb_field_t *field) {
  assert(field);
  return pb_part_size(&(field->part));
}

/*!
 * Test whether a field is field.
 *
 * \param[in] field Field
 * \return          Test result
 */
PB_INLINE int
pb_field_empty(const pb_field_t *field) {
  assert(field);
  return pb_part_empty(&(field->part));
}

/* ------------------------------------------------------------------------- */

/*!
 * Test whether two fields are the same.
 *
 * \warning Field alignment is checked implicitly by comparing the fields'
 * memory segments and thus their versions.
 *
 * \param[in] x Field
 * \param[in] y Field
 * \return      Test result
 */
PB_INLINE int
pb_field_equals(const pb_field_t *x, const pb_field_t *y) {
  assert(x && y);
  return !memcmp(x, y, sizeof(pb_field_t));
}

#endif /* PB_MESSAGE_FIELD_H */
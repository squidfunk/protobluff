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

#ifndef PB_FIELD_H
#define PB_FIELD_H

#include <assert.h>
#include <string.h>

#include <protobluff/field.h>

#include "lib/common.h"
#include "lib/part.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_field_t
pb_field_create_without_default(
  struct pb_message_t *message,        /* Message */
  pb_tag_t tag);                       /* Tag */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Create an invalid field.
 *
 * \return Field
 */
#define pb_field_create_invalid() \
  ((pb_field_t){ .part = pb_part_create_invalid() })

/*!
 * Create a copy of a field.
 *
 * \param[in] field Field
 * \return          Field copy
 */
#define pb_field_copy(field) \
  (assert(field), *(const pb_field_t *)(field))

/*!
 * Retrieve the part of a field.
 *
 * \param[in] field Field
 * \return          Part
 */
#define pb_field_part(field) \
  (assert(field), (const pb_part_t *)&((field)->part))

/*!
 * Retrieve the underlying binary of a field.
 *
 * \param[in] field Field
 * \return          Binary
 */
#define pb_field_binary(field) \
  (assert(field), pb_part_binary(&((field)->part)))

/*!
 * Retrieve the version of a field.
 *
 * \param[in] field Field
 * \return          Version
 */
#define pb_field_version(field) \
  (assert(field), pb_part_version(&((field)->part)))

/*!
 * Retrieve the start offset of a field within its underlying binary.
 *
 * \param[in] field Field
 * \return          Start offset
 */
#define pb_field_start(field) \
  (assert(field), pb_part_start(&((field)->part)))

/*!
 * Retrieve the end offset of a field within its underlying binary.
 *
 * \param[in] field Field
 * \return          End offset
 */
#define pb_field_end(field) \
  (assert(field), pb_part_end(&((field)->part)))

/*!
 * Retrieve the size of a field.
 *
 * \param[in] field Field
 * \return          Field size
 */
#define pb_field_size(field) \
  (assert(field), pb_part_size(&((field)->part)))

/*!
 * Test whether a field is empty.
 *
 * \param[in] field Field
 * \return          Test result
 */
#define pb_field_empty(field) \
  (assert(field), pb_part_empty(&((field)->part)))

/*!
 * Ensure that a field is properly aligned.
 *
 * \param[in,out] field Field
 * \return              Error code
 */
#define pb_field_align(field) \
  (assert(field), !pb_part_aligned(&((field)->part)) \
    ? pb_part_align(&((field)->part)) : PB_ERROR_NONE)

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
#define pb_field_equals(x, y) \
  (assert((x) && (y)), !memcmp((x), (y), sizeof(pb_field_t)))

#endif /* PB_FIELD_H */
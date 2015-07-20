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

#ifndef PB_CURSOR_H
#define PB_CURSOR_H

#include <assert.h>

#include <protobluff/cursor.h>

#include "lib/common.h"
#include "lib/message.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_cursor_t
pb_cursor_create_internal(
  pb_message_t *message,               /* Message */
  pb_tag_t tag);                       /* Tag */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_cursor_align(
  pb_cursor_t *cursor);                /* Cursor */

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Create a cursor without a tag, halting on every field.
 *
 * \param[in] message Message
 * \return            Cursor
 */
#define pb_cursor_create_without_tag(message) \
  (assert(message), pb_cursor_create_internal((message), 0))

/*!
 * Create an invalid cursor.
 *
 * \return Cursor
 */
#define pb_cursor_create_invalid() \
  ((pb_cursor_t){ \
    .message = pb_message_create_invalid(), \
    .error   = PB_ERROR_INVALID })

/*!
 * Create a copy of a cursor.
 *
 * \param[in] cursor Cursor
 * \return           Cursor copy
 */
#define pb_cursor_copy(cursor) \
  (assert(cursor), *(const pb_cursor_t *)(cursor))

/*!
 * Retrieve the tag a cursor is looking for.
 *
 * \param[in] cursor Cursor
 * \return           Tag to look for
 */
#define pb_cursor_tag_internal(cursor) \
  (assert(cursor), (const pb_tag_t)(cursor)->tag)

/*!
 * Retrieve the underlying binary of a cursor.
 *
 * \param[in] cursor Cursor
 * \return           Binary
 */
#define pb_cursor_binary(cursor) \
  (assert(cursor), pb_message_binary(&((cursor)->message)))

/*!
 * Retrieve the version of a cursor.
 *
 * \param[in] cursor Cursor
 * \return           Version
 */
#define pb_cursor_version(cursor) \
  (assert(cursor), pb_message_version(&((cursor)->message)))

/*!
 * Retrieve the offsets at the current position of a cursor.
 *
 * \param[in] cursor Cursor
 * \return           Offsets
 */
#define pb_cursor_current(cursor) \
  (assert(cursor), (const pb_offset_t *)&((cursor)->current.offset))

#endif /* PB_CURSOR_H */
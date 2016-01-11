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

#ifndef PB_INCLUDE_MESSAGE_ONEOF_H
#define PB_INCLUDE_MESSAGE_ONEOF_H

#include <assert.h>

#include <protobluff/core/descriptor.h>
#include <protobluff/message/common.h>
#include <protobluff/message/cursor.h>

/* ----------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------- */

struct pb_message_t;

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_oneof_t {
  const pb_oneof_descriptor_t
    *descriptor;                       /*!< Oneof descriptor */
  pb_cursor_t cursor;                  /*!< Cursor */
} pb_oneof_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_oneof_t
pb_oneof_create(
  const pb_oneof_descriptor_t
    *descriptor,                       /* Oneof descriptor */
  struct pb_message_t *message);       /* Message */

PB_EXPORT void
pb_oneof_destroy(
  pb_oneof_t *oneof);                  /* Oneof */

PB_EXPORT pb_tag_t
pb_oneof_case(
  pb_oneof_t *oneof);                  /* Oneof */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_oneof_clear(
  pb_oneof_t *oneof);                  /* Oneof */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the descriptor of a oneof.
 *
 * \param[in] oneof Oneof
 * \return          Oneof Descriptor
 */
PB_INLINE const pb_oneof_descriptor_t *
pb_oneof_descriptor(const pb_oneof_t *oneof) {
  assert(oneof);
  return oneof->descriptor;
}

/*!
 * Retrieve the internal error state of a oneof.
 *
 * \param[in] oneof Oneof
 * \return          Error code
 */
PB_INLINE pb_error_t
pb_oneof_error(const pb_oneof_t *oneof) {
  assert(oneof);
  return pb_cursor_error(&(oneof->cursor)) != PB_ERROR_EOM
    ? pb_cursor_error(&(oneof->cursor))
    : PB_ERROR_NONE;
}

/*!
 * Test whether a oneof is valid.
 *
 * \param[in] oneof Oneof
 * \return          Test result
 */
PB_INLINE int
pb_oneof_valid(const pb_oneof_t *oneof) {
  assert(oneof);
  return !pb_oneof_error(oneof);
}

#endif /* PB_INCLUDE_MESSAGE_ONEOF_H */
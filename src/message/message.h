/*
 * Copyright (c) 2013-2017 Martin Donath <martin.donath@squidfunk.com>
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

#ifndef PB_MESSAGE_MESSAGE_H
#define PB_MESSAGE_MESSAGE_H

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <protobluff/message/message.h>

#include "message/common.h"
#include "message/journal.h"
#include "message/part.h"

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Create an invalid message.
 *
 * \return Message
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_message_t
pb_message_create_invalid(void) {
  pb_message_t message = {
    .descriptor = NULL,
    .part       = pb_part_create_invalid()
  };
  return message;
}

/*!
 * Create a shallow copy of a message.
 *
 * \param[in] message Message
 * \return            Message copy
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_message_t
pb_message_copy(const pb_message_t *message) {
  assert(message);
  return *message;
}

/*!
 * Retrieve the underlying journal of a message.
 *
 * \param[in] message Message
 * \return            Journal
 */
PB_INLINE pb_journal_t *
pb_message_journal(pb_message_t *message) {
  assert(message);
  return pb_part_journal(&(message->part));
}

/*!
 * Retrieve the version of a message.
 *
 * \param[in] message Message
 * \return            Version
 */
PB_INLINE pb_version_t
pb_message_version(const pb_message_t *message) {
  assert(message);
  return pb_part_version(&(message->part));
}

/*!
 * Test whether a message is properly aligned.
 *
 * \param[in] message Message
 * \return            Test result
 */
PB_INLINE int
pb_message_aligned(const pb_message_t *message) {
  assert(message);
  return pb_part_aligned(&(message->part));
}

/*!
 * Ensure that a message is properly aligned.
 *
 * \param[in,out] message Message
 * \return                Error code
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_error_t
pb_message_align(pb_message_t *message) {
  assert(message);
  return !pb_part_aligned(&(message->part))
    ? pb_part_align(&(message->part))
    : PB_ERROR_NONE;
}

/*!
 * Retrieve the start offset of a message within its underlying journal.
 *
 * \param[in] message Message
 * \return            Start offset
 */
PB_INLINE size_t
pb_message_start(const pb_message_t *message) {
  assert(message);
  return pb_part_start(&(message->part));
}

/*!
 * Retrieve the end offset of a message within its underlying journal.
 *
 * \param[in] message Message
 * \return            End offset
 */
PB_INLINE size_t
pb_message_end(const pb_message_t *message) {
  assert(message);
  return pb_part_end(&(message->part));
}

/*!
 * Retrieve the size of a message.
 *
 * \param[in] message Message
 * \return            Message size
 */
PB_INLINE size_t
pb_message_size(const pb_message_t *message) {
  assert(message);
  return pb_part_size(&(message->part));
}

/*!
 * Test whether a message is empty.
 *
 * \param[in] message Message
 * \return            Test result
 */
PB_INLINE int
pb_message_empty(const pb_message_t *message) {
  assert(message);
  return pb_part_empty(&(message->part));
}

/* ------------------------------------------------------------------------- */

/*!
 * Test whether two messages are the same.
 *
 * \warning Message alignment is checked implicitly by comparing the messages'
 * memory segments and thus their versions.
 *
 * \param[in] x Message
 * \param[in] y Message
 * \return      Test result
 */
PB_INLINE int
pb_message_equals(const pb_message_t *x, const pb_message_t *y) {
  assert(x && y);
  return !memcmp(x, y, sizeof(pb_message_t));
}

#endif /* PB_MESSAGE_MESSAGE_H */

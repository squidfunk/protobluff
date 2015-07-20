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

#ifndef PB_MESSAGE_H
#define PB_MESSAGE_H

#include <assert.h>
#include <string.h>

#include <protobluff/message.h>

#include "lib/part.h"

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Create an invalid message.
 *
 * \return Message
 */
#define pb_message_create_invalid() \
  ((pb_message_t){ .part = pb_part_create_invalid() })

/*!
 * Create a copy of a message.
 *
 * \param[in] message Message
 * \return            Message copy
 */
#define pb_message_copy(message) \
  (assert(message), *(const pb_message_t *)(message))

/*!
 * Retrieve the part of a message.
 *
 * \param[in] message Message
 * \return            Part
 */
#define pb_message_part(message) \
  (assert(message), (const pb_part_t *)&((message)->part))

/*!
 * Retrieve the underlying binary of a message.
 *
 * \param[in] message Message
 * \return            Binary
 */
#define pb_message_binary(message) \
  (assert(message), pb_part_binary(&((message)->part)))

/*!
 * Retrieve the version of a message.
 *
 * \param[in] message Message
 * \return            Version
 */
#define pb_message_version(message) \
  (assert(message), pb_part_version(&((message)->part)))

/*!
 * Retrieve the start offset of a message within its underlying binary.
 *
 * \param[in] message Message
 * \return            Start offset
 */
#define pb_message_start(message) \
  (assert(message), pb_part_start(&((message)->part)))

/*!
 * Retrieve the end offset of a message within its underlying binary.
 *
 * \param[in] message Message
 * \return            End offset
 */
#define pb_message_end(message) \
  (assert(message), pb_part_end(&((message)->part)))

/*!
 * Retrieve the size of a message.
 *
 * \param[in] message Message
 * \return            Message size
 */
#define pb_message_size(message) \
  (assert(message), pb_part_size(&((message)->part)))

/*!
 * Test whether a message is empty.
 *
 * \param[in] message Message
 * \return            Test result
 */
#define pb_message_empty(message) \
  (assert(message), pb_part_empty(&((message)->part)))

/*!
 * Ensure that a message is properly aligned.
 *
 * \param[in,out] message Message
 * \return                Error code
 */
#define pb_message_align(message) \
  (assert(message), !pb_part_aligned(&((message)->part)) \
    ? pb_part_align(&((message)->part)) : PB_ERROR_NONE)

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
#define pb_message_equals(x, y) \
  (assert((x) && (y)), !memcmp((x), (y), sizeof(pb_message_t)))

#endif /* PB_MESSAGE_H */
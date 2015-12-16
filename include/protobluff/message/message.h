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

#ifndef PB_INCLUDE_MESSAGE_MESSAGE_H
#define PB_INCLUDE_MESSAGE_MESSAGE_H

#include <assert.h>
#include <stddef.h>

#include <protobluff/core/descriptor.h>
#include <protobluff/message/common.h>
#include <protobluff/message/journal.h>
#include <protobluff/message/part.h>

/* ----------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------- */

struct pb_cursor_t;
struct pb_field_t;

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_message_t {
  const pb_descriptor_t *descriptor;   /*!< Descriptor */
  pb_part_t part;                      /*!< Part */
} pb_message_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_message_t
pb_message_create(
  const pb_descriptor_t *descriptor,   /* Descriptor */
  pb_journal_t *journal);              /* Journal */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_message_t
pb_message_create_within(
  pb_message_t *message,               /* Message */
  pb_tag_t tag);                       /* Tag */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_message_t
pb_message_create_nested(
  pb_message_t *message,               /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size);                        /* Tag count */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_message_t
pb_message_create_from_cursor(
  struct pb_cursor_t *cursor);         /* Cursor */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_message_t
pb_message_create_from_field(
  const pb_descriptor_t *descriptor,   /* Descriptor */
  struct pb_field_t *field);           /* Bytes field */

PB_EXPORT void
pb_message_destroy(
  pb_message_t *message);              /* Message */

PB_EXPORT pb_error_t
pb_message_check(
  const pb_message_t *message);        /* Message */

PB_EXPORT int
pb_message_has(
  pb_message_t *message,               /* Message */
  pb_tag_t tag);                       /* Tag */

PB_EXPORT int
pb_message_match(
  pb_message_t *message,               /* Message */
  pb_tag_t tag,                        /* Tag */
  const void *value);                  /* Pointer holding value */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_message_get(
  pb_message_t *message,               /* Message */
  pb_tag_t tag,                        /* Tag */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_message_put(
  pb_message_t *message,               /* Message */
  pb_tag_t tag,                        /* Tag */
  const void *value);                  /* Pointer holding value */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_message_erase(
  pb_message_t *message,               /* Message */
  pb_tag_t tag);                       /* Tag */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_message_clear(
  pb_message_t *message);              /* Message */

PB_DEPRECATED
PB_EXPORT void *
pb_message_raw(
  pb_message_t *message,               /* Message */
  pb_tag_t tag);                       /* Tag */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the descriptor of a message.
 *
 * \param[in] message Message
 * \return            Descriptor
 */
PB_INLINE const pb_descriptor_t *
pb_message_descriptor(const pb_message_t *message) {
  assert(message);
  return message->descriptor;
}

/*!
 * Retrieve the part of a message.
 *
 * \param[in] message Message
 * \return            Part
 */
PB_INLINE const pb_part_t *
pb_message_part(const pb_message_t *message) {
  assert(message);
  return &(message->part);
}

/*!
 * Dump a message.
 *
 * \warning Don't use this in production, only for debugging.
 *
 * \param[in] message Message
 */
PB_INLINE void
pb_message_dump(const pb_message_t *message) {
  assert(message);
  pb_part_dump(&(message->part));
}

/*!
 * Retrieve the internal error state of a message.
 *
 * \param[in] message Message
 * \return            Error code
 */
PB_INLINE pb_error_t
pb_message_error(const pb_message_t *message) {
  assert(message);
  return pb_part_error(&(message->part));
}

/*!
 * Test whether a message is valid.
 *
 * \param[in] message Message
 * \return            Test result
 */
PB_INLINE int
pb_message_valid(const pb_message_t *message) {
  assert(message);
  return !pb_message_error(message);
}

#endif /* PB_INCLUDE_MESSAGE_MESSAGE_H */
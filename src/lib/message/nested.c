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

#include <assert.h>
#include <stdlib.h>

#include "lib/common.h"
#include "lib/cursor.h"
#include "lib/field/descriptor.h"
#include "lib/message.h"
#include "lib/message/descriptor.h"
#include "lib/message/nested.h"

/* ----------------------------------------------------------------------------
 * Internal functions
 * ------------------------------------------------------------------------- */

/*!
 * Resolve a submessage within a message recursively for a branch of tags.
 *
 * If a submessage is not existent, it is not created. This does resemble
 * nearly the same functionality as pb_message_create_nested(), except that
 * messages are not implicitly created when not found, which is necessary as
 * all read operations need to be non-destructive.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \return                Cursor
 */
static pb_cursor_t
resolve(pb_message_t *message, const pb_tag_t tags[], size_t size) {
  assert(message && tags && size);
  pb_cursor_t cursor = pb_cursor_create_invalid();
  if (unlikely_(!pb_message_valid(message)))
    return cursor;

  /* Resolve message recursively */
  pb_message_t copy = pb_message_copy(message);
  for (size_t t = 0; t < size; ++t) {
    pb_cursor_destroy(&cursor);

#ifndef NDEBUG

    /* Assert non-repeated messages */
    const pb_field_descriptor_t *descriptor =
      pb_message_descriptor_field_by_tag(
        pb_message_descriptor(message), tags[t]);
    assert(descriptor &&
      pb_field_descriptor_type(descriptor)  == PB_TYPE_MESSAGE &&
      pb_field_descriptor_label(descriptor) != PB_LABEL_REPEATED);

#endif /* NDEBUG */

    /* Use cursor to omit message creation */
    cursor = pb_cursor_create(&copy, tags[t]);
    if (!pb_cursor_valid(&cursor))
      break;

    /* Extract next message from cursor */
    pb_message_destroy(&copy);
    copy = pb_message_create_from_cursor(&cursor);
  }
  pb_message_destroy(&copy);
  return cursor;
}

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Test whether a nested message contains at least one field for a given tag.
 *
 * Whether the message is valid or not is checked by the cursor, so there is
 * no need to perform this check before creating the cursor.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \return                Test result
 */
extern int
pb_message_nested_has(
    pb_message_t *message, const pb_tag_t tags[], size_t size) {
  assert(message && tags && size > 1);
  int result = 0;

  /* Use cursor to omit field creation */
  pb_cursor_t cursor = resolve(message, tags, --size);
  if (pb_cursor_valid(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);
    result = pb_message_has(&submessage, tags[size]);
    pb_message_destroy(&submessage);
  }
  pb_cursor_destroy(&cursor);
  return result;
}

/*!
 * Compare the value from a nested message with the given value.
 *
 * Whether the message is valid or not is checked by the cursor, so there is
 * no need to perform this check before creating the cursor.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \param[in]     value   Pointer holding value
 * \return                Test result
 */
extern int
pb_message_nested_match(
    pb_message_t *message,
    const pb_tag_t tags[], size_t size,
    const void *value) {
  assert(message && tags && size > 1 && value);
  int result = 0;

  /* Use cursor to omit field creation */
  pb_cursor_t cursor = resolve(message, tags, --size);
  if (pb_cursor_valid(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);
    result = pb_message_match(&submessage, tags[size], value);
    pb_message_destroy(&submessage);
  }
  pb_cursor_destroy(&cursor);
  return result;
}

/*!
 * Read the value from a nested message for a branch of tags.
 *
 * Whether the message is valid or not is checked by the cursor, so there is
 * no need to perform this check before creating the cursor.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \param[out]    value   Pointer receiving value
 * \return                Error code
 */
extern pb_error_t
pb_message_nested_get(
    pb_message_t *message,
    const pb_tag_t tags[], size_t size,
    void *value) {
  assert(message && tags && size > 1 && value);
  pb_error_t error = PB_ERROR_INVALID;

  /* Use cursor to omit field creation */
  pb_cursor_t cursor = resolve(message, tags, --size);
  if (unlikely_(!pb_cursor_valid(&cursor))) {
    if ((error = pb_cursor_error(&cursor)) == PB_ERROR_OFFSET)
      error = PB_ERROR_ABSENT;
  } else {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);
    error = pb_message_get(&submessage, tags[size], value);
    pb_message_destroy(&submessage);
  }
  pb_cursor_destroy(&cursor);
  return error;
}

/*!
 * Write a value or submessage for a branch of tags to a nested message.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \param[in]     value   Pointer holding value
 * \return                Error code
 */
extern pb_error_t
pb_message_nested_put(
    pb_message_t *message,
    const pb_tag_t tags[], size_t size,
    const void *value) {
  assert(message && tags && size > 1 && value);
  pb_message_t submessage =
    pb_message_create_nested(message, tags, --size);
  pb_error_t error = pb_message_put(&submessage, tags[size], value);
  pb_message_destroy(&submessage);
  return error;
}

/*!
 * Erase a field or submessage for a branch of tags from a nested message.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \return                Error code
 */
extern pb_error_t
pb_message_nested_erase(
    pb_message_t *message, const pb_tag_t tags[], size_t size) {
  assert(message && tags && size > 1);
  pb_error_t error = PB_ERROR_INVALID;

  /* Use cursor to omit field creation */
  pb_cursor_t cursor = resolve(message, tags, --size);
  if (pb_cursor_valid(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);
    error = pb_message_erase(&submessage, tags[size]);
    pb_message_destroy(&submessage);
  }
  pb_cursor_destroy(&cursor);
  return error;
}

/*!
 * Retrieve a pointer to the raw data for a branch of tags from a message.
 *
 * \param[in,out] message Message
 * \param[in]     tags[]  Tags
 * \param[in]     size    Tag count
 * \return                Error code
 */
extern void *
pb_message_nested_raw(
    pb_message_t *message, const pb_tag_t tags[], size_t size) {
  assert(message && tags && size > 1);
  void *value = NULL;

  /* Use cursor to omit field creation */
  pb_cursor_t cursor = resolve(message, tags, --size);
  if (pb_cursor_valid(&cursor)) {
    pb_message_t submessage = pb_message_create_from_cursor(&cursor);
    value = pb_message_raw(&submessage, tags[size]);
    pb_message_destroy(&submessage);
  }
  pb_cursor_destroy(&cursor);
  return value;
}
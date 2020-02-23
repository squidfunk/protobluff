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

#ifndef PB_INCLUDE_MESSAGE_NESTED_H
#define PB_INCLUDE_MESSAGE_NESTED_H

#include <stddef.h>

#include <protobluff/message/common.h>
#include <protobluff/message/message.h>

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT int
pb_message_nested_has(
  pb_message_t *message,               /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size);                        /* Tag count */

PB_EXPORT int
pb_message_nested_match(
  pb_message_t *message,               /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size,                         /* Tag count */
  const void *value);                  /* Pointer holding value */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_error_t
pb_message_nested_get(
  pb_message_t *message,               /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size,                         /* Tag count */
  void *value);                        /* Pointer receiving value */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_error_t
pb_message_nested_put(
  pb_message_t *message,               /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size,                         /* Tag count */
  const void *value);                  /* Pointer holding value */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_error_t
pb_message_nested_erase(
  pb_message_t *message,               /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size);                        /* Tag count */

#endif /* PB_INCLUDE_MESSAGE_NESTED_H */

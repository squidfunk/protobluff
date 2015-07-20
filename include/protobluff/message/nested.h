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

#ifndef PB_INCLUDE_MESSAGE_NESTED_H
#define PB_INCLUDE_MESSAGE_NESTED_H

#include <stddef.h>

#include <protobluff/common.h>

/* ----------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------- */

struct pb_message_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT int
pb_message_nested_has(
  struct pb_message_t *message,        /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size);                        /* Tag count */

PB_EXPORT int
pb_message_nested_match(
  struct pb_message_t *message,        /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size,                         /* Tag count */
  const void *value);                  /* Pointer holding value */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_message_nested_get(
  struct pb_message_t *message,        /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size,                         /* Tag count */
  void *value);                        /* Pointer receiving value */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_message_nested_put(
  struct pb_message_t *message,        /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size,                         /* Tag count */
  const void *value);                  /* Pointer holding value */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_message_nested_erase(
  struct pb_message_t *message,        /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size);                        /* Tag count */

PB_EXPORT void *
pb_message_nested_raw(
  struct pb_message_t *message,        /* Message */
  const pb_tag_t tags[],               /* Tags */
  size_t size);                        /* Tag count */

#endif /* PB_INCLUDE_MESSAGE_NESTED_H */
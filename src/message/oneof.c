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

#include <assert.h>
#include <stdlib.h>

#include "core/descriptor.h"
#include "message/common.h"
#include "message/cursor.h"
#include "message/message.h"
#include "message/oneof.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a oneof from a descriptor and a message.
 *
 * \param[in]     descriptor Oneof descriptor
 * \param[in,out] message    Message
 * \return                   Oneof
 */
extern pb_oneof_t
pb_oneof_create(
    const pb_oneof_descriptor_t *descriptor, pb_message_t *message) {
  assert(descriptor && message);
  pb_oneof_t oneof = {
    .descriptor = descriptor,
    .cursor     = pb_cursor_create_without_tag(message)
  };
  return oneof;
}

/*!
 * Destroy a oneof.
 *
 * \param[in,out] oneof Oneof
 */
extern void
pb_oneof_destroy(pb_oneof_t *oneof) {
  assert(oneof);
  pb_cursor_destroy(&(oneof->cursor));
}

/*!
 * Retrieve the active tag of a oneof.
 *
 * \param[in,out] oneof Oneof
 * \return              Tag
 */
extern pb_tag_t
pb_oneof_case(pb_oneof_t *oneof) {
  assert(oneof);
  pb_tag_t tag = 0;
  if (pb_cursor_rewind(&(oneof->cursor))) {
    do {
      const pb_field_descriptor_t *descriptor =
        pb_cursor_descriptor(&(oneof->cursor));
      if (pb_field_descriptor_oneof(descriptor) == oneof->descriptor)
        tag = pb_cursor_tag(&(oneof->cursor));
    } while (pb_cursor_next(&(oneof->cursor)));
  }
  return tag;
}

/*!
 * Clear all members of a oneof.
 *
 * \param[in,out] oneof Oneof
 * \return              Error code
 */
extern pb_error_t
pb_oneof_clear(pb_oneof_t *oneof) {
  assert(oneof);
  pb_error_t error = PB_ERROR_NONE;
  if (pb_cursor_rewind(&(oneof->cursor))) {
    do {
      const pb_field_descriptor_t *descriptor =
        pb_cursor_descriptor(&(oneof->cursor));
      if (pb_field_descriptor_oneof(descriptor) == oneof->descriptor)
        error = pb_cursor_erase(&(oneof->cursor));
    } while (!error && pb_cursor_next(&(oneof->cursor)));
  }
  return error;
}
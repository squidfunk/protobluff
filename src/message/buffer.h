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

#ifndef PB_MESSAGE_BUFFER_H
#define PB_MESSAGE_BUFFER_H

#include <stdint.h>
#include <stdlib.h>

#include <protobluff/message/buffer.h>

#include "core/buffer.h"
#include "message/common.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_buffer_write(
  pb_buffer_t *buffer,                 /* Buffer */
  size_t start,                        /* Start offset */
  size_t end,                          /* End offset */
  const uint8_t data[],                /* Raw data */
  size_t size);                        /* Raw data size */

PB_WARN_UNUSED_RESULT
extern pb_error_t
pb_buffer_clear(
  pb_buffer_t *buffer,                 /* Buffer */
  size_t start,                        /* Start offset */
  size_t end);                         /* End offset */

extern void
pb_buffer_dump_range(
  const pb_buffer_t *buffer,           /* Buffer */
  size_t start,                        /* Start offset */
  size_t end);                         /* End offset */

#endif /* PB_MESSAGE_BUFFER_H */
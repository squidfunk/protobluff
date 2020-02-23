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

#ifndef PB_INCLUDE_MESSAGE_COMMON_H
#define PB_INCLUDE_MESSAGE_COMMON_H

#include <stddef.h>

#include <protobluff/core/common.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef size_t pb_version_t;           /*!< Version */

/* ------------------------------------------------------------------------- */

typedef struct pb_offset_t {
  size_t start;                        /*!< Start offset */
  size_t end;                          /*!< End offset */
  struct {
    ptrdiff_t origin;                  /*!< Relative offset to origin */
    ptrdiff_t tag;                     /*!< Relative offset to tag */
    ptrdiff_t length;                  /*!< Relative offset to length */
  } diff;
} pb_offset_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT const char *
pb_error_string(
  pb_error_t error);                   /* Error */

#endif /* PB_INCLUDE_MESSAGE_COMMON_H */

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

#ifndef PB_INCLUDE_UTIL_DESCRIPTOR_H
#define PB_INCLUDE_UTIL_DESCRIPTOR_H

#include <protobluff/core/common.h>
#include <protobluff/core/descriptor.h>

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT const pb_field_descriptor_t *
pb_descriptor_field_by_name(
  const pb_descriptor_t *descriptor,   /* Descriptor */
  const char name[]);                  /* Name */

/* ------------------------------------------------------------------------- */

PB_EXPORT const pb_enum_value_descriptor_t *
pb_enum_descriptor_value_by_name(
  const pb_enum_descriptor_t
    *descriptor,                       /* Enum descriptor */
  const char name[]);                  /* Name */

#endif /* PB_INCLUDE_UTIL_DESCRIPTOR_H */

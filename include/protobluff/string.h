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

#ifndef PB_INCLUDE_STRING_H
#define PB_INCLUDE_STRING_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_string_t {
  uint8_t *data;                       /*!< Bytes */
  size_t size;                         /*!< Byte count */
} pb_string_t;

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

#define pb_string_binary_(data, size) \
  ((pb_string_t){ (size) ? (uint8_t *)(data) : NULL, (size) })

#define pb_string_printable_(data) \
  (pb_string_binary_((data), strlen(data)))

/* ------------------------------------------------------------------------- */

#define pb_string_args_method_(_0, _2, _1, n, args...) n

#define pb_string_args_(args...) \
  pb_string_args_method_(0, args, binary, printable, 0)

#define pb_string_method_(method, args...) \
  pb_string_ ## method ## _(args)

#define pb_string_(method, args...) \
  pb_string_method_(method, args)

/* ------------------------------------------------------------------------- */

/*!
 * Initialize an immutable string from a constant.
 *
 * \warning Internally, string constants are referenced without the trailing
 * nul-terminator for reasons of consistency.
 *
 * \param[in] data String data
 * \return         String
 */
#define pb_string_const(data) \
  { (uint8_t *)(data), (sizeof(data) / sizeof(*data)) - 1 }

/*!
 * Initialize an immutable string.
 *
 * If only one argument is given, it is assumed that the string is printable,
 * and thus nul-terminated. Otherwise, the size must be passed.
 *
 * \warning This function may not be used for static initializers, as it
 * checks the string's length with strlen() which will not compile. In order
 * to initialize a static variable, use pb_string_const() instead.
 *
 * \param[in] args String data, optional size
 * \return         String
 */
#define pb_string_init(args...) \
  (pb_string_(pb_string_args_(args), args))

#endif /* PB_INCLUDE_STRING_H */
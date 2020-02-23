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

#ifndef PB_INCLUDE_CORE_STRING_H
#define PB_INCLUDE_CORE_STRING_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <protobluff/core/common.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_string_t {
  uint8_t *data;                       /*!< Raw data */
  size_t size;                         /*!< Raw data size */
} pb_string_t;

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Initialize a string.
 *
 * \param[in,out] data Raw data
 * \param[in]     size Raw data size
 * \return             String
 */
PB_INLINE pb_string_t
pb_string_init(uint8_t data[], size_t size) {
  assert(data);
  pb_string_t string = {
    .data = data,
    .size = size
  };
  return string;
}

/*!
 * Initialize an empty string.
 *
 * \return String
 */
PB_INLINE pb_string_t
pb_string_init_empty(void) {
  return pb_string_init(NULL, 0);
}

/*!
 * Initialize a string from a nul-terminated array of characters.
 *
 * \param[in,out] data Characters
 * \return             String
 */
PB_INLINE pb_string_t
pb_string_init_from_chars(char *data) {
  return pb_string_init((uint8_t *)data, strlen(data));
}

/*!
 * Retrieve the raw data of a string.
 *
 * \param[in] string String
 * \return           Raw data
 */
PB_INLINE uint8_t *
pb_string_data(const pb_string_t *string) {
  assert(string);
  return string->data;
}

/*!
 * Retrieve the size of a string.
 *
 * \param[in] string String
 * \return           Raw data size
 */
PB_INLINE size_t
pb_string_size(const pb_string_t *string) {
  assert(string);
  return string->size;
}

/*!
 * Test whether a string is empty.
 *
 * \param[in] string String
 * \return           Test result
 */
PB_INLINE int
pb_string_empty(const pb_string_t *string) {
  assert(string);
  return !pb_string_size(string);
}

/*!
 * Test whether two strings are the same.
 *
 * \param[in] x String
 * \param[in] y String
 * \return      Test result
 */
PB_INLINE int
pb_string_equals(const pb_string_t *x, const pb_string_t *y) {
  assert(x && y);
  return !memcmp(pb_string_data(x), pb_string_data(y), pb_string_size(y));
}

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Initialize a const string for a static definition.
 *
 * \warning Internally, string constants are referenced without the trailing
 * nul-terminator for reasons of consistency.
 *
 * \param[in] data String data
 * \return         String
 */
#define pb_string_const(data) \
  { (uint8_t *)(data), (sizeof(data) / sizeof(*data)) - 1 }

#endif /* PB_INCLUDE_CORE_STRING_H */

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
#include <string.h>

#include "lib/common.h"
#include "lib/enum/descriptor.h"

/* ----------------------------------------------------------------------------
 * Macros
 * ------------------------------------------------------------------------- */

/*!
 * Return the smaller of two values.
 *
 * \param[in] x Value to compare
 * \param[in] y Value to compare
 * \return      Smaller value
 */
#define min(x, y) \
  ((x) < (y) ? (x) : (y))

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the value for a given number from an enum descriptor.
 *
 * \warning The fields actually need to be in ascending order, so you better
 * ensure that or be pleasantly suprised by undefined behaviour.
 *
 * \param[in] descriptor Enum descriptor
 * \param[in] number     Number
 * \return               Enum descriptor value
 */
extern const struct pb_enum_descriptor_value_t *
pb_enum_descriptor_value_by_number(
    const pb_enum_descriptor_t *descriptor, pb_enum_t number) {
  assert(descriptor && number);
  for (size_t v = min(number, descriptor->value.size); v > 0; v--) {
    if (pb_enum_descriptor_value_number(
        &(descriptor->value.data[v - 1])) == number) {
      return &(descriptor->value.data[v - 1]);
    } else if (pb_enum_descriptor_value_number(
        &(descriptor->value.data[v - 1])) < number) {
      break;
    }
  }
  return NULL;
}

/*!
 * Retrieve the value for a given name from an enum descriptor.
 *
 * \warning Querying a enum descriptor by name is far less efficient than
 * querying by number, as a lot more comparisons are involved.
 *
 * \param[in] descriptor Enum descriptor
 * \param[in] name       Name
 * \return               Enum descriptor value
 */
extern const struct pb_enum_descriptor_value_t *
pb_enum_descriptor_value_by_name(
    const pb_enum_descriptor_t *descriptor, const char name[]) {
  assert(descriptor && name);
  for (size_t v = 0; v < descriptor->value.size; v++) {
    if (strcmp(pb_enum_descriptor_value_name(
        &(descriptor->value.data[v])), name))
      continue;
    return &(descriptor->value.data[v]);
  }
  return NULL;
}
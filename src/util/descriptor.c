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

#include "core/common.h"
#include "util/descriptor.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the field descriptor for a given name from a descriptor.
 *
 * \warning Querying a descriptor by name is far less efficient than querying
 * by tag, as a lot more comparisons are involved.
 *
 * \param[in] descriptor Descriptor
 * \param[in] name[]     Name
 * \return               Field descriptor
 */
extern const pb_field_descriptor_t *
pb_descriptor_field_by_name(
    const pb_descriptor_t *descriptor, const char name[]) {
  assert(descriptor && name);
  for (size_t f = 0; f < descriptor->field.size; ++f) {
    if (strcmp(pb_field_descriptor_name(&(descriptor->field.data[f])), name))
      continue;
    return &(descriptor->field.data[f]);
  }
  return pb_descriptor_extension(descriptor) ?
    pb_descriptor_field_by_name(
      pb_descriptor_extension(descriptor), name) : NULL;
}

/* ------------------------------------------------------------------------- */

/*!
 * Retrieve the value descriptor for a given name from an enum descriptor.
 *
 * \warning Querying an enum descriptor by name is far less efficient than
 * querying by number, as a lot more comparisons are involved.
 *
 * \param[in] descriptor Enum descriptor
 * \param[in] name       Name
 * \return               Enum value descriptor
 */
extern const struct pb_enum_value_descriptor_t *
pb_enum_descriptor_value_by_name(
    const pb_enum_descriptor_t *descriptor, const char name[]) {
  assert(descriptor && name);
  for (size_t v = 0; v < descriptor->value.size; ++v) {
    if (strcmp(pb_enum_value_descriptor_name(
        &(descriptor->value.data[v])), name))
      continue;
    return &(descriptor->value.data[v]);
  }
  return NULL;
}
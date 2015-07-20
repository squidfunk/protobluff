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

#include "lib/field/descriptor.h"
#include "lib/message/descriptor.h"

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
 * Retrieve the field descriptor for a given tag from a message descriptor.
 *
 * We can leverage the fact that the fields are always in ascending order, as
 * the generator will generate it in this exact way, so using the tag number as
 * an array index, there are two scenarios:
 *
 * -# The tag number is smaller than the number of fields, so we can limit the
 *    fields we have to check to those left to the tag number array index. The
 *    respective field may then be at or left to the exact position.
 *
 * -# The tag number found at the array index is larger than the tag number
 *    we're looking for. In this case, we can abort the search.
 *
 * \warning The fields actually need to be in ascending order, so you better
 * ensure that or be pleasantly suprised by undefined behaviour.
 *
 * \param[in] descriptor Message descriptor
 * \param[in] tag        Tag
 * \return               Field descriptor
 */
extern const pb_field_descriptor_t *
pb_message_descriptor_field_by_tag(
    const pb_message_descriptor_t *descriptor, pb_tag_t tag) {
  assert(descriptor && tag);
  for (size_t f = min(tag, descriptor->field.size); f > 0; f--) {
    if (pb_field_descriptor_tag(
        &(descriptor->field.data[f - 1])) == tag) {
      return &(descriptor->field.data[f - 1]);
    } else if (pb_field_descriptor_tag(
        &(descriptor->field.data[f - 1])) < tag) {
      break;
    }
  }
  return NULL;
}

/*!
 * Retrieve the field descriptor for a given name from a message descriptor.
 *
 * \warning Querying a message descriptor by name is far less efficient than
 * querying by tag, as a lot more comparisons are involved.
 *
 * \param[in] descriptor Message descriptor
 * \param[in] name[]     Name
 * \return               Field descriptor
 */
extern const pb_field_descriptor_t *
pb_message_descriptor_field_by_name(
    const pb_message_descriptor_t *descriptor, const char name[]) {
  assert(descriptor && name);
  for (size_t f = 0; f < descriptor->field.size; f++) {
    if (strcmp(pb_field_descriptor_name(&(descriptor->field.data[f])), name))
      continue;
    return &(descriptor->field.data[f]);
  }
  return NULL;
}
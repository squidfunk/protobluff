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

#include <alloca.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "core/common.h"
#include "core/decoder.h"
#include "util/descriptor.h"
#include "util/validator.h"

/* ----------------------------------------------------------------------------
 * Forward declarations
 * ------------------------------------------------------------------------- */

static pb_error_t
check(
  const pb_decoder_t *decoder);        /* Decoder */

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_validator_tags_t {
  pb_tag_t *data;                      /*!< Tags */
  size_t size;                         /*!< Tag count */
} pb_validator_tags_t;

/* ----------------------------------------------------------------------------
 * Decoder callback
 * ------------------------------------------------------------------------- */

/*!
 * Field handler that counts occurrences.
 *
 * \param[in]     descriptor Field descriptor
 * \param[in]     value      Pointer holding value
 * \param[in,out] user       User data
 */
static pb_error_t
handler(
    const pb_field_descriptor_t *descriptor, const void *value, void *user) {
  assert(descriptor && value && user);
  pb_validator_tags_t *tags = user;

  /* Count field occurrence */
  uint8_t miss = 1;
  for (size_t t = 0; miss && t < tags->size; ++t)
    if (tags->data[t] == pb_field_descriptor_tag(descriptor))
      miss = 0;
  if (miss)
    tags->data[tags->size++] = pb_field_descriptor_tag(descriptor);

  /* In case of submessage, perform recursive check */
  return pb_field_descriptor_type(descriptor) == PB_TYPE_MESSAGE
    ? check(value)
    : PB_ERROR_NONE;
}

/* ----------------------------------------------------------------------------
 * Internal functions
 * ------------------------------------------------------------------------- */

/*!
 * Decode a buffer and count field occurrences.
 *
 * \param[in] decoder Decoder
 * \return            Error code
 */
static pb_error_t
check(const pb_decoder_t *decoder) {
  assert(decoder);
  assert(pb_decoder_valid(decoder));

  /* Initialize occurrence counter */
  pb_validator_tags_t tags = {
    .data = alloca(sizeof(pb_tag_t)
          * pb_descriptor_size(pb_decoder_descriptor(decoder))),
    .size = 0
  };

  /* Count occurrences using the given decoder */
  pb_error_t error = pb_decoder_decode(decoder, handler, &tags);

  /* Check for absent required fields */
  if (likely_(!error)) {
    pb_descriptor_iter_t it =
      pb_descriptor_iter_create(decoder->descriptor);
    if (pb_descriptor_iter_begin(&it)) {
      do {
        const pb_field_descriptor_t *descriptor =
          pb_descriptor_iter_current(&it);
        if (pb_field_descriptor_label(descriptor) == PB_LABEL_REQUIRED) {
          uint8_t miss = 1;
          for (size_t t = 0; miss && t < tags.size; ++t)
            if (tags.data[t] == pb_field_descriptor_tag(descriptor))
              miss = 0;
          if (miss)
            error = PB_ERROR_ABSENT;
        }
      } while (!error && pb_descriptor_iter_next(&it));
    }
    pb_descriptor_iter_destroy(&it);
  }
  return error;
}

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Validate a buffer.
 *
 * Fields do not necessarily occur in ascending order, so they are checked in
 * order of their appearance, unknown fields are skipped.
 *
 * \param[in] validator Validator
 * \param[in] buffer    Buffer
 * \return              Error code
 */
extern pb_error_t
pb_validator_check(
    const pb_validator_t *validator, const pb_buffer_t *buffer) {
  assert(validator && buffer);
  if (unlikely_(!pb_buffer_valid(buffer)))
    return PB_ERROR_INVALID;
  pb_decoder_t decoder = pb_decoder_create(validator->descriptor, buffer);
  pb_error_t   error   = check(&decoder);
  pb_decoder_destroy(&decoder);
  return error;
}
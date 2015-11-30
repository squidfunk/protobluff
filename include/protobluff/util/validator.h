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

#ifndef PB_INCLUDE_UTIL_VALIDATOR_H
#define PB_INCLUDE_UTIL_VALIDATOR_H

#include <assert.h>

#include <protobluff/core/buffer.h>
#include <protobluff/core/common.h>
#include <protobluff/core/descriptor.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_validator_t {
  const pb_descriptor_t *descriptor;   /*!< Descriptor */
} pb_validator_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_validator_check(
  const pb_validator_t *validator,     /* Validator */
  const pb_buffer_t *buffer);          /* Buffer */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Create a validator.
 *
 * \param[in] descriptor Descriptor
 * \return               Validator
 */
PB_WARN_UNUSED_RESULT
PB_INLINE pb_validator_t
pb_validator_create(const pb_descriptor_t *descriptor) {
  assert(descriptor);
  pb_validator_t validator = {
    .descriptor = descriptor
  };
  return validator;
}

/*!
 * Destroy a validator.
 *
 * \param[in,out] validator Validator
 */
PB_INLINE void
pb_validator_destroy(pb_validator_t *validator) {
  assert(validator); /* Nothing to be done */
}

/*!
 * Retrieve the descriptor of a validator.
 *
 * \param[in] validator Validator
 * \return              Descriptor
 */
PB_INLINE const pb_descriptor_t *
pb_validator_descriptor(const pb_validator_t *validator) {
  assert(validator);
  return validator->descriptor;
}

#endif /* PB_INCLUDE_UTIL_VALIDATOR_H */
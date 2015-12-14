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

#ifndef PB_INCLUDE_CORE_ENCODER_H
#define PB_INCLUDE_CORE_ENCODER_H

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include <protobluff/core/allocator.h>
#include <protobluff/core/buffer.h>
#include <protobluff/core/common.h>
#include <protobluff/core/descriptor.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef struct pb_encoder_t {
  const pb_descriptor_t *descriptor;   /*!< Descriptor */
  pb_buffer_t buffer;                  /*!< Buffer */
} pb_encoder_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_encoder_t
pb_encoder_create(
  const pb_descriptor_t *descriptor);  /* Descriptor */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_encoder_t
pb_encoder_create_with_allocator(
  pb_allocator_t *allocator,           /* Allocator */
  const pb_descriptor_t *descriptor);  /* Descriptor */

PB_EXPORT void
pb_encoder_destroy(
  pb_encoder_t *encoder);              /* Encoder */

PB_WARN_UNUSED_RESULT
PB_EXPORT pb_error_t
pb_encoder_encode(
  pb_encoder_t *encoder,               /* Encoder */
  pb_tag_t tag,                        /* Tag */
  const void *values,                  /* Pointer holding value(s) */
  size_t size);                        /* Value count */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the descriptor of an encoder.
 *
 * \param[in] encoder Encoder
 * \return            Descriptor
 */
PB_INLINE const pb_descriptor_t *
pb_encoder_descriptor(const pb_encoder_t *encoder) {
  assert(encoder);
  return encoder->descriptor;
}

/*!
 * Retrieve the buffer of an encoder.
 *
 * \param[in] encoder Encoder
 * \return            Buffer
 */
PB_INLINE const pb_buffer_t *
pb_encoder_buffer(const pb_encoder_t *encoder) {
  assert(encoder);
  return &(encoder->buffer);
}

/*!
 * Retrieve the internal error state of an encoder.
 *
 * \param[in] encoder Encoder
 * \return            Error code
 */
PB_INLINE pb_error_t
pb_encoder_error(const pb_encoder_t *encoder) {
  assert(encoder);
  return pb_buffer_error(&(encoder->buffer));
}

/*!
 * Test whether an encoder is valid.
 *
 * \param[in] encoder Encoder
 * \return            Test result
 */
PB_INLINE int
pb_encoder_valid(const pb_encoder_t *encoder) {
  assert(encoder);
  return !pb_encoder_error(encoder);
}

#endif /* PB_INCLUDE_CORE_ENCODER_H */
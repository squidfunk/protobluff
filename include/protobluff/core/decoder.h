/*
 * Copyright (c) 2013-2017 Martin Donath <martin.donath@squidfunk.com>
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

#ifndef PB_INCLUDE_CORE_DECODER_H
#define PB_INCLUDE_CORE_DECODER_H

#include <assert.h>

#include <protobluff/core/buffer.h>
#include <protobluff/core/common.h>
#include <protobluff/core/descriptor.h>

/* ----------------------------------------------------------------------------
 * Type definitions
 * ------------------------------------------------------------------------- */

typedef pb_error_t
(*pb_decoder_handler_f)(
  const pb_field_descriptor_t
    *descriptor,                       /*!< Field descriptor */
  const void *value,                   /*!< Pointer holding value */
  void *user);                         /*!< User data */

/* ------------------------------------------------------------------------- */

typedef struct pb_decoder_t {
  const pb_descriptor_t *descriptor;   /*!< Descriptor */
  const pb_buffer_t *buffer;           /*!< Buffer */
} pb_decoder_t;

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_decoder_t
pb_decoder_create(
  const pb_descriptor_t
    *descriptor,                       /* Descriptor */
  const pb_buffer_t *buffer);          /* Buffer */

PB_EXPORT void
pb_decoder_destroy(
  pb_decoder_t *decoder);              /* Decoder */

PB_EXPORT PB_WARN_UNUSED_RESULT
pb_error_t
pb_decoder_decode(
  const pb_decoder_t *decoder,         /* Decoder */
  pb_decoder_handler_f handler,        /* Handler */
  void *user);                         /* User data */

/* ----------------------------------------------------------------------------
 * Inline functions
 * ------------------------------------------------------------------------- */

/*!
 * Retrieve the descriptor of a decoder.
 *
 * \param[in] decoder Decoder
 * \return            Descriptor
 */
PB_INLINE const pb_descriptor_t *
pb_decoder_descriptor(const pb_decoder_t *decoder) {
  assert(decoder);
  return decoder->descriptor;
}

/*!
 * Retrieve the buffer of a decoder.
 *
 * \param[in] decoder Decoder
 * \return            Buffer
 */
PB_INLINE const pb_buffer_t *
pb_decoder_buffer(const pb_decoder_t *decoder) {
  assert(decoder);
  return decoder->buffer;
}

/*!
 * Retrieve the internal error state of a decoder.
 *
 * \param[in] decoder Decoder
 * \return            Error code
 */
PB_INLINE pb_error_t
pb_decoder_error(const pb_decoder_t *decoder) {
  assert(decoder);
  return pb_buffer_error(decoder->buffer);
}

/*!
 * Test whether a decoder is valid.
 *
 * \param[in] decoder Decoder
 * \return            Test result
 */
PB_INLINE int
pb_decoder_valid(const pb_decoder_t *decoder) {
  assert(decoder);
  return !pb_decoder_error(decoder);
}

#endif /* PB_INCLUDE_CORE_DECODER_H */

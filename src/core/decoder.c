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
#include <string.h>

#include "core/buffer.h"
#include "core/common.h"
#include "core/decoder.h"
#include "core/descriptor.h"
#include "core/stream.h"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

/*!
 * Create a decoder.
 *
 * \warning A decoder does not take ownership of the provided buffer, so the
 * caller must ensure that the buffer is not freed during operations.
 *
 * \param[in] descriptor Descriptor
 * \param[in] buffer     Buffer
 * \return               Decoder
 */
extern pb_decoder_t
pb_decoder_create(
    const pb_descriptor_t *descriptor, const pb_buffer_t *buffer) {
  assert(descriptor && buffer);
  pb_decoder_t decoder = {
    .descriptor = descriptor,
    .buffer     = buffer
  };
  return decoder;
}

/*!
 * Destroy a decoder.
 *
 * \param[in,out] decoder Decoder
 */
extern void
pb_decoder_destroy(pb_decoder_t *decoder) {
  assert(decoder); /* Nothing to be done */
}

/*!
 * Decode a buffer using a handler.
 *
 * The decoder
 *
 * \param[in]     decoder Decoder
 * \param[in]     handler Handler
 * \param[in,out] user    User data
 * \return                Error code
 */
extern pb_error_t
pb_decoder_decode(
    const pb_decoder_t *decoder, pb_decoder_handler_f handler, void *user) {
  assert(decoder && handler);
  if (unlikely_(!pb_decoder_valid(decoder)))
    return PB_ERROR_INVALID;
  pb_error_t error = PB_ERROR_NONE;

  /* Iterate tag-value pairs */
  pb_stream_t stream = pb_stream_create(decoder->buffer);
  while (!error && pb_stream_left(&stream)) {
    pb_tag_t tag;
    if (unlikely_(error = pb_stream_read(&stream, PB_TYPE_UINT32, &tag)))
      break;

    /* Extract wiretype and tag */
    pb_wiretype_t wiretype = tag & 7;
    tag >>= 3;

    /* Check descriptor and skip field if unknown */
    const pb_field_descriptor_t *descriptor =
      pb_descriptor_field_by_tag(decoder->descriptor, tag);
    if (unlikely_(!descriptor)) {
      error = pb_stream_skip(&stream, wiretype);
      continue;
    }

    /* Allocate temporary space for type-agnostic decoding */
    size_t item = pb_field_descriptor_type_size(descriptor);
    void *value = alloca(item);

    /* Non-packed fields may also be encoded in packed encoding */
    pb_type_t type = pb_field_descriptor_type(descriptor);
    if (wiretype != pb_field_descriptor_wiretype(descriptor) &&
        wiretype == PB_WIRETYPE_LENGTH) {
      uint32_t length;
      if (unlikely_(error = pb_stream_read(&stream, PB_TYPE_UINT32, &length)))
        break;

      /* Ensure we're within the stream's boundaries */
      size_t offset = pb_stream_offset(&stream);
      if (pb_stream_offset(&stream) + length > pb_buffer_size(decoder->buffer))
        error = PB_ERROR_OFFSET;

      /* Iterate values of packed field */
      while (!error && pb_stream_offset(&stream) < offset + length)
        if (likely_(!(error = pb_stream_read(&stream, type, value))))
          error = handler(descriptor, value, user);

    /* Read value of given type from stream */
    } else {
      if (unlikely_(error = pb_stream_read(&stream, type, value)))
        break;

      /* Invoke handler */
      if (pb_field_descriptor_type(descriptor) == PB_TYPE_MESSAGE) {
        pb_buffer_t buffer = pb_buffer_create_zero_copy_internal(
          pb_string_data(value), pb_string_size(value));

        /* Create decoder for nested message and invoke handler */
        pb_decoder_t subdecoder = pb_decoder_create(
          pb_field_descriptor_reference(descriptor), &buffer);
        error = handler(descriptor, &subdecoder, user);

        /* Free all allocated memory */
        pb_decoder_destroy(&subdecoder);
        pb_buffer_destroy(&buffer);
      } else {
        error = handler(descriptor, value, user);
      }
    }
  }
  pb_stream_destroy(&stream);
  return error;
}
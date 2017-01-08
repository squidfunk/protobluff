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

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "person.pb.h"

/* ----------------------------------------------------------------------------
 * Decoder callbacks
 * ------------------------------------------------------------------------- */

/*!
 * Handler for phonenumber messages.
 *
 * If the tag numbers of the .proto file are ascending and there are a lot
 * of tags, it probably best to use a jump table.
 *
 * \param[in]     descriptor Field descriptor
 * \param[in]     value      Pointer holding value
 * \param[in,out] user       User data
 */
static pb_error_t
handler_phonenumber(
    const pb_field_descriptor_t *descriptor, const void *value, void *user) {
  assert(descriptor && value);
  pb_error_t error = PB_ERROR_NONE;
  switch (pb_field_descriptor_tag(descriptor)) {

    /* Print number */
    case PERSON_PHONENUMBER_NUMBER_T:
      printf("%.*s",
        (int)pb_string_size(value), pb_string_data(value));
      break;

    /* Print type */
    case PERSON_PHONENUMBER_TYPE_T:
      printf(" (%s)", pb_enum_value_descriptor_name(
        pb_enum_descriptor_value_by_number(
          pb_field_descriptor_enum(descriptor),
          *(const pb_enum_t *)value)));
      break;
  }
  return error;
}

/*!
 * Handler for person messages.
 *
 * \param[in]     descriptor Field descriptor
 * \param[in]     value      Pointer holding value
 * \param[in,out] user       User data
 */
static pb_error_t
handler_person(
    const pb_field_descriptor_t *descriptor, const void *value, void *user) {
  assert(descriptor && value);
  pb_error_t error = PB_ERROR_NONE;
  switch (pb_field_descriptor_tag(descriptor)) {

    /* Print name */
    case PERSON_NAME_T:
      printf("name:  %.*s\n",
        (int)pb_string_size(value), pb_string_data(value));
      break;

    /* Print id */
    case PERSON_ID_T:
      printf("id:    %d\n", *(const int32_t *)value);
      break;

    /* Print email */
    case PERSON_EMAIL_T:
      printf("email: %.*s\n",
        (int)pb_string_size(value), pb_string_data(value));
      break;

    /* Print phone numbers */
    case PERSON_PHONE_T:
      printf("phone: ");
      error = pb_decoder_decode(value, handler_phonenumber, user);
      printf("\n");
      break;
  }
  return error;
}

/* ----------------------------------------------------------------------------
 * Program
 * ------------------------------------------------------------------------- */

/*!
 * Decode a person message.
 *
 * \return Exit code
 */
int
main(void) {

  /* Wire-encoded person message */
  const uint8_t data[] = {
     10,   8,  74, 111, 104, 110,  32,  68, 111, 101,
     16, 210,   9,  26,  16, 106, 100, 111, 101,  64,
    101, 120,  97, 109, 112, 108, 101,  46,  99, 111,
    109,  34,  19,  10,  15,  43,  49,  45,  53,  52,
     49,  45,  55,  53,  52,  45,  51,  48,  49,  48,
     16,   1,  34,  19,  10,  15,  43,  49,  45,  53,
     52,  49,  45,  50,  57,  51,  45,  56,  50,  50,
     56,  16,   0
  };

  /* Create a zero-copy buffer and dump it */
  pb_buffer_t buffer = pb_buffer_create_zero_copy(
    (uint8_t *)data, sizeof(data));
  pb_buffer_dump(&buffer);

  /* Create a person decoder and run decoder */
  pb_decoder_t person = person_decoder_create(&buffer);
  pb_error_t   error  = person_decode(&person, handler_person, NULL);

  /* Print error, if any */
  if (error)
    fprintf(stderr, "ERROR: %s\n", pb_error_string(error));

  /* Free all allocated memory and return */
  person_decoder_destroy(&person);
  pb_buffer_destroy(&buffer);
  return error
    ? EXIT_FAILURE
    : EXIT_SUCCESS;
}

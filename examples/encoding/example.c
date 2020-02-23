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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "person.pb.h"

/* ----------------------------------------------------------------------------
 * Program
 * ------------------------------------------------------------------------- */

/*!
 * Encode a person message and dump it.
 *
 * This is one of two examples (together with the messages example) showing
 * how to build a Protocol Buffers message with protobluff. While this approach
 * is significantly faster since it doesn't have to realign the message after
 * every update, it is not possible to update values after they were written,
 * which is easily done with the other approach. For optional and required
 * fields that are written multiple times only the last occurrence will be
 * reported when decoding the Protocol Buffers message, as demanded by the
 * Protocol Buffers specification.
 *
 * \return Exit code
 */
int
main(void) {

  /* Create a person encoder */
  pb_encoder_t person = person_encoder_create();

  /* Define the values we want to set */
  pb_string_t name   = pb_string_init_from_chars("John Doe"),
              email  = pb_string_init_from_chars("jdoe@example.com"),
              home   = pb_string_init_from_chars("+1-541-754-3010"),
              mobile = pb_string_init_from_chars("+1-541-293-8228");
  int32_t     id     = 1234;

  /* Encode values for person message and check return codes */
  pb_error_t error = PB_ERROR_NONE;
  do {
    if ((error = person_encode_name(&person, &name)) ||
        (error = person_encode_id(&person, &id)) ||
        (error = person_encode_email(&person, &email)))
      break;

    /* Set home and mobile number */
    pb_encoder_t phone[] = {
      person_phonenumber_encoder_create(),
      person_phonenumber_encoder_create()
    };
    if (!(error = person_phonenumber_encode_number(&(phone[0]), &home)) &&
        !(error = person_phonenumber_encode_type_home(&(phone[0]))) &&
        !(error = person_phonenumber_encode_number(&(phone[1]), &mobile)) &&
        !(error = person_phonenumber_encode_type_mobile(&(phone[1]))) &&
        !(error = person_encode_phone(&person, phone, 2))) {

      /* Dump the internal buffer */
      const pb_buffer_t *buffer = pb_encoder_buffer(&person);
      pb_buffer_dump(buffer);

      /* The encoded message can be accessed as follows */
      // const uint8_t *data = pb_buffer_data(buffer);
      // const size_t   size = pb_buffer_size(buffer);
    }
    person_phonenumber_encoder_destroy(&(phone[0]));
    person_phonenumber_encoder_destroy(&(phone[1]));
  } while (0);

  /* Print error, if any */
  if (error)
    fprintf(stderr, "ERROR: %s\n", pb_error_string(error));

  /* Free all allocated memory and return */
  person_encoder_destroy(&person);
  return error
    ? EXIT_FAILURE
    : EXIT_SUCCESS;
}
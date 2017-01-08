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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "person.pb.h"

/* ----------------------------------------------------------------------------
 * Program
 * ------------------------------------------------------------------------- */

/*!
 * Create a person message and dump it.
 *
 * This is not the fastest approach for building messages from the ground up.
 * When setting fields one-by-one, protobluff will determine the offset of a
 * field and adjust the length prefixes of the wrapping messages everytime.
 * If speed is a concern consider using an encoder instead. However, when
 * setting or retrieving single fields from a message this is by far the
 * fastest approach, as only the necessary parts of the message have to be
 * decoded, not the whole message. Additionally dynamic memory allocation
 * can be completley omitted.
 *
 * \return Exit code
 */
int
main(void) {

  /* Create an empty journal to assemble a new person message */
  pb_journal_t journal = pb_journal_create_empty();

  /* Create a person message */
  pb_message_t person = person_create(&journal);

  /* Define the values we want to set */
  pb_string_t name   = pb_string_init_from_chars("John Doe"),
              email  = pb_string_init_from_chars("jdoe@example.com"),
              home   = pb_string_init_from_chars("+1-541-754-3010"),
              mobile = pb_string_init_from_chars("+1-541-293-8228");
  int32_t     id     = 1234;

  /* Set values on person message and check return codes */
  pb_error_t error = PB_ERROR_NONE;
  do {
    if ((error = person_put_name(&person, &name)) ||
        (error = person_put_id(&person, &id)) ||
        (error = person_put_email(&person, &email)))
      break;

    /* Set home number */
    pb_message_t phone1 = person_create_phone(&person);
    if (!(error = person_phonenumber_put_number(&phone1, &home)) &&
        !(error = person_phonenumber_put_type_home(&phone1))) {

      /* Set mobile number */
      pb_message_t phone2 = person_create_phone(&person);
      if (!(error = person_phonenumber_put_number(&phone2, &mobile)) &&
          !(error = person_phonenumber_put_type_mobile(&phone2))) {

        /* Dump the journal */
        pb_journal_dump(&journal);

        /* The encoded message can be accessed as follows */
        // const uint8_t *data = pb_journal_data(&journal);
        // const size_t   size = pb_journal_size(&journal);
      }
      person_phonenumber_destroy(&phone2);
    }
    person_phonenumber_destroy(&phone1);
  } while (0);

  /* Print error, if any */
  if (error)
    fprintf(stderr, "ERROR: %s\n", pb_error_string(error));

  /* Cleanup and invalidate */
  person_destroy(&person);

  /* Free all allocated memory and return */
  pb_journal_destroy(&journal);
  return error
    ? EXIT_FAILURE
    : EXIT_SUCCESS;
}
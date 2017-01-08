## Full runtime

### Overview

The full runtime includes the logic of the [lite runtime](#lite-runtime) and
adds support for partial reads and writes of messages. It weighs around 41kb
(`-O3`, stripped) or respectively 32kb (`-Os`, stripped) when compiled on
x86_64.

### Linking

The canonical name of the full runtime is **protobluff**. The following
compiler and linker flags must be obtained and added to your build toolchain:

``` sh
pkg-config --cflags protobluff # Add output to compiler flags
pkg-config --libs   protobluff # Add output to linker flags
```

With [Autotools][], the `PKG_CHECK_MODULES` macro can be used within your
`configure.ac`, which will check for the full runtime and define the flags
`protobluff_CFLAGS` and `protobluff_LDFLAGS`:

``` makefile
PKG_CHECK_MODULES([protobluff], [protobluff])
```

### Usage

Here's a [usage example][Protocol Buffers Example] taken from the original
description of the Google Protocol Buffers library and adapted to protobluff's
full runtime:

```c
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
```

## Lite runtime

### Overview

The lite runtime includes the bare minimum of logic to decode and encode
Protocol Buffers messages. It's most suitable for environments with constrained
resources, as it only weighs around 17kb (`-O3`, stripped) or respectively 13kb
(`-Os`, stripped) when compiled on x86_64. Though not being the highest
priority task on the list, compatibility with embedded environments is planned.

The lite runtime enables the decoding and encoding of whole messages. However,
it has no explicit support for oneofs and message validation.

### Linking

The canonical name of the lite runtime is **protobluff-lite**. The following
compiler and linker flags must be obtained and added to your build toolchain:

``` sh
pkg-config --cflags protobluff-lite # Add output to compiler flags
pkg-config --libs   protobluff-lite # Add output to linker flags
```

With [Autotools][], the `PKG_CHECK_MODULES` macro can be used within your
`configure.ac`, which will check for the lite runtime and define the flags
`protobluff_CFLAGS` and `protobluff_LDFLAGS`:

``` makefile
PKG_CHECK_MODULES([protobluff], [protobluff-lite])
```

### Usage

Here's a [usage example][Protocol Buffers Example] taken from the original
description of the Google Protocol Buffers library and adapted to protobluff's
lite runtime:

``` c
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
```

[Autotools]: http://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html
[Protocol Buffers Example]: https://developers.google.com/protocol-buffers/docs/overview#how-do-they-work

## Installation

### Building from source

protobluff is built using [Autotools](
  http://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html
) and can be linked as a static or shared library. It has no runtime
dependencies and is fully self-contained, except for the code generator which
depends on the original Protocol Buffers library and is necessary to generate
bindings from `.proto` schema files. If the original library is not available,
the generator is not built. The following commands build and install the
protobluff library and code generator:

``` sh
./autogen.sh &&
./configure &&
make &&
make test &&
make install
```

protobluff should compile and run on all UNIX systems (including Linux and Mac
OS) as it adheres to the C99 and C++98 standards and does not make use of any
system-specific functionality.

After installing protobluff, the code generator can be used to generate
bindings from `.proto` schema files to get started. See
[this section](#using-the-code-generator) for more information.

### Additional options

By default, protobluff is compiled aggressively optimized with `-O3` and some
further optimizations which make it nearly impossible to debug. If debugging
is necessary, optimizations should be disabled. Stripped compilation will
remove all symbols that are not defined in the public header files, allowing
further optimizations. Enabling the coverage report is only necessary to
determine unit test coverage, and thus only needed during development.

``` sh
./configure
  --disable-optimized # No optimizations (default: enabled)
  --enable-stripped   # Strip internal symbols (default: disabled)
  --enable-coverage   # Coverage report (default: disabled)
```

The tests can only be built if stripped compilation is not enabled, as no
internal symbols would be visible to the unit tests.

## Using the code generator

The code generator is tightly integrated with the `protoc` compiler toolchain
included in the default Protocol Buffers distribution. Use the `protoc` command
to invoke the protobluff code generator through the `--protobluff_out` flag,
to generate and write the respective `.pb.h` and `.pb.c` files to a specific
location:

``` sh
protoc --protobluff_out=. *.proto
```

The `.pb.h` header files contain the bindings, the `.pb.c` source files contain
the descriptor definitions and defaults which are referenced by the bindings.
Therefore, the source files must be compiled together with your project.

## Using the generated bindings

Here's a [usage example](
  https://developers.google.com/protocol-buffers/docs/overview#how-do-they-work
) taken from the original description of the Google Protocol Buffers library
and adapted to protobluff:

``` c
/* Create an empty binary to assemble a new person message */
pb_binary_t binary = pb_binary_create_empty();

/* Create a person message */
pb_message_t person = person_create(&binary);

/* Define the values we want to set */
pb_string_t name   = pb_string_init("John Doe"),
            email  = pb_string_init("jdoe@example.com"),
            home   = pb_string_init("+1-541-754-3010"),
            mobile = pb_string_init("+1-541-293-8228");
int32_t     id     = 1234;

/* Set values on person message and check return codes */
pb_error_t error;
do {
  if ((error = person_put_name(&person, &name)) ||
      (error = person_put_email(&person, &email)) ||
      (error = person_put_id(&person, &id)))
    break;

  /* Set home number */
  pb_message_t phone1 = person_create_phone(&person);
  if ((error = person_phonenumber_put_type_home(&phone1)) ||
      (error = person_phonenumber_put_number(&phone1, &home)))
    break;

  /* Set mobile number */
  pb_message_t phone2 = person_create_phone(&person);
  if ((error = person_phonenumber_put_type_mobile(&phone2)) ||
      (error = person_phonenumber_put_number(&phone2, &mobile)))
    break;

  /* All values were set successfully, the binary is ready to be persisted,
     sent or whatever - no marshalling necessary. The raw message data and
     size can be directly obtained through the binary */
  const uint8_t *data = pb_binary_data(&binary);
  const size_t   size = pb_binary_size(&binary);
} while (0);

/* Check error */
if (error)
  fprintf(stderr, "Error: %s", pb_error_string(error));

/* Cleanup and invalidate */
person_destroy(&person);

/* Free all allocated memory */
pb_binary_destroy(&binary);
```

At first, the name, email and id of the person are encoded, and afterwards
the two phone numbers. However, the order of operations is not important.
protobluff will ensure that the fields are written in the right place, so we
could also encode the name, then create the mobile phone number again, add the
email and id, change the phone number, and add the home phone number last.
The result will be the same.

## Linking

### Manually

For the generated bindings to function, your project must be linked against the
protobluff runtime. The recommended way is to dynamically link the shared
library. Therefore, the following compiler and linker flags must be obtained
and added to your build toolchain:

``` sh
pkg-config --cflags libprotobluff # Add output to compiler flags
pkg-config --libs   libprotobluff # Add output to linker flags
```

### Autotools

If you're using Autotools, the `PKG_CHECK_MODULES` macro will take care of the
heavy lifting. Adding the following line to your `configure.ac` file will place
the compiler flags into the variable `protobluff_CFLAGS` and the linker flags
into the variable `protobluff_LDFLAGS`:

``` makefile
PKG_CHECK_MODULES([protobluff], [libprotobluff])
```
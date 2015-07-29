# protobluff

[![Build Status](https://travis-ci.org/squidfunk/protobluff.svg)](https://travis-ci.org/squidfunk/protobluff)
[![Coverage Status](https://coveralls.io/repos/squidfunk/protobluff/badge.svg?branch=master&service=github)](https://coveralls.io/github/squidfunk/protobluff?branch=master)
[![Release Status](https://img.shields.io/github/release/squidfunk/protobluff.svg)](https://github.com/squidfunk/protobluff/releases/latest)
[![License](https://img.shields.io/github/license/squidfunk/protobluff.svg)](https://github.com/squidfunk/protobluff/blob/master/LICENSE)

protobluff is an extremely lightweight Protocol Buffers implementation for C.

## Theory of Operation

[Protocol Buffers][] is a language-neutral, platform-neutral and extensible
message format developed by Google for serializing structured data. It uses
schema files to describe the structure of messages, which are in turn used to
generate language-specific bindings to automatically handle the decoding and
encoding for the developer. However, as messages can have a variable amount
of repeated submessages and fields, decoding and encoding may involve a large
number of scattered allocations which in turn is not very cache-friendly.

protobluff follows a different approach. It entirely skips the necessary
decoding and encoding steps when reading or writing values from messages,
as it directly operates on the encoded binary. New values can be incrementally
read or written, memory management is centralized and handled by the underlying
binary. If no alterations that change the size of the underlying binary are
expected, the binary can be used in *zero-copy mode*, omitting all dynamic
allocations.

Updates on fixed-sized wire types on little-endian machines can be carried out
in-place using raw-pointers to the underlying binary. These include the native
Protocol Buffers types `fixed(32|64)`, `sfixed(32|64)`, `float` and `double`
(see the [Protocol Buffers Encoding Guide][] for more information). Strings may
also be accessed through raw-pointers, however writing a string of different
length may result in garbled data, and is thus not recommended.

## Installation

### Building from source

protobluff is built using [Autotools][] and can be linked as a static or shared
library. It has no runtime dependencies and is fully self-contained, except for
the code generator which depends on the original Protocol Buffers library and
is necessary to generate bindings from `.proto` schema files. If the original
library is not available, the generator is not built. The following commands
build and install the protobluff library and code generator:

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
is necessary, one should disable optimizations. Stripped compilation will
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

The code generator is tightly integrated with the protoc compiler toolchain
included in the default Protocol Buffers distribution. Use the `protoc` command
to invoke the protobluff code generator through the `--protobluff_out` flag,
to generate and write the respective `.pb.c` and `.pb.h` files to a specific
location:

``` sh
protoc --protobluff_out=. *.proto
```

The `.pb.h` header files will contain the bindings, the `.pb.c` source files
contain the descriptor definitions which are referenced by the bindings.
Therefore, the source files must be compiled together with your project.

## Using the generated bindings

Here's a usage example taken from the original description of the Google
Protocol Buffers library and adapted to protobluff:

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
pb_error_t error = PB_ERROR_NONE;
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
     sent or whatever - no marshalling necessary. The raw message data and size
     can be directly obtained through the binary */
  const uint8_t *data = pb_binary_data(&binary);
  const size_t   size = pb_binary_size(&binary);
} while (0);

/* Cleanup and invalidate */
person_destroy(&person);

/* Free all allocated memory */
pb_binary_destroy(&binary);
```

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

## Features

### Already supported

1. Message definitions
2. Submessage definitions
3. All scalar types
4. Enumerations
5. Strings and binaries
6. Optional, required and repeated fields
7. Imports
8. Packages

### Not yet supported

1. Circular (sub)message definitions
2. Deprecation warnings
3. Message extensions
4. Packed fields
5. RPC (probably as an extensions to protobluff)
6. General proto3 support

## License

Copyright (c) 2013-2015 Martin Donath

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

[Protocol Buffers]: https://developers.google.com/protocol-buffers/docs/overview
[Protocol Buffers Encoding Guide]: https://developers.google.com/protocol-buffers/docs/encoding
[Autotools]: http://www.gnu.org/software/automake/manual/html_node/Autotools-Introduction.html
[Valgrind]: http://valgrind.org/
[LCOV]: http://ltp.sourceforge.net/coverage/lcov.php
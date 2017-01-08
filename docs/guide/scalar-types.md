## Overview

The Protocol Buffers specification defines a multitude of signed and unsigned
integer types (each with different implications for the underlying wire
representation), as well as types for the representation of floating point
numbers, booleans, bytes and strings.

## Type mappings

The following table shows the type mappings from C types (both native and
custom types like `pb_string_t`) to Protocol Buffer schema types:

| Native type   | Schema type                  |
|---------------|------------------------------|
| `int32_t`     | `int32`, `sint32`, `fixed32` |
| `int64_t`     | `int64`, `sint64`, `fixed64` |
| `uint8_t`     | `bool`                       |
| `uint32_t`    | `uint32`, `fixed32`          |
| `uint64_t`    | `uint64`, `fixed64`          |
| `float`       | `float`                      |
| `double`      | `double`                     |
| `pb_enum_t`   | `enum`                       |
| `pb_string_t` | `bytes`, `string`            |

See the [Protocol Buffers specification][] for more information on types.

## Strings and byte arrays

While the mappings of integer and floating point types should be quite
self-explanatory, strings are worth discussing: in wire representation, strings
and byte arrays are stored as a length-prefixed, unterminated list of unsigned
chars. Because there's no nul-terminator, the length needs to be explicitly
returned. This is what the datatype `pb_string_t` is for. The header file
`core/string.h` defines a few useful functions for working with strings:

``` c
/* Initialize a string with explicit length */
pb_string_t string = pb_string_init("TEST", 4);

/* Initialize a string from a nul-terminated array of unsigned chars */
pb_string_t string = pb_string_init_from_chars("TEST");

/* Initialize an empty string - may be useful at times */
pb_string_t string = pb_string_init_empty();

/* Retrieve the underlying data and size of a string */
const uint8_t *data = pb_string_data(&string);
const size_t   size = pb_string_size(&string);
```

Strings are always allocated on the stack and do not take ownership of the
provided array of unsigned chars, so there's no need for freeing them, but the
caller must ensure that the array is not freed during operations.

[Protocol Buffers specification]: https://developers.google.com/protocol-buffers/docs/proto?hl=en#scalar

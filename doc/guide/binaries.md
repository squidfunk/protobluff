## Overview

protobluff directly operates on the wire representation of a message, so reads
and writes can be carried out without decoding and re-encoding the whole
binary. When altering the underlying binary through writes to fields,
protobluff tracks all changes in a journal, so pointers to different messages
within the same binary remain valid, even though field offsets may change.

Binaries are the single point of allocation inside protobluff, which means
the developer is only responsible for creating and destroying the binary. This
softly mitigates the pain of memory management, which is common in some other
Protocol Buffers implementations for the C language. All other structures
inside protobluff are allocated on the stack.

``` c
pb_binary_t binary = pb_binary_create_empty();
if (!pb_binary_valid(&binary)) {
  fprintf(stderr, "Error: %s", pb_error_string(pb_binary_error(&binary)));
} else {
  ...
  const uint8_t *data = pb_binary_data(&binary);
  const size_t   size = pb_binary_size(&binary);
  ...
}
pb_binary_destroy(&binary);
```

## Creating a binary

An encoded Protocol Buffers message is represented as an array of bytes of
a certain size:

``` c
uint8_t data[] = ...
size_t  size   = ...
```

In order to decode and work with this Protocol Buffers message, a protobluff
binary must be created with the encoded message's raw data and size:

``` c
pb_binary_t binary = pb_binary_create(data, size);
```

protobluff does not take ownership of the original binary, but copies the data
to an internally allocated buffer. This is necessary, because writes to fields
of the message may alter the length of the externally allocated array of bytes.

## Creating a zero-copy binary

If the original data is not expected to be altered, a zero-copy binary can be
created. protobluff will assume that the externally allocated buffer is not
freed during operations:

``` c
pb_binary_t binary = pb_binary_create_zero_copy(data, size);
```

All operations that do not impact the length of the binary will succeed and
change the original encoded Protocol Buffers message in-place, e.g. all read
operations and write operations that only change the contents, but not the
length of a field. Updates on fixed-sized wire types on little-endian machines
can be carried out in-place. These include the native Protocol Buffers types
`fixed(32|64)`, `sfixed(32|64)`, `float` and `double` (see the
[Protocol Buffers Encoding Guide](
  https://developers.google.com/protocol-buffers/docs/encoding
) for more information).

Alterations that change the length of the binary will fail and return
`PB_ERROR_ALLOC`, as protobluff will not (and can not) resize the externally
allocated buffer.

## Creating an empty binary

If no binary data is given, e.g. when a new Protocol Buffers message should be
constructed from scratch, an empty binary can be created:

``` c
pb_binary_t binary = pb_binary_create_empty();
```

## Freeing a binary

When finished working with the underlying message, the binary must be
destroyed explicitly, so the internally allocated space can be freed:

``` c
pb_binary_destroy(&binary);
```

Zero-copy binaries should also always be destroyed for reasons of consistency,
even if, at the time of writing, no internal allocations are taking place, as
this may be subject to change in the future.

## Error handling

Afar from zero-copy binaries, creation of regular binaries can theoretically
fail due to an out-of-memory condition (`ENOMEM`). protobluff is designed to
fully recover from such conditions, passing the error back to the caller after
cleaning up any prior allocations. For this reason, the caller should always
validate whether binary creation was successful:

``` c
pb_binary_t binary = pb_binary_create_empty();
if (!pb_binary_valid(&binary))
  fprintf(stderr, "Error: %s", pb_error_string(pb_binary_error(&binary)));
```

If the caller doesn't validate the binary after creation and the binary is
not valid, all subsequent calls on messages, cursors and fields will return
errors (see the section on [error handling](/guide/error-handling/) for more
details).

## Accessors

The binary may not be altered directly, only by invoking the generated Protocol
Buffer message accessors or the low-level interface. The raw data and size
underlying the binary can be extracted with the following methods at any time:

``` c
const uint8_t *data = pb_binary_data(&binary);
const size_t   size = pb_binary_size(&binary);
```

## Custom allocators

protobluff is designed to allow [custom allocator](/guide/allocators/)
implementations, facilitating a more fine grained control of how memory is
layed out. The default allocator is just a wrapper around the POSIX functions
`malloc`, `realloc` and `free`. Alternative implementations may yield better
performance in specific cases, like chunk allocators or fixed-size allocators.
A binary can be easily created together with a custom allocator:

``` c
pb_binary_t binary = pb_binary_create_with_allocator(&allocator, data, size);
```

The same holds for an empty binary:

``` c
pb_binary_t binary = pb_binary_create_empty_with_allocator(&allocator);
```

For guidance on this topic, see the documentation on
[custom allocators](/guide/allocators/).
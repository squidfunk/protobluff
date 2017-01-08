## Overview

protobluff directly operates on the wire representation of a message, so reads
and writes can be carried out without decoding and re-encoding the whole
message.

In order to work with a message, the raw data must be wrapped inside a buffer
for the lite runtime and inside a journal for the full runtime. Buffers are the
single point of allocation inside protobluff, which means the developer is only
responsible for creating and destroying the buffer (or respectively journal).
This mitigates the pain of memory management, which is common in some other
Protocol Buffers implementations for the C language. All other structures
inside protobluff are allocated on the stack.

Journals are just buffers on steroids - they track every change made. This is
crucial when working with more than one pointer (field, submessage or cursor)
to a message, especially in the case of partial updates, as those pointers may
get invalidated by those updates, e.g. if field offsets change.

``` c
pb_buffer_t buffer = pb_buffer_create_empty();
if (!pb_buffer_valid(&buffer)) {
  fprintf(stderr, "Error: %s", pb_error_string(pb_buffer_error(&buffer)));
} else {
  ...
  const uint8_t *data = pb_buffer_data(&buffer);
  const size_t   size = pb_buffer_size(&buffer);
  ...
}
pb_buffer_destroy(&buffer);
```

Buffers and journals share the same interface, as journals are just wrappers
around buffers, so within the following function calls, `buffer` may be
substituted for `journal` to get the equivalent function call.

## Creating a buffer

An encoded Protocol Buffers message is represented as an array of bytes of
a certain size:

``` c
uint8_t data[] = ...
size_t  size   = ...
```

In order to decode and work with this Protocol Buffers message, a protobluff
buffer (or respectively journal) must be created with the encoded message's raw
data and size:

``` c
pb_buffer_t buffer = pb_buffer_create(data, size);
```

protobluff does not take ownership of the original buffer, but copies the data
to an internally allocated buffer. This is necessary, because writes to fields
of the message may alter the length of the externally allocated array of bytes.

## Creating a zero-copy buffer

If the original data is not expected to be altered, a zero-copy buffer can be
created. protobluff will assume that the externally allocated buffer is not
freed during operations:

``` c
pb_buffer_t buffer = pb_buffer_create_zero_copy(data, size);
```

All operations that do not impact the length of the buffer will succeed and
change the original encoded Protocol Buffers message in-place, e.g. all read
operations and write operations that only change the contents, but not the
length of a field. Alterations that change the length of the buffer will fail
and return `PB_ERROR_ALLOC`, as protobluff will not (and cannot) resize the
externally allocated buffer.

## Creating an empty buffer

If no buffer data is given, e.g. when a new Protocol Buffers message should be
constructed from scratch, an empty buffer can be created:

``` c
pb_buffer_t buffer = pb_buffer_create_empty();
```

## Freeing a buffer

When finished working with the underlying message, the buffer must be
destroyed explicitly, so the internally allocated space can be freed:

``` c
pb_buffer_destroy(&buffer);
```

Zero-copy buffers should also always be destroyed for reasons of consistency,
even if, at the time of writing, no internal allocations are taking place, as
this may be subject to change in the future.

## Error handling

Afar from zero-copy buffers, creation of regular buffers can theoretically
fail due to an out-of-memory condition (`ENOMEM`). protobluff is designed to
fully recover from such conditions, passing the error back to the caller after
cleaning up any prior allocations. For this reason, the caller should always
validate whether buffer creation was successful:

``` c
pb_buffer_t buffer = pb_buffer_create_empty();
if (!pb_buffer_valid(&buffer))
  fprintf(stderr, "Error: %s", pb_error_string(pb_buffer_error(&buffer)));
```

If the caller doesn't validate the buffer after creation and the buffer is
not valid, all subsequent calls on messages, cursors and fields will return
errors (see the section on [error handling](/guide/error-handling/) for more
details).

## Accessors

The buffer may not be altered directly, only by invoking the generated Protocol
Buffer message accessors or the low-level interface. The raw data and size
underlying the buffer can be extracted with the following methods at any time:

``` c
const uint8_t *data = pb_buffer_data(&buffer);
const size_t   size = pb_buffer_size(&buffer);
```

## Custom allocators

protobluff is designed to allow [custom allocator](/guide/allocators/)
implementations, facilitating a more fine grained control of how memory is
layed out. The default allocator is just a wrapper around the POSIX functions
`malloc`, `realloc` and `free`. Alternative implementations may yield better
performance in specific cases, like chunk allocators or fixed-size allocators.
A buffer can be easily created together with a custom allocator:

``` c
pb_buffer_t buffer = pb_buffer_create_with_allocator(&allocator, data, size);
```

The same holds for an empty buffer:

``` c
pb_buffer_t buffer = pb_buffer_create_empty_with_allocator(&allocator);
```

For guidance on this topic, see the documentation on
[custom allocators](/guide/allocators/).

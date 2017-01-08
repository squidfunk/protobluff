## Overview

From a paranoid point of view, there's a lot that can go wrong when working
with Protocol Buffers messages, e.g. allocation errors, buffer underruns or
garbled data. For this reason, protobluff is designed to anticipate and catch
all possible errors and to be very specific about what went wrong.

protobluff will always handle erroneous messages gracefully by design. If a
buffer with an allocation error is passed to a message constructor, the message
will be flagged invalid. Next, if a field to or from this invalid message is
written or read, the resulting return code will indicate the same. The error
will gracefully propagate through all subsequent function calls. However, to
catch errors and unexpected behavior, the validity of the created structures
and return values should always be checked explicitly.

Errors are represented through codes defined in the type `pb_error_t` in the
public header file `core/common.h`. For a detailed explanation of the specific
error codes see [this section](#error-codes).

## Errors in return values

All functions that return the type `pb_error_t` are meant to be encapsulated
in conditional clauses. The compiler will be very whiny if this is forgotten,
thanks to the custom compiler attribute `__warn_unused_result__` that is
defined for the respective functions if supported by the compiler (Clang and
GCC have had this for a long time). Though it is not recommended, this
behavior can be deactivated by compiling the linking program with the flag
`-Wno-unused-result`.

If the call to a function is successful, `PB_ERROR_NONE` is returned which is
set to the value `0`, POSIX-style. All other error codes have values greater
than zero. This allows easy checks for error conditions in two levels of
verbosity. If no further details regarding the error are necessary, checking
for errors is as simple as:

``` c
if (pb_validator_check(&validator, &buffer)) {
  /* Nope, there was an error */
}
```

When more information on the type of the error is needed, the following idiom
should be applied:

``` c
pb_error_t error;
if ((error = pb_validator_check(&validator, &buffer))) {
  /* Nope, there was an error */
}
```

The function `pb_error_string` is similar to the POSIX function `strerror`
that returns a human-readable representation of an error code:

``` c
fprintf(stderr, "Error: %s", pb_error_string(error));
```

## Errors upon creation

When creating new structures, protobluff makes heavy use of the stack where
possible to minimize the need for dynamic allocations:

``` c
pb_buffer_t buffer = pb_buffer_create_empty();
```

Usually, libraries that use dynamic allocation and return pointers implement
error conditions by returning `NULL` pointers. However, to get further details
on the type of error an extra argument to receive the type of error would have
to be passed to the function call or a global variable like `errno` would have
to be used. As protobluff uses stack allocation wherever possible, a different
approach has to be taken.

For this reason, errors that happen upon creation are encoded into the
resulting structure, which has two advantages opposed to passing an argument to
receive the error: first, there's no need to pass an error pointer as an extra
argument. Second, as stated before, the error is encoded into the structure, so
subsequent calls on an erroneous structure will fail gracefully, e.g.:

``` c
/* Allocation fails, therefore buffer is invalid */
pb_buffer_t buffer = pb_buffer_create_empty();

/* Message is invalid, as it's created from an invalid buffer */
pb_message_t message = pb_message_create(&descriptor, &buffer);

/* Write to field is not executed, as message is invalid */
pb_message_put(&message, 1, &value);
```

Nevertheless, it is highly recommended to check every function that returns a
new structure or the type `pb_error_t` for errors. See the specific sections
for more details on how this should be done in the respective cases.

## Error codes

### `PB_ERROR_NONE`

The operation was successful. This constant will always yield the value `0`.

### `PB_ERROR_ALLOC`

The required memory could not be allocated, almost only due to an out-of-memory
condition (`ENOMEM`). This may happen when creating a buffer or implicitly
altering a buffer by writing to its dedicated message or fields.

### `PB_ERROR_INVALID`

The arguments passed to the function were not valid, e.g. passing an invalid
buffer to a message constructor or trying to write a value to a message for a
tag that is not part of the underlying message definition.

### `PB_ERROR_VARINT`

An invalid variable-sized integer was encountered, i.e. the corresponding type
is too small. If this happens, the underlying message is either garbled or the
message definitions may not be in sync. For example, a field was changed from
`int32` to `int64` on the encoding side, but forgotten to be updated on the
decoding side.

### `PB_ERROR_OFFSET`

This error is most likely related to garbled data, e.g. when a length prefix
denoted that a message of certain length follows, but the buffer terminated
unexpectedly.

### `PB_ERROR_ABSENT`

This error is returned when trying to read the value of an optional field that
is not present and the field does not define a default value, or a required
field is missing during message validation.

### `PB_ERROR_EOM`

Returned by `pb_cursor_error`, when the cursor is past the end of the last
matching field occurrence, to indicate that the cursor in not valid anymore.

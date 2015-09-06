## Overview

As Protocol Buffers is a data interchange format, messages and message types
are at the very core of protobluff. Message types are logically grouped
together and described in a `.proto` schema file in a language-neutral format.
From this schema file, protobluff generates bindings for the C language, so
that wire-encoded messages of the respective type can be easily processed,
omitting the necessity for manual parsing like it is with schema-less message
formats. However, protobluff does not generate structs from Protocol Buffers
definitions like most of the implementations for the C language do, but rather
descriptors that are used to dynamically alter messages.

## Defining a message type

Protocol Buffer message definitions are written in a declarative language.
For further explanations, we'll use the basic example from the
[Protocol Buffers Developer Guide](
  https://developers.google.com/protocol-buffers/docs/overview#how-do-they-work
) that defines a simple message with information about a person in a file
called `person.proto`:

``` protobuf
message Person {
  message PhoneNumber {
    enum PhoneType {
      MOBILE = 0;
      HOME = 1;
      WORK = 2;
    }
    required string number = 1;
    optional PhoneType type = 2 [default = HOME];
  }
  required string name = 1;
  required int32 id = 2;
  optional string email = 3;
  repeated PhoneNumber phone = 4;
}
```

After defining the message type, the required bindings must be generated. The
code generator can be invoked with the following command:

``` sh
protoc --protobluff_out=. person.proto
```

This will generate two files, namely `person.pb.h` and `person.pb.c` in the
directory specified for the `--protobluff_out` flag. The header file must be
included in the parts of the application where the bindings are used, the
source file must be compiled with the application. Furthermore, the protobluff
runtime must be linked against the application. See the
[getting started guide](/getting-started/#linking) for further details on how
to link protobluff.

The resulting bindings in `person.pb.h` look like this (excerpt):

``` c
/* Person : create */
#define person_create(binary) \
  (pb_message_create( \
    &person_descriptor, (binary)))

/* Person : destroy */
#define person_destroy(message) \
  (person_descriptor_assert(message), \
    (pb_message_destroy(message)))

/* Person.name : get */
#define person_get_name(message, value) \
  (person_descriptor_assert(message), \
    (pb_message_get((message), 1, (value))))

/* Person.name : put */
#define person_put_name(message, value) \
  (person_descriptor_assert(message), \
    (pb_message_put((message), 1, (value))))
```

Why macros? Because there's no overhead when compiling the program, as all
function calls are translated to protobluff's low-level interface. While in
development mode descriptors are checked with assertions for the correct type,
code for production should be compiled with the flag `-DNDEBUG`, which will
remove all assertions.

## Creating a message

A message can easily be created with a valid binary:

``` c
pb_message_t person = person_create(&binary);
if (!pb_message_valid(&person)) {
  /* Error creating person message */
}
```

## Reading from a message

To read the value of a field, the generated accessors can be used:

``` c
pb_string_t name;
if (person_get_name(&person, &name)) {
  /* Error reading name from person message */
}
```

This macro is translated to a call to `pb_message_get` which only allows
non-repeated fields with primitive types to be read, because there's only
one return value. See the documentation on
[repeated fields](/guide/repeated-fields/) for more information.

If the message doesn't contain a value for the respective field and no
default value is set, the function will return `PB_ERROR_ABSENT`. See the
following sections for more information on
[default values](/guide/optional-fields-and-default-values/#default-values) and
[error handling](#error-handling) for messages.

## Writing to a message

Writing a value to a field is equally straight forward:

``` c
pb_string_t name = pb_string_init("John Doe");
if (person_put_name(&person, &name)) {
  /* Error writing name to person message */
}
```

This macro is translated to a call to `pb_message_put` which allow all Protocol
Buffers datatypes to be written. However, writing a repeated field or
submessage will always create (append) a new instance. If a specific instance
should be updated, cursors must be used. Again, see
[this section](/guide/repeated-fields) for guidance on this topic.

## Erasing from a message

Erasing a field can be done with:

``` c
if (person_erase_name(&person)) {
  /* Error erasing name from person message */
}
```

Erasing a field or submessage from a message is an idempotent operation,
regardless of whether the field existed or not. Erasing a repeated field or
submessage will always erase all occurrences. In order to erase specific
occurrences of a field, a cursor must be used.

## Validating a message

The validity of a message, namely the fact that all required fields are
present, is not checked by the accessors for the sake of performance.
Validation needs to be done explicitly:

``` c
if (pb_message_check(&person)) {
  /* Nope, message is not valid */
}
```

This can be omitted, when it is known that the message is valid.

## Freeing a message

Burn after reading -- though messages don't perform any dynamic allocations,
it is recommended to destroy them after use:

``` c
person_destroy(&person);
```

This will keep your application future-proof.

## Dumping a message

For debugging purposes, messages can be dumped to inspect the underlying wire
format:

``` c
pb_message_dump(&person);
```

This will write something like the following to the terminal, denoting the
content and offsets of a message within the context of other messages:

```
   0  offset start
  73  offset end
  73  length
----  ---------------------------------------  -------------------
   0   10   8  74 111 104 110  32  68 111 101  . . J o h n   D o e
  10   16 210   9  26  16 106 100 111 101  64  . . . . . j d o e @
  20  101 120  97 109 112 108 101  46  99 111  e x a m p l e . c o
  30  109  34  19  10  15  43  49  45  53  52  m " . . . + 1 - 5 4
  40   49  45  55  53  52  45  51  48  49  48  1 - 7 5 4 - 3 0 1 0
  50   16   1  34  19  10  15  43  49  45  53  . . " . . . + 1 - 5
  60   52  49  45  50  57  51  45  56  50  50  4 1 - 2 9 3 - 8 2 2
  70   56  16   0                              8 . .
```

## Error handling

All functions and macros that create new structures or alter the contents of
messages and return the type `pb_error_t` should be checked for errors. See
the section on [error handling](/guide/error-handling) for a detailed
description and semantics of all possible errors.
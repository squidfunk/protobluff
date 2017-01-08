## Overview

protobluff implements the concept of a cursor to access separate instances of a
repeated field within an encoded message. Cursors are capable of handling
tag/value-encoded fields, as well as packed fields very well.

The following explanations on handling repeated fields are based on the example
message type defined in the section on
[working with messages](/guide/messages/#defining-a-message-type).

## Creating a cursor

A cursor over a message for a repeated field can be created as follows:

``` c
pb_cursor_t cursor = person_create_phone_cursor(&person);
if (!pb_cursor_valid(&cursor))
  /* Error creating cursor on phone numbers */
}
```

The cursor will always either point to a valid instance of a repeated field or
be invalid. If the cursor isn't valid, the cursor may either have encountered
garbled data or there is no instance for the repeated field.

## Working with a cursor

After checking if the cursor is valid, the current field may be read, written
or erased. While scalar types may be directly read and written through the
cursor, submessages must be created from the cursor in order to work with them.
Erasing a field is independent of its type.

### Submessages

Submessages must be created through the function `pb_message_create_from_cursor`
which will return a message of type `pb_message_t`:

``` c
pb_cursor_t cursor = person_create_phone_cursor(&person);
if (!pb_cursor_valid(&cursor)) {
  do {
    pb_message_t phone = pb_message_create_from_cursor(&cursor);
    if (!pb_message_valid())
      break;
    ...
    person_phonenumber_destroy(&phone);
  } while (pb_cursor_next(&cursor))
}
```

The fields of the message can then be normally accessed as described in the
section on [working with messages](/guide/messages/). After working with the
message, it must be explicitly destroyed.

### Scalar types

Scalar types may be accessed through the functions `pb_cursor_get`,
`pb_cursor_put` and `pb_cursor_erase`, like in the following example:

``` c
pb_cursor_t cursor = ...
if (!pb_cursor_valid(&cursor)) {
  pb_error_t error = PB_ERROR_NONE;

  /* Iterate instances of repeated field */
  do {
    uint32_t value;
    if (error = pb_cursor_get(&cursor, &value)))
      break;

    /* Erase value if equal to 0 */
    if (value == 0) {
      error = pb_cursor_erase(&cursor);

    /* Otherwise decrement value */
    } else {    
      value--;
      error = pb_cursor_put(&cursor, &value);
    }
  } while (!error && pb_cursor_next(&cursor))
}
```

## Freeing a cursor

Like messages, cursors should always be explicitly destroyed to be
future-proof:

``` c
pb_cursor_destroy(&cursor);
```

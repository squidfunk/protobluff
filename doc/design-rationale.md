## The problem with Protocol Buffers

[Protocol Buffers][] is a language-neutral, platform-neutral and extensible
message format developed by Google for serializing structured data. It uses
schema files to describe the structure of messages, which are in turn used to
generate language-specific bindings to automatically handle the decoding and
encoding logic for the developer.

In most language-specific bindings, a Protocol Buffers message is entirely read
and decoded to a native structure (like a `struct` in C or a `class` in C++)
in a transparent manner. The problem is that this may involve a large number
of allocations, especially for deeply nested and repeated messages. This is
particularly wasteful if only a few fields are needed in order to process a
message, e.g. to decode the header for efficient routing or to update single
fields/values within large messages.

## A different approach

protobluff follows a different approach: it omits the need to decode a
Protocol Buffers message in order to process it – all operations are directly
carried out on the encoded message, so only specific parts/fields of the
encoded message need to be decoded. The same is true for encoding – only the
value that needs to be altered must be encoded, the rest of the message
remains untouched.

The technical design goals behind protobluff are:

* Errors are handled gracefully, at all times. protobluff anticipates
  errors like garbled data (regression via fuzzing tests) or out-of-memory
  conditions and returns an error to the caller.

* All invariants are checked with asserts, all runtime errors are checked
  during program execution. All structures contain a valid flag/state. When
  passing an invalid structure to a function, the program does not crash.

* Stack-allocation is used wherever possible, so the programmer can decide on
  where to use dynamic and static allocation. Memory management is centralized,
  which significantly reduces programming errors.

* The runtime should be extensible, i.e. it should be easy to write parsers
  and writers for other formats like JSON or XML or other functionality on top
  of the core library.

Furthermore, protobluff comes with two [runtimes](/guide/runtimes/): a **lite**
runtime which enables easy and efficient stream-processing of Protocol Buffers
messages via a callback-based decoder and an encoder, and a **full** runtime
which adds support for partial reads and writes of messages. New values can be
read or written incrementally, memory management is centralized and handled by
the underlying [journal](/guide/buffers-and-journals/). If no alterations that
change the size of the underlying journal are expected, the journal can be used
in zero-copy mode, omitting all dynamic allocations.

## Trade-offs

In software development, everything is a trade-off. protobluff doesn't check
the encoded messages for validity, i.e. whether all required fields are set.
This should be done by the programmer using the validator explicitly, and is
attributed to the fact that protobluff enables incremental reading and writing
of messages, as discussed above. Moreover it is redundant when the programmer
knows that a message is valid (e.g. when the messages comes from a database).

The lite runtime is efficient for decoding and encoding whole messages. The
full runtime should be used when only a few fields of a message need to be
read or written. protobluff is not very efficient when decoding an encoding
whole messages with the full runtime over and over. There are other libraries
for that. However, as protobluff has a modular design, this functionality
could be added if needed.

[Protocol Buffers]: https://developers.google.com/protocol-buffers
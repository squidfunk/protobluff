## Overview

protobluff is not the only C implementation for Protocol Buffers in the world,
but it's actually one of the more recent ones. It has a different approach for
handling messages than other libraries, like already described in the
[designal rationale](/design-rationale/).

This document compares protobluff to other Protocol Buffers implementations
for the C language, in particular [protobuf-c][], [nanopb][], [lwpb][] and
[pbc][]. Compared are only libraries that are not exclusively targeted at
microcontrollers, but are designed for 32- and 64-bit systems.

## Feature comparison

### proto2

protobluff implements all features of the Protocol Buffers specification
version 2 (known as proto2), except groups and generic services which are both
deprecated. However, as Google is pushing out [gRPC][], there may be a
gRPC-based service implementation in form of a plugin in the future. Below is
a table showing the supported features of protobluff next to other Protocol
Buffers libraries for C:

| Feature / Implementation | protobluff | protobuf-c | nanopb  | lwpb | pbc |
|--------------------------|------------|------------|---------|------|-----|
| Message types            | yes        | yes        | yes     | yes  | yes |
| Nested message types     | yes        | yes        | yes     | yes  | yes |
| Cyclic message types     | yes        | yes        | partial | yes  | yes |
| Scalar types             | yes        | yes        | yes     | yes  | yes |
| Default values           | yes        | yes        | yes     | yes  | yes |
| Enumerations             | yes        | yes        | yes     | yes  | yes |
| Extensions               | yes        | -          | yes     | -    | yes |
| Oneofs                   | yes        | yes        | yes     | -    | -   |
| Packages                 | yes        | yes        | yes     | yes  | yes |
| Packed option            | yes        | yes        | yes     | yes  | yes |
| Deprecations             | yes        | partial    | -       | -    | -   |

In respect to the deprecation of generic services and groups, protobluff is the
only library which is feature complete. [nanopb][] is pretty close: it only
misses deprecations and is inconvenient for working with cyclic messages.
Furthermore, it's targeted at microcontrollers. [protobuf-c][] doesn't support
extensions and supports deprecations for fields only. [lwpb][] and [pbc][] seem
inactive, as they don't support oneofs, a feature introduced in version 2.6.

Both, protobuf-c and lwpb provide a custom service implementation. However, the
benefit is questionable, because the way messages are handled within a service
may be highly application-dependent. Therefore, the solution proposed by both
libraries may or may not be a good fit for your application. This is the reason
why Google deprecated generic services and open sourced gRPC to provide a
standard protocol for service interaction and integration.

### proto3

The newest version of the Protocol Buffers specification is
[version 3][Protocol Buffers specification] (known as proto3) which is
wire-compatible with proto2. protobluff is basically compatible with proto3 –
it will compile with the proto3 runtime, but it is not yet optimized for the
new syntax.

The main features that need to be implemented for full proto3 support are
maps and the **Any** type as a replacement for extensions, as well as some
helper functions for working with the new set of default types and maybe the
new JSON-mapping. However, the last two are of lower priority.

At the time of writing, none of the aforementioned alternative implementations
supports proto3 yet.

[protobuf-c]: https://github.com/protobuf-c/protobuf-c
[nanopb]: https://github.com/nanopb/nanopb
[lwpb]: https://github.com/acg/lwpb
[pbc]: https://github.com/cloudwu/pbc
[gRPC]: http://grpc.io
[Protocol Buffers specification]: https://developers.google.com/protocol-buffers/docs/proto3?hl=en

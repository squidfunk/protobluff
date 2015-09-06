## Overview

Scalar values are the most primitive types of fields. The Protocol Buffers
language defines a multitude of signed and unsigned integer types (each with
different implications for the underlying wire representation), as well as
types for the representation of floating point numbers, booleans, bytes and
strings. The following section explains how the Protocol Buffer types map to
native datatypes.

Protocol Buffer types are subscripted with <sub>p</sub>, native datatypes with
<sub>c</sub>.

## Integers (32-bit)

`uint32`<sub>p</sub> and `fixed32`<sub>p</sub> map to `uint32_t`<sub>c</sub>,
`int32`<sub>p</sub>, `sint32`<sub>p</sub> and `sfixed32`<sub>p</sub> map to
`int32_t`<sub>c</sub>

32-bit integers come in variable-sized and fixed-sized encodings and map to
the native integer types with a wordsize of four bytes in signed and unsigned
flavors.

## Integers (64-bit)

`uint64`<sub>p</sub> and `fixed64`<sub>p</sub> map to `uint64_t`<sub>c</sub>,
`int64`<sub>p</sub>, `sint64`<sub>p</sub> and `sfixed64`<sub>p</sub> map to
`int64_t`<sub>c</sub>

64-bit integers come in variable-sized and fixed-sized encodings and map to
the native integer types with a wordsize of eight bytes in signed and unsigned
flavors.

## Booleans

`bool`<sub>p</sub> maps to `uint8_t`<sub>c</sub>

protobluff uses unsigned chars or more specifically `uint8_t`<sub>c</sub> for
the representation of boolean values, because the native `bool`<sub>c</sub>
datatype is only available from C99 on and not implemented and/or available on
all platforms.

## Floats and doubles

`float`<sub>p</sub> maps to `float`<sub>c</sub>, `double`<sub>p</sub> maps to
`double`<sub>c</sub>

The wire representation of floating point numbers adheres to the IEEE-754
standard with a wordsize of four for single-precision and a wordsize of eight
for double-precision.

## Bytes and strings

`bytes`<sub>p</sub> and `string`<sub>p</sub> map to `pb_string_t`<sub>c</sub>

Strings and byte arrays are stored as a length-prefixed, unterminated list of
unsigned chars on the wire. The datatype `pb_string_t`<sub>c</sub> is used to
read or write byte arrays and strings.
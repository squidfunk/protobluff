## Overview

The following document discusses some proto2 features, that don't have a huge
impact on message handling.

## Oneofs

protobluff is capable of handling oneofs. It is even designed to handle oneofs
in some edge cases, like merged messages, where multiple instances of a oneof
may occur. When setting a field that is part of a oneof, protobluff ensures
that the field is not overshadowed by a later instance by traversing the cursor
to the end and erasing all members of the oneof.

Furthermore, for each oneof, the code generator will create a function to
determine which member of a oneof is currently active. This function returns
a tag number and can be used inside a `switch` statement.

## Extensions

Extensions are implemented as a linked list of descriptors. They may be defined
in file or message scope (the latter are known as nested extensions). The
code generator will infix extensions with `_X_`, in order to avoid name clashes
with existing fields. Extensions are initialized automatically upon start up
of the program through constructor attributes.

## Packages

If a `.proto` file defines a package, the name of the package is prefixed to
all variables and functions defined inside that file. This is especially useful
to avoid name clashes between packages.

## Deprecations

Inside a `.proto` schema file, fields, messages, enums and enum values can be
annotated with the `deprecated` option. protobluff will flag all functions that
access the annotated structure with `PB_DEPRECATED`, a compiler attribute to
signal deprecated access. This will trigger warnings during compilation without
any runtime penalty.
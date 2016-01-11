# Copyright (c) 2013-2016 Martin Donath <martin.donath@squidfunk.com>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to
# deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

# -----------------------------------------------------------------------------
# Prologue
# -----------------------------------------------------------------------------

AC_PREREQ(2.69)

AC_INIT([protobluff], [0.5.0], [martin.donath@squidfunk.com])
AM_INIT_AUTOMAKE([subdir-objects foreign])

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

: ${CFLAGS=""}
: ${CXXFLAGS=""}

AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP

LT_INIT

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------

# Library versioning as <current:revision:age> - also remember to synchronize
# this value with the version info in the core/common.h header file.
AC_SUBST([VERSION_INFO], [4:0:0])

# Checks for programs
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Check for Protocol Buffers support to build generator
AC_CHECK_LIB([protobuf], [main], [LIBPROTOBUF="-lprotobuf"], [
	AC_MSG_WARN([libprotobuf not found; cannot build protoc-gen-protobluff])])
AC_CHECK_LIB([protoc], [main], [LIBPROTOC="-lprotoc"], [
	AC_MSG_WARN([libprotoc not found; cannot build protoc-gen-protobluff])])
AC_CHECK_PROGS([PROTOC], [protoc])
if test -z "$PROTOC"; then
	AC_MSG_WARN([protoc not found; cannot build protoc-gen-protobluff])
fi
AM_CONDITIONAL(HAVE_PROTOBUF, [ \
	test -n "$LIBPROTOBUF" -a -n "$LIBPROTOC" -a -n "$PROTOC"])
if test -z "$HAVE_PROTOBUF"; then
	protobuf_LIBS="$LIBPROTOBUF $LIBPROTOC"
else
	protobuf_LIBS=""
fi
AC_SUBST([protobuf_LIBS])
AC_SUBST([PROTOC])

# Check for ability to build and run tests
PKG_CHECK_MODULES([check], [check >= 0.9.10], [],
	AC_MSG_WARN([check not found; cannot build tests]))
AM_CONDITIONAL(HAVE_CHECK, [test -n "$check_LIBS"])

# Additional compiler flags for check
if test "${CC}" = "clang"; then
	check_CFLAGS+=" -g -Wno-incompatible-pointer-types-discards-qualifiers"
else
	check_CFLAGS+=" -g"
fi

# Check for ability to build coverage report
AC_CHECK_PROGS([LCOV], [lcov])
if test -z "$LCOV"; then
	AC_MSG_WARN([lcov not found; cannot build coverage report])
fi
AC_CHECK_PROGS([GENHTML], [genhtml])
if test -z "$GENHTML"; then
	AC_MSG_WARN([genhtml not found; cannot build coverage report])
fi
AM_CONDITIONAL(HAVE_COVERAGE, [test -n "$LCOV" -a -n "$GENHTML"])

# Check for ability to run memcheck
AC_CHECK_PROGS([VALGRIND], [valgrind])
if test -z "$VALGRIND"; then
	AC_MSG_WARN([valgrind not found; cannot run leak check])
fi
AM_CONDITIONAL(HAVE_VALGRIND, [test -n "$VALGRIND"])

# -----------------------------------------------------------------------------
# Arguments
# -----------------------------------------------------------------------------

# Disable optimized compilation
AC_ARG_ENABLE([optimized],
	[AS_HELP_STRING([--disable-optimized],
		[no optimizations @<:@default=enabled@:>@])],
	[OPTIMIZATIONS="-g"],
	[OPTIMIZATIONS="-O3 -DNDEBUG"
	 OPTIMIZATIONS+=" -fno-exceptions"
	 OPTIMIZATIONS+=" -fno-unwind-tables"
	 OPTIMIZATIONS+=" -fno-asynchronous-unwind-tables"
	 OPTIMIZATIONS+=" -fno-stack-protector"
	 OPTIMIZATIONS+=" -fomit-frame-pointer"])
AC_SUBST([OPTIMIZATIONS])

# Enable stripped build
AC_ARG_ENABLE([stripped],
	[AS_HELP_STRING([--enable-stripped],
		[strip internal symbols @<:@default=disabled@:>@])],
	[CPPFLAGS+=" -fvisibility=hidden"], [])

# Enable coverage report
AC_ARG_ENABLE([coverage],
	[AS_HELP_STRING([--enable-coverage],
		[coverage report @<:@default=disabled@:>@])],
	[coverage_CPPFLAGS="--coverage" coverage_LDFLAGS="--coverage"],
	[coverage_CPPFLAGS=""           coverage_LDFLAGS=""])
AC_SUBST([coverage_CPPFLAGS])
AC_SUBST([coverage_LDFLAGS])

# -----------------------------------------------------------------------------
# Header files, types and functions
# -----------------------------------------------------------------------------

# Check for header files
AC_HEADER_STDBOOL
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([ \
	float.h \
	limits.h \
	stddef.h \
	stdint.h \
	stdlib.h \
	string.h])

# Check for keywords and types
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_CHECK_TYPES([ptrdiff_t])

# Check for library functions
AC_FUNC_ALLOCA
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_STRTOD
AC_CHECK_FUNCS([ \
	localeconv \
	memmove \
	setlocale \
	strchr \
	strpbrk \
	strtol \
	strtoul \
	strtoull])

# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------

# Configuration files
AC_CONFIG_FILES([
	Makefile
	examples/decoding/Makefile
	examples/encoding/Makefile
	examples/messages/Makefile
	examples/Makefile
	include/Makefile
	src/core/Makefile
	src/generator/Makefile
	src/message/Makefile
	src/util/Makefile
	src/Makefile
	src/protobluff-lite.pc
	src/protobluff.pc
	tests/core/buffer/Makefile
	tests/core/decoder/Makefile
	tests/core/descriptor/Makefile
	tests/core/encoder/Makefile
	tests/core/stream/Makefile
	tests/core/varint/Makefile
	tests/core/Makefile
	tests/message/buffer/Makefile
	tests/message/cursor/Makefile
	tests/message/field/Makefile
	tests/message/journal/Makefile
	tests/message/message/Makefile
	tests/message/nested/Makefile
	tests/message/oneof/Makefile
	tests/message/part/Makefile
	tests/message/Makefile
	tests/util/chunk_allocator/Makefile
	tests/util/descriptor/Makefile
	tests/util/validator/Makefile
	tests/util/Makefile
	tests/Makefile
])

# Compiler flags
CFLAGS+=" -std=c99"
CXXFLAGS+=" -std=c++98"
CPPFLAGS+=" -Wall -Wno-deprecated-declarations -Wno-address -march=native"

# Output result
AC_OUTPUT

# Print build configuration
AC_MSG_RESULT([
  $PACKAGE $VERSION

    CC:                     ${CC}
    CFLAGS:                 ${CFLAGS} ${OPTIMIZATIONS}
    CXX:                    ${CXX}
    CXXFLAGS:               ${CXXFLAGS} ${OPTIMIZATIONS}
    CPPFLAGS:               ${CPPFLAGS} ${coverage_CPPFLAGS}
    LDFLAGS:                ${LDFLAGS} ${coverage_LDFLAGS}

    prefix:                 ${prefix}
    sysconfdir:             ${sysconfdir}
    libdir:                 ${libdir}
    includedir:             ${includedir}
])
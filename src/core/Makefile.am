# Copyright (c) 2013-2020 Martin Donath <martin.donath@squidfunk.com>

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
# Intermediary library: core
# -----------------------------------------------------------------------------

# Build core intermediary library
noinst_LTLIBRARIES = libprotobluff-core.la
libprotobluff_core_la_SOURCES = \
	allocator.c \
	buffer.c \
	decoder.c \
	descriptor.c \
	encoder.c \
	stream.c \
	varint.c
libprotobluff_core_la_CPPFLAGS = \
	-I@top_builddir@/src \
	-I@top_builddir@/include \
	@OPTIMIZATIONS@ \
	@coverage_CPPFLAGS@
libprotobluff_core_la_LDFLAGS = \
	@coverage_LDFLAGS@

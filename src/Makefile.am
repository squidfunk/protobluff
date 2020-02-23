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
# Subdirectories
# -----------------------------------------------------------------------------

SUBDIRS = core message util generator

# -----------------------------------------------------------------------------
# Shared library: libprotobluff-lite
# -----------------------------------------------------------------------------

# Build libprotobluff-lite shared library
lib_LTLIBRARIES = libprotobluff-lite.la
libprotobluff_lite_la_SOURCES =
libprotobluff_lite_la_LIBADD = \
	@top_builddir@/src/core/libprotobluff-core.la
libprotobluff_lite_la_LDFLAGS = \
	-version-info @VERSION_INFO@

# -----------------------------------------------------------------------------
# Shared library: libprotobluff
# -----------------------------------------------------------------------------

# Build libprotobluff shared library
lib_LTLIBRARIES += libprotobluff.la
libprotobluff_la_SOURCES =
libprotobluff_la_LIBADD = \
	@top_builddir@/src/message/libprotobluff-message.la \
	@top_builddir@/src/util/libprotobluff-util.la \
	@top_builddir@/src/core/libprotobluff-core.la
libprotobluff_la_LDFLAGS = \
	-version-info @VERSION_INFO@

# Copyright (c) 2013-2015 Martin Donath <martin.donath@squidfunk.com>

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
# Executable: protoc-gen-protobluff
# -----------------------------------------------------------------------------

if HAVE_PROTOBUF

# Build protoc-gen-protobluff executable
bin_PROGRAMS = protoc-gen-protobluff
protoc_gen_protobluff_SOURCES = \
	enum.cc \
	enum_value.cc \
	extension.cc \
	field.cc \
	file.cc \
	generator.cc \
	message.cc \
	oneof.cc \
	protoc-gen-protobluff.cc \
	strutil.cc
protoc_gen_protobluff_CXXFLAGS = \
	-Wno-sign-compare
protoc_gen_protobluff_CPPFLAGS = \
	-I@top_builddir@/src \
	-I@top_builddir@/include \
	@OPTIMIZATIONS@
protoc_gen_protobluff_LDFLAGS = \
	@protobuf_LIBS@

endif # HAVE_PROTOBUF
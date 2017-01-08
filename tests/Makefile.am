# Copyright (c) 2013-2017 Martin Donath <martin.donath@squidfunk.com>

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

SUBDIRS = core message util

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------

# Add core tests
TESTS = \
	core/buffer/test \
	core/decoder/test \
	core/descriptor/test \
	core/encoder/test \
	core/stream/test \
	core/varint/test

# Add message tests
TESTS += \
	message/buffer/test \
	message/cursor/test \
	message/field/test \
	message/journal/test \
	message/message/test \
	message/nested/test \
	message/oneof/test \
	message/part/test

# Add util tests
TESTS += \
	util/chunk_allocator/test \
	util/descriptor/test \
	util/validator/test

# -----------------------------------------------------------------------------
# Coverage report
# -----------------------------------------------------------------------------

if HAVE_COVERAGE

# Reset execution counts
clean-local:
	@LCOV@ -z -d @top_builddir@/src
	rm -rf coverage
	rm -f  coverage.info

endif # HAVE_COVERAGE

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
# Subdirectories
# -----------------------------------------------------------------------------

SUBDIRS = include src tests

# -----------------------------------------------------------------------------
# pkg-config configuration
# -----------------------------------------------------------------------------

# Create pkg-config recipe
pkgconfigdir = @libdir@/pkgconfig
pkgconfig_DATA = \
	src/protobluff.pc \
	src/protobluff-lite.pc

# -----------------------------------------------------------------------------
# Custom build rules
# -----------------------------------------------------------------------------

# Alias check with test
test: check

# Clean profiling information
clean-local:
	find src -name '*.gcda' -type f -delete
	find src -name '*.gcno' -type f -delete

# Purge development files
purge: clean
	find . -depth -name '.deps*' -exec rm -rf '{}' \;
	find . -depth -name '.libs*' -exec rm -rf '{}' \;
	find . -name 'Makefile' -type f -delete
	find . -name 'Makefile.in' -type f -delete
	find . -name '.dirstamp' -type f -delete
	find . -name '.DS_Store' -type f -delete
	find . -name '*.log' -type f -delete
	find . -name '*.trs' -type f -delete
	rm -f  aclocal.m4
	rm -rf autom4te.cache
	rm -f  autoscan.log
	rm -f  compile
	rm -f  configure
	rm -f  configure.scan
	rm -f  config.*
	rm -f  depcomp
	rm -f  install-sh
	rm -f  libtool
	rm -f  ltmain.sh
	rm -rf m4
	rm -f  missing
	rm -f  stamp-h1
	rm -f  test-driver

# -----------------------------------------------------------------------------
# Coverage report
# -----------------------------------------------------------------------------

if HAVE_COVERAGE

# Generate and display code coverage report
coverage:
	@LCOV@ -q -c --no-external -d @top_builddir@/src -o tests/coverage.info
	@GENHTML@ tests/coverage.info -o tests/coverage -c tests/coverage.css

endif # HAVE_COVERAGE

# -----------------------------------------------------------------------------
# Leak check
# -----------------------------------------------------------------------------

if HAVE_VALGRIND

# Perform leak check using unit tests
memcheck: check
	@for t in `find tests -maxdepth 3 -perm +111 -type f`; do \
	  @LIBTOOL@ --mode=execute @VALGRIND@ --dsymutil=yes --tool=memcheck \
	  --track-origins=yes --leak-check=full --show-reachable=yes \
	  --suppressions=valgrind.supp $$t; \
	done

endif # HAVE_VALGRIND
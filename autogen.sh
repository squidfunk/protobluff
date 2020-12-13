#!/bin/bash

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

# Sometimes autoreconf doesn't create m4 directory, so do it explicitly
mkdir m4 > /dev/null 2>&1

# 1st pass: generate relevant files
command -v autoreconf > /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo "autogen.sh: error: could not find autoreconf." 1>&2
  echo "autoconf is required to run autogen.sh." 1>&2
  exit 1
fi

# Generate Makefile.in from Makefile.am
command -v automake --add-missing > /dev/null 2>&1
if [ $? -ne 0 ]; then
  echo "autogen.sh: error: could not find automake." 1>&2
  echo "automake is required to run autogen.sh." 1>&2
  exit 1
fi

# Generate files to build shared libraries
command -v libtool > /dev/null 2>&1
if  [ $? -ne 0 ]; then
  echo "autogen.sh: error: could not find libtool." 1>&2
  echo "libtool is required to run autogen.sh." 1>&2
  exit 1
fi

# 2nd pass: generate configure script
autoreconf --install --force --verbose
if [ $? -ne 0 ]; then
  echo "autogen.sh: error: autoreconf exited with status $?" 1>&2
  exit 1
fi

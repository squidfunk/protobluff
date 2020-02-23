/*
 * Copyright (c) 2013-2020 Martin Donath <martin.donath@squidfunk.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef PB_GENERATOR_ENUM_HH
#define PB_GENERATOR_ENUM_HH

#include <map>
#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

#include "generator/enum_value.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::map;
  using ::std::string;
  using ::std::unique_ptr;

  using ::google::protobuf::EnumDescriptor;
  using ::google::protobuf::io::Printer;

  class Enum {

  public:
    explicit
    Enum(
      const EnumDescriptor
        *descriptor);                  /* Enum descriptor */

    void
    GenerateValues(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateDeclaration(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateDescriptor(
      Printer *printer)                /* Printer */
    const;

  private:
    const EnumDescriptor *descriptor_; /* Enum descriptor */
    unique_ptr<
      unique_ptr<EnumValue>[]
    > values_;                         /* Enum value generators */
    map<string, string> variables_;    /* Variables */
  };
}

#endif /* PB_GENERATOR_ENUM_HH */

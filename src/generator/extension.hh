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

#ifndef PB_GENERATOR_EXTENSION_HH
#define PB_GENERATOR_EXTENSION_HH

#include <map>
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

#include "generator/field.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::map;
  using ::std::string;
  using ::std::vector;

  using ::google::protobuf::Descriptor;
  using ::google::protobuf::FieldDescriptor;
  using ::google::protobuf::io::Printer;

  class Extension {

  public:

    explicit
    Extension(
      const Descriptor *descriptor,    /* Descriptor */
      const Descriptor
        *scope = NULL);                /* Scope descriptor */

    void
    AddField(
      const FieldDescriptor
        *descriptor);                  /* Field descriptor */

    void
    GenerateDefaults(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateDescriptor(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateInitializer(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateAccessors(
      Printer *printer)                /* Printer */
    const;

    bool
    HasDefaults()
    const;

  private:
    const Descriptor *descriptor_;     /* Descriptor */
    const Descriptor *scope_;          /* Scope descriptor */
    vector<Field *> fields_;           /* Field generators */
    map<string, string> variables_;    /* Variables */
  };
}

#endif /* PB_GENERATOR_EXTENSION_HH */

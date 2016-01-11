/*
 * Copyright (c) 2013-2015 Martin Donath <martin.donath@squidfunk.com>
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

#ifndef PB_GENERATOR_FIELD_HH
#define PB_GENERATOR_FIELD_HH

#include <map>
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::map;
  using ::std::string;
  using ::std::vector;

  using ::google::protobuf::FieldDescriptor;
  using ::google::protobuf::io::Printer;

  class Field {

  public:
    explicit
    Field(
      const FieldDescriptor
        *descriptor);                  /* Field descriptor */

    void
    GenerateDefault(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateDescriptor(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateEncoder(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateAccessors(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateAccessors(
      Printer *printer,                /* Printer */
      vector<
        const FieldDescriptor *
      > &trace)                        /* Trace */
    const;

    bool
    HasDefault()
    const;

    friend bool
    FieldComparator(
      const Field *x,                  /* Field generator */
      const Field *y);                 /* Field generator */

  private:
    const FieldDescriptor
      *descriptor_;                    /* Field descriptor */
    map<string, string> variables_;    /* Variables */

    bool
    ShouldTrace()
    const;
  };

  bool
  FieldComparator(
    const Field *x,                    /* Field generator */
    const Field *y);                   /* Field generator */
}

#endif /* PB_GENERATOR_FIELD_HH */
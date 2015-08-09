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

#ifndef PB_PROTOBLUFF_MESSAGE_HH
#define PB_PROTOBLUFF_MESSAGE_HH

#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

#include "bin/field.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::vector;

  using ::google::protobuf::Descriptor;
  using ::google::protobuf::FieldDescriptor;
  using ::google::protobuf::io::Printer;
  using ::google::protobuf::scoped_array;
  using ::google::protobuf::scoped_ptr;

  class Message {

  public:
    explicit
    Message(
      const Descriptor *descriptor);   /* Descriptor */

    void
    GenerateDeclarations(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateDefaults(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateDescriptors(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateDescriptorAssertions(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateDefinitions(
      Printer *printer)                /* Printer */
    const;

    void
    GenerateDefinitions(
      Printer *printer,                /* Printer */
      vector<
        const FieldDescriptor *
      > &trace)                        /* Trace */
    const;

    bool
    HasDefaults()
    const;

  private:
    const Descriptor *descriptor;      /* Descriptor */
    scoped_array<
      scoped_ptr<Field>
    > fields;                          /* Field generators */
    scoped_array<
      scoped_ptr<Message>
    > nested;                          /* Nested generators */
  };
}

#endif /* PB_PROTOBLUFF_MESSAGE_HH */
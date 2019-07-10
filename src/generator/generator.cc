/*
 * Copyright (c) 2013-2017 Martin Donath <martin.donath@squidfunk.com>
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

#include <cassert>
#include <string>

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/stubs/common.h>

#include "generator/file.hh"
#include "generator/generator.hh"
#include "generator/strutil.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::string;

  using ::google::protobuf::compiler::CodeGenerator;
  using ::google::protobuf::compiler::OutputDirectory;
  using ::google::protobuf::FileDescriptor;
  using ::google::protobuf::io::Printer;
  using ::google::protobuf::io::ZeroCopyOutputStream;

  using ::google::protobuf::StripSuffixString;

  /*!
   * Generate Protobluff-compatible source and header files from a file.
   *
   * \param[in]  descriptor File descriptor
   * \param[in]  parameter  Command-line parameters
   * \param[in]  directory  Output directory
   * \param[out] error      Error pointer
   */
  bool Generator::
  Generate(
      const FileDescriptor *descriptor, const string &parameter,
      OutputDirectory *directory, string *error) const {
    assert(descriptor && directory && error);

    /* Construct basename for source and header files */
    string basename = StripSuffixString(descriptor->name(), ".proto") + ".pb";

    /* Create file generator from descriptor */
    File file (descriptor);

    /* Generate header file */
    {
      std::unique_ptr<ZeroCopyOutputStream> output(
        directory->Open(basename + ".h"));
      Printer printer (output.get(), '`');
      file.GenerateHeader(&printer);
    }

    /* Generate source file */
    {
      std::unique_ptr<ZeroCopyOutputStream> output(
        directory->Open(basename + ".c"));
      Printer printer (output.get(), '`');
      file.GenerateSource(&printer);
    }
    return true;
  }
}

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

#include <algorithm>
#include <cassert>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

#include "generator/enum.hh"
#include "generator/enum_value.hh"
#include "generator/strutil.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::sort;
  using ::std::vector;

  using ::google::protobuf::EnumDescriptor;
  using ::google::protobuf::FieldDescriptor;
  using ::google::protobuf::io::Printer;
  using ::google::protobuf::scoped_ptr;

  using ::google::protobuf::LowerString;
  using ::google::protobuf::SimpleItoa;
  using ::google::protobuf::StringReplace;

  /*!
   * Create an enum generator.
   *
   * \param[in] descriptor Enum descriptor
   */
  Enum::
  Enum(const EnumDescriptor *descriptor) :
      descriptor_(descriptor),
      values_(new scoped_ptr<EnumValue>[descriptor_->value_count()]) {

    /* Sort enum value generators by number */
    vector<EnumValue *> sorted;
    for (size_t v = 0; v < descriptor_->value_count(); v++)
      sorted.push_back(new EnumValue(descriptor_->value(v)));
    sort(sorted.begin(), sorted.end(), EnumValueComparator);

    /* Initialize enum value generators */
    for (size_t v = 0; v < descriptor_->value_count(); v++)
      values_[v].reset(sorted[v]);

    /* Extract full name for signature */
    variables_["signature"] = descriptor_->full_name();

    /* Prepare enum symbol */
    variables_["enum"] = StringReplace(
      variables_["signature"], ".", "_", true);
    LowerString(&(variables_["enum"]));
  }

  /*!
   * Generate values.
   *
   * \param[in,out] printer Printer
   */
  void Enum::
  GenerateValues(Printer *printer) const {
    assert(printer);
    printer->Print(variables_,
      "/* `signature` : values */\n");
    for (size_t v = 0; v < descriptor_->value_count(); v++)
      values_[v]->GenerateValue(printer);
    printer->Print("\n");
  }

  /*!
   * Generate declaration.
   *
   * \param[in,out] printer Printer
   */
  void Enum::
  GenerateDeclaration(Printer *printer) const {
    assert(printer);

    /* Generate forward declaration */
    printer->Print(variables_,
      "/* `signature` : descriptor */\n"
      "extern const pb_enum_descriptor_t\n"
      "`enum`_descriptor;\n"
      "\n");
  }

  /*!
   * Generate descriptor.
   *
   * \param[in,out] printer Printer
   */
  void Enum::
  GenerateDescriptor(Printer *printer) const {
    assert(printer);

    /* Generate descriptor header */
    if (descriptor_->value_count()) {
      printer->Print(variables_,
        "/* `signature` : descriptor */\n"
        "const pb_enum_descriptor_t\n"
        "`enum`_descriptor = { {\n"
        "  (const pb_enum_value_descriptor_t []){\n");

      /* Generate enum value descriptors */
      for (size_t i = 0; i < 2; i++)
        printer->Indent();
      for (size_t v = 0; v < descriptor_->value_count(); v++) {
        values_[v]->GenerateDescriptor(printer);
        if (v < descriptor_->value_count() - 1)
          printer->Print(",");
        printer->Print("\n");
      }
      for (size_t i = 0; i < 2; i++)
        printer->Outdent();

      /* Generate descriptor footer */
      printer->Print(
        "\n"
        "  }, `values` } };\n"
        "\n", "values", SimpleItoa(descriptor_->value_count()));

    /* Print empty descriptor, if enum contains no values */
    } else {
      printer->Print(variables_,
        "/* `signature` : descriptor */\n"
        "pb_enum_descriptor_t\n"
        "`enum`_descriptor = {};\n"
        "\n");
    }
  }
}

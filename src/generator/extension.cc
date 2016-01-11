/*
 * Copyright (c) 2013-2016 Martin Donath <martin.donath@squidfunk.com>
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
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

#include "generator/extension.hh"
#include "generator/field.hh"
#include "generator/strutil.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::sort;
  using ::std::string;
  using ::std::vector;

  using ::google::protobuf::Descriptor;
  using ::google::protobuf::FieldDescriptor;
  using ::google::protobuf::io::Printer;

  using ::google::protobuf::LowerString;
  using ::google::protobuf::SimpleItoa;
  using ::google::protobuf::StringReplace;
  using ::google::protobuf::StripPrefixString;

  /*!
   * Create an extension generator.
   *
   * If the extension is defined within a message type, the scope descriptor
   * points to the containing message type. Otherwise, it is NULL.
   *
   * \param[in] descriptor Descriptor
   * \param[in] scope      Scope descriptor
   */
  Extension::
  Extension(const Descriptor *descriptor, const Descriptor *scope) :
      descriptor_(descriptor),
      scope_(scope) {

    /* Extract full name for signature */
    variables_["signature"] = descriptor_->full_name();

    /* Prepare message symbol */
    variables_["message"] = StringReplace(
      variables_["signature"], ".", "_", true);
    LowerString(&(variables_["message"]));

    /* Suffix scope to identifiers, if given */
    string suffix ("");
    if (scope_) {
      suffix = scope_->full_name();

      /* Check if the base and extension types are in the same package */
      if (!scope_->file()->package().compare(descriptor_->file()->package()))
        suffix = StripPrefixString(suffix,
          scope_->file()->package() + ".");

      /* Append to signature */
      variables_["signature"] += ".[" + suffix +"]";
      suffix = "_" + suffix;
    }

    /* Prepare extension symbol */
    variables_["extension"] = StringReplace(
      suffix, ".", "_", true);
    LowerString(&(variables_["extension"]));
  }

  /*
   * Add a field to an extension generator.
   *
   * \param[in] descriptor Field descriptor
   */
  void Extension::
  AddField(const FieldDescriptor *descriptor) {
    assert(descriptor && descriptor->is_extension());
    assert(descriptor->containing_type() == descriptor_);

    /* Initialize field generators and preserve ascending order */
    fields_.push_back(new Field(descriptor));
    sort(fields_.begin(), fields_.end(), FieldComparator);
  }

  /*!
   * Generate default values.
   *
   * \param[in,out] printer Printer
   */
  void Extension::
  GenerateDefaults(Printer *printer) const {
    assert(printer);
    for (size_t f = 0; f < fields_.size(); f++)
      fields_[f]->GenerateDefault(printer);
  }

  /*!
   * Generate descriptor.
   *
   * \param[in,out] printer Printer
   */
  void Extension::
  GenerateDescriptor(Printer *printer) const {
    assert(printer);

    /* Generate descriptor header */
    printer->Print(variables_,
      "/* `signature` : extension descriptor */\n"
      "static pb_descriptor_t\n"
      "`message`_X`extension`_descriptor = { {\n"
      "  (const pb_field_descriptor_t []){\n");

    /* Generate field descriptors */
    for (size_t i = 0; i < 2; i++)
      printer->Indent();
    for (size_t f = 0; f < fields_.size(); f++) {
      fields_[f]->GenerateDescriptor(printer);
      if (f < fields_.size() - 1)
        printer->Print(",");
      printer->Print("\n");
    }
    for (size_t i = 0; i < 2; i++)
      printer->Outdent();

    /* Generate descriptor footer */
    printer->Print(
      "\n"
      "  }, `fields` } };\n"
      "\n", "fields", SimpleItoa(fields_.size()));
  }

  /*!
   * Generate initializer.
   *
   * \param[in,out] printer Printer
   */
  void Extension::
  GenerateInitializer(Printer *printer) const {
    assert(printer);
    printer->Print(variables_,
      "/* `signature` : extension initializer */\n"
      "PB_CONSTRUCTOR\n"
      "static void\n"
      "`message`_descriptor_extend`extension`() {\n"
      "  pb_descriptor_extend(\n"
      "    &`message`_descriptor,\n"
      "    &`message`_X`extension`_descriptor);\n"
      "}\n"
      "\n");
  }

  /*!
   * Generate accessors.
   *
   * \param[in,out] printer Printer
   */
  void Extension::
  GenerateAccessors(Printer *printer) const {
    assert(printer);
    for (size_t f = 0; f < fields_.size(); f++)
      fields_[f]->GenerateAccessors(printer);
  }

  /*!
   * Check whether an extension defines default values.
   *
   * \return Test result
   */
  bool Extension::
  HasDefaults() const {
    for (size_t f = 0; f < fields_.size(); f++)
      if (fields_[f]->HasDefault())
        return true;

    /* No default values */
    return false;
  }
}
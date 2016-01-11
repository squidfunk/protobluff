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

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

#include "generator/oneof.hh"
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
  using ::google::protobuf::OneofDescriptor;
  using ::google::protobuf::io::Printer;
  using ::google::protobuf::scoped_ptr;

  using ::google::protobuf::JoinStrings;
  using ::google::protobuf::LowerString;
  using ::google::protobuf::SimpleItoa;
  using ::google::protobuf::StringReplace;

  /*!
   * Create a oneof generator.
   *
   * \param[in] descriptor Oneof descriptor
   */
  Oneof::
  Oneof(const OneofDescriptor *descriptor) :
      descriptor_(descriptor),
      fields_(new scoped_ptr<Field>[descriptor_->field_count()]) {

    /* Sort field generators by tag */
    vector<Field *> sorted;
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      sorted.push_back(new Field(descriptor_->field(f)));
    sort(sorted.begin(), sorted.end(), FieldComparator);

    /* Initialize field generators */
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      fields_[f].reset(sorted[f]);

    /* Extract full name for signature */
    variables_["signature"] = descriptor_->full_name();

    /* Extract name */
    variables_["name"] = descriptor_->name();

    /* Prepare oneof symbol */
    variables_["oneof"] = StringReplace(
      variables_["signature"], ".", "_", true);
    LowerString(&(variables_["oneof"]));

    /* Prepare message symbol */
    variables_["message"] = StringReplace(
      descriptor_->containing_type()->full_name(), ".", "_", true);
    LowerString(&(variables_["message"]));
  }

  /*!
   * Generate declaration.
   *
   * \param[in,out] printer Printer
   */
  void Oneof::
  GenerateDeclaration(Printer *printer) const {
    assert(printer);

    /* Generate forward declaration */
    printer->Print(variables_,
      "/* `signature` : descriptor */\n"
      "extern const pb_oneof_descriptor_t\n"
      "`oneof`_descriptor;\n"
      "\n");
  }

  /*!
   * Generate descriptor.
   *
   * \param[in,out] printer Printer
   */
  void Oneof::
  GenerateDescriptor(Printer *printer) const {
    assert(printer);

    /* Generate descriptor header */
    printer->Print(variables_,
      "/* `signature` : descriptor */\n"
      "const pb_oneof_descriptor_t\n"
      "`oneof`_descriptor = {\n"
      "  &`message`_descriptor, {\n"
      "    (const size_t []){\n"
      "      ");

    /* Retrieve containing type */
    const Descriptor *descriptor = descriptor_->containing_type();

    /* Retrieve all field tags */
    vector<int> tags;
    for (size_t f = 0; f < descriptor->field_count(); f++)
      tags.push_back(descriptor->field(f)->number());

    /* Sort tags to determine indexes */
    sort(tags.begin(), tags.end());

    /* Generate indexes */
    for (size_t f = 0, g = 0; f < descriptor->field_count(); f++) {
      if (descriptor->field(f) == descriptor_->field(g)) {
        printer->Print("`index`", "index", SimpleItoa(f));
        if (g < descriptor_->field_count() - 1)
          printer->Print(", ");
        g++;
      }
    }

    /* Generate descriptor footer */
    printer->Print(
      "\n"
      "    }, `fields` } };\n"
      "\n", "fields", SimpleItoa(descriptor_->field_count()));
  }

  /*!
   * Generate accessors.
   *
   * \param[in,out] printer Printer
   */
  void Oneof::
  GenerateAccessors(Printer *printer) const {
    assert(printer);
    printer->Print(variables_,
      "/* `signature` : case */\n"
      "PB_INLINE pb_tag_t\n"
      "`message`_case_`name`(\n"
      "    pb_message_t *message) {\n"
      "  assert(pb_message_descriptor(message) == \n"
      "    &`message`_descriptor);\n"
      "  pb_oneof_t oneof = pb_oneof_create(\n"
      "    &`message`_descriptor, message);\n"
      "  pb_tag_t tag = pb_oneof_case(&oneof);\n"
      "  pb_oneof_destroy(&oneof);\n"
      "  return tag;\n"
      "}\n"
      "\n");
  }
}
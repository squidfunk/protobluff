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

#include <cassert>
#include <map>
#include <string>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>

#include "generator/enum_value.hh"
#include "generator/strutil.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::map;
  using ::std::string;

  using ::google::protobuf::EnumValueDescriptor;
  using ::google::protobuf::io::Printer;

  using ::google::protobuf::SimpleItoa;
  using ::google::protobuf::StringReplace;
  using ::google::protobuf::UpperString;

  /*!
   * Create an enum value generator.
   *
   * \param[in] descriptor Enum value descriptor
   */
  EnumValue::
  EnumValue(const EnumValueDescriptor *descriptor) :
      descriptor_(descriptor) {

    /* Extract full name for signature */
    variables_["signature"] = descriptor_->full_name();

    /* Extract number */
    variables_["number"] = SimpleItoa(descriptor_->number());

    /* Prepare descriptor variables */
    variables_["descriptor.number"] = variables_["number"];
    variables_["descriptor.name"]   = descriptor_->name();

    /* Prepare value variables */
    variables_["value.number"] = variables_["number"];
    variables_["value.name"]   = StringReplace(
      variables_["signature"], ".", "_", true);
    UpperString(&(variables_["value.name"]));
  }

  /*!
   * Generate descriptor.
   *
   * \param[in,out] printer Printer
   */
  void EnumValue::
  GenerateDescriptor(Printer *printer) const {
    assert(printer);
    printer->Print(variables_,
      "\n"
      "/* `signature` = `number` */\n"
      "{ .number = `descriptor.number`,\n"
      "  .name   = \"`descriptor.name`\" }");
  }

  /*!
   * Generate value.
   *
   * \param[in,out] printer Printer
   */
  void EnumValue::
  GenerateValue(Printer *printer) const {
    assert(printer);
    printer->Print(variables_,
      "#define `value.name` `value.number`\n");
  }

  /*!
   * Comparator for enum value generators.
   *
   * \param[in] x Enum value generator
   * \param[in] y Enum value generator
   * \return      Test result
   */
  bool
  EnumValueComparator(const EnumValue *x, const EnumValue *y) {
    return x->descriptor_->number() < y->descriptor_->number();
  }
}
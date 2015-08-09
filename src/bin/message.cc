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

#include <map>
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

#include "bin/field.hh"
#include "bin/message.hh"
#include "bin/strutil.hh"

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
  using ::google::protobuf::scoped_array;
  using ::google::protobuf::scoped_ptr;

  using ::google::protobuf::LowerString;
  using ::google::protobuf::SimpleItoa;
  using ::google::protobuf::StringReplace;

  /*!
   * Create a message generator.
   *
   * \param[in] descriptor descriptor
   */
  Message::
  Message(const Descriptor *descriptor) :
      descriptor(descriptor),
      fields(new scoped_ptr<Field>[descriptor->field_count()]),
      nested(new scoped_ptr<Message>[descriptor->nested_type_count()]) {

    /* Initialize fields */
    for (size_t f = 0; f < descriptor->field_count(); f++)
      fields[f].reset(new Field(descriptor->field(f)));

    /* Initialize nested messages */
    for (size_t n = 0; n < descriptor->nested_type_count(); n++)
      nested[n].reset(new Message(descriptor->nested_type(n)));
  }

  /*!
   * Generate declarations.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateDeclarations(Printer *printer) const {
    map<string, string> variables;

    /* Extract full name for signature */
    variables["signature"] = descriptor->full_name();

    /* Extract and prepare message name */
    variables["message"] = StringReplace(
      descriptor->full_name(), ".", "_", true);
    LowerString(&(variables["message"]));

    /* Generate forward declaration */
    printer->Print(variables,
      "/* `signature` */\n"
      "extern const pb_message_descriptor_t\n"
      "`message`_descriptor;\n"
      "\n");

    /* Generate forward declarations for nested messages */
    for (size_t n = 0; n < descriptor->nested_type_count(); n++)
      nested[n]->GenerateDeclarations(printer);
  }

  /*!
   * Generate defaults.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateDefaults(Printer *printer) const {
    for (size_t f = 0; f < descriptor->field_count(); f++)
      if (descriptor->field(f)->has_default_value())
        fields[f]->GenerateDefault(printer);

    /* Generate defaults for nested messages */
    for (size_t n = 0; n < descriptor->nested_type_count(); n++)
      nested[n]->GenerateDefaults(printer);
  }

  /*!
   * Generate descriptors.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateDescriptors(Printer *printer) const {
    map<string, string> variables;

    /* Extract full name for signature */
    variables["signature"] = descriptor->full_name();

    /* Extract and prepare message name */
    variables["message"] = StringReplace(
      descriptor->full_name(), ".", "_", true);
    LowerString(&(variables["message"]));

    /* Generate descriptor header */
    if (descriptor->field_count()) {
      printer->Print(variables,
        "/* message `signature` */\n"
        "const pb_message_descriptor_t\n"
        "`message`_descriptor = { {\n"
        "  (const pb_field_descriptor_t []){\n");

      /* Generate field descriptors */
      for (size_t i = 0; i < 2; i++)
        printer->Indent();
      for (size_t f = 0; f < descriptor->field_count(); f++) {
        fields[f]->GenerateDescriptor(printer);
        if (f < descriptor->field_count() - 1)
          printer->Print(",");
        printer->Print("\n");
      }
      for (size_t i = 0; i < 2; i++)
        printer->Outdent();

      /* Generate descriptor footer */
      printer->Print(
        "\n"
        "  }, `fields` } };\n"
        "\n", "fields", SimpleItoa(descriptor->field_count()));

    /* Print empty descriptor, if message contains no fields */
    } else {
      printer->Print(variables,
        "/* message `signature` */\n"
        "const pb_message_descriptor_t\n"
        "`message`_descriptor = {};\n"
        "\n");
    }

    /* Generate descriptors for nested messages */
    for (size_t n = 0; n < descriptor->nested_type_count(); n++)
      nested[n]->GenerateDescriptors(printer);
  }

  /*!
   * Generate descriptor assertions.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateDescriptorAssertions(Printer *printer) const {
    map<string, string> variables;

    /* Extract full name for signature */
    variables["signature"] = descriptor->full_name();

    /* Extract and prepare message name */
    variables["message"] = StringReplace(
      descriptor->full_name(), ".", "_", true);
    LowerString(&(variables["message"]));

    /* Generate descriptor assertion */
    printer->Print(variables,
      "/* `signature` : descriptor */\n"
      "#define __`message`_descriptor_assert(descriptor) \\\n"
      "  (assert((descriptor) && pb_message_descriptor(descriptor) == \\\n"
      "    &`message`_descriptor))\n"
      "\n");

    /* Generate descriptor assertions for nested messages */
    for (size_t n = 0; n < descriptor->nested_type_count(); n++)
      nested[n]->GenerateDescriptorAssertions(printer);
  }

  /*!
   * Generate definitions.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateDefinitions(Printer *printer) const {
    map<string, string> variables;

    /* Extract full name for signature */
    variables["signature"] = descriptor->full_name();

    /* Extract and prepare message name */
    variables["message"] = StringReplace(
      descriptor->full_name(), ".", "_", true);
    LowerString(&(variables["message"]));

    /* Generate constructor */
    printer->Print(variables,
      "/* `signature` : create */\n"
      "#define `message`_create(binary) \\\n"
      "  (assert(binary), pb_message_create( \\\n"
      "    &`message`_descriptor, (binary)))\n"
      "\n");

    /* Generate constructor for byte fields */
    printer->Print(variables,
      "/* `signature` : create from bytes */\n"
      "#define `message`_create_from_bytes(field) \\\n"
      "  (assert(field), pb_message_create_from_bytes( \\\n"
      "    &`message`_descriptor, (field)))\n"
      "\n");

    /* Generate destructor */
    printer->Print(variables,
      "/* `signature` : destroy */\n"
      "#define `message`_destroy(message) \\\n"
      "  (__`message`_descriptor_assert(message), \\\n"
      "    (pb_message_destroy(message)))\n"
      "\n");

    /* Generate definitions for fields */
    for (size_t f = 0; f < descriptor->field_count(); f++)
      fields[f]->GenerateDefinitions(printer);

    /* Generate definitions for nested messages */
    for (size_t n = 0; n < descriptor->nested_type_count(); n++)
      nested[n]->GenerateDefinitions(printer);
  }

  /*!
   * Generate nested definitions.
   *
   * The trace is used to keep track of the fields that are involved from the
   * uppermost level to the definition of the underlying message.
   *
   * \param[in,out] printer Printer
   * \param[in,out] trace   Trace
   */
  void Message::
  GenerateDefinitions(
      Printer *printer, vector<const FieldDescriptor *> &trace) const {
    for (size_t f = 0; f < descriptor->field_count(); f++)
      fields[f]->GenerateDefinitions(printer, trace);
  }

  /*!
   * Check whether a message or its nested message have defaults.
   *
   * \return Test result
   */
  bool Message::
  HasDefaults() const {
    for (size_t f = 0; f < descriptor->field_count(); f++)
      if (descriptor->field(f)->has_default_value())
        return true;

    /* Check defaults for nested messages */
    for (size_t n = 0; n < descriptor->nested_type_count(); n++)
      if (nested[n]->HasDefaults())
        return true;

    /* No defaults */
    return false;
  }
}
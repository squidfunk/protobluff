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
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

#include "bin/enum.hh"
#include "bin/field.hh"
#include "bin/message.hh"
#include "bin/strutil.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::cerr;
  using ::std::endl;
  using ::std::map;
  using ::std::string;
  using ::std::vector;

  using ::google::protobuf::Descriptor;
  using ::google::protobuf::EnumDescriptor;
  using ::google::protobuf::EnumValueDescriptor;
  using ::google::protobuf::FieldDescriptor;
  using ::google::protobuf::io::Printer;
  using ::google::protobuf::scoped_ptr;

  using ::google::protobuf::HasPrefixString;
  using ::google::protobuf::JoinStrings;
  using ::google::protobuf::LowerString;
  using ::google::protobuf::SimpleDtoa;
  using ::google::protobuf::SimpleFtoa;
  using ::google::protobuf::SimpleItoa;
  using ::google::protobuf::StringReplace;
  using ::google::protobuf::StripPrefixString;
  using ::google::protobuf::StripSuffixString;
  using ::google::protobuf::UpperString;

  /*!
   * Create a field generator.
   *
   * \param[in] descriptor Field descriptor
   */
  Field::
  Field(const FieldDescriptor *descriptor) :
      descriptor_(descriptor) {

    /* Prefix scope to identifiers, if given */
    string prefix ("");
    if (descriptor_->extension_scope()) {
      const Descriptor *scope = descriptor_->extension_scope();
      prefix = scope->full_name();

      /* Check if the base and extension types are in the same package */
      if (!scope->file()->package().compare(descriptor_->file()->package()))
        prefix = "[" + StripPrefixString(prefix,
          scope->file()->package() + ".") + "].";
    }

    /* Extract full name for signature */
    variables_["signature"] = descriptor_->is_extension()
      ? descriptor_->containing_type()->full_name()
        + "." + prefix + descriptor_->name()
      : descriptor_->full_name();

    /* Strip parenthesis from prefix */
    prefix = StringReplace(prefix, "[", "", true);
    prefix = StringReplace(prefix, "]", "", true);

    /* Extract tag, name and type */
    variables_["tag"]  = SimpleItoa(descriptor_->number());
    variables_["name"] = prefix + descriptor_->name();
    variables_["type"] = descriptor_->type_name();

    /* Prepare label */
    switch (descriptor_->label()) {
      case FieldDescriptor::LABEL_REQUIRED:
        variables_["label"] = "required";
        break;
      case FieldDescriptor::LABEL_OPTIONAL:
        variables_["label"] = "optional";
        break;
      case FieldDescriptor::LABEL_REPEATED:
        variables_["label"] = "repeated";
        break;
    }

    /* Extract default definition */
    if (descriptor_->has_default_value() || descriptor_->enum_type()) {
      vector<string> parts;

      /* Push name of containing type and variable to parts */
      parts.push_back(StringReplace(
        descriptor_->containing_type()->full_name(), ".", "_", true));
      parts.push_back(StringReplace(variables_["name"], ".", "_", true));
      for (size_t f = 0; f < parts.size(); f++)
        LowerString(&(parts.at(f)));

      /* Prepare default symbol */
      variables_["default.symbol"] = JoinStrings(parts,
        descriptor_->is_extension() ? "_X_" : "_");

      /* Extract default type and value */
      switch (descriptor_->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32:
          variables_["default.type"]  = "int32_t";
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_int32()) + "L";
          break;
        case FieldDescriptor::CPPTYPE_INT64:
          variables_["default.type"]  = "int64_t";
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_int64()) + "LL";
          break;
        case FieldDescriptor::CPPTYPE_UINT32:
          variables_["default.type"]  = "uint32_t";
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_uint32()) + "UL";
          break;
        case FieldDescriptor::CPPTYPE_UINT64:
          variables_["default.type"]  = "uint64_t";
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_uint64()) + "ULL";
          break;
        case FieldDescriptor::CPPTYPE_FLOAT:
          variables_["default.type"]  = "float";
          variables_["default.value"] = SimpleFtoa(
            descriptor_->default_value_float());
          break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
          variables_["default.type"]  = "double";
          variables_["default.value"] = SimpleDtoa(
            descriptor_->default_value_double());
          break;
        case FieldDescriptor::CPPTYPE_BOOL:
          variables_["default.type"]  = "uint8_t";
          variables_["default.value"] =
            descriptor_->default_value_bool() ? "1" : "0";
          break;
        case FieldDescriptor::CPPTYPE_ENUM:
          variables_["default.type"]  = "pb_enum_t";
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_enum()->number()) + "L";
          break;
        case FieldDescriptor::CPPTYPE_STRING:
          variables_["default.type"]  = "pb_string_t";
          variables_["default.value"] = "pb_string_const(\""
            + descriptor_->default_value_string() + "\"";
          if (descriptor_->type() == FieldDescriptor::TYPE_BYTES)
            variables_["default.value"] += ", " + SimpleItoa(
              descriptor_->default_value_string().length());
          variables_["default.value"] += ")";
          break;
        default:
          return;
      }
    }

    /* Prepare descriptor variables */
    variables_["descriptor.tag"]   = variables_["tag"];
    variables_["descriptor.name"]  = variables_["name"];
    variables_["descriptor.type"]  = variables_["type"];
    variables_["descriptor.label"] = variables_["label"];
    UpperString(&(variables_["descriptor.type"]));
    UpperString(&(variables_["descriptor.label"]));

    /* Extract enum type name */
    if (descriptor_->enum_type()) {
      variables_["type"] = descriptor_->enum_type()->full_name();

      /* Prepare descriptor symbol */
      variables_["descriptor.symbol"] = StringReplace(
        variables_["type"], ".", "_", true);
      LowerString(&(variables_["descriptor.symbol"]));
    }

    /* Extract nested message type name */
    if (descriptor_->message_type()) {
      variables_["type"] = descriptor_->message_type()->full_name();

      /* Prepare descriptor symbol */
      variables_["descriptor.symbol"] = StringReplace(
        variables_["type"], ".", "_", true);
      LowerString(&(variables_["descriptor.symbol"]));
    }

    /* Prepare definition variables */
    variables_["define.tag"] = variables_["tag"];
    variables_["define.name"] = StringReplace(
      variables_["name"], ".", "_", true);
    LowerString(&(variables_["define.name"]));

    /* Prepare definition symbol */
    variables_["define.symbol"] = StringReplace(
      descriptor_->containing_type()->full_name(), ".", "_", true);
    LowerString(&(variables_["define.symbol"]));

    /* Prepend extension marker to name */
    if (descriptor_->is_extension())
      variables_["define.name"] = "X_" + variables_["define.name"];
  }

  /*!
   * Check whether a field has a default value.
   *
   * \return Test result
   */
  bool Field::
  HasDefault() const {
    return descriptor_->has_default_value() || descriptor_->enum_type();
  }

  /*!
   * Generate default value.
   *
   * \param[in,out] printer Printer
   */
  void Field::
  GenerateDefault(Printer *printer) const {
    assert(printer);
    if (descriptor_->has_default_value() || descriptor_->enum_type()) {
      printer->Print(variables_,
        "/* `signature` : default */\n"
        "static const `default.type`\n"
        "`default.symbol`_default = `default.value`;\n"
        "\n");
    }
  }

  /*!
   * Generate descriptor.
   *
   * \param[in,out] printer Printer
   */
  void Field::
  GenerateDescriptor(Printer *printer) const {
    assert(printer);

    /* Emit warning for deprecated field */
    if (descriptor_->options().deprecated())
      cerr << descriptor_->file()->name()
           << ": WARNING - \""
           << descriptor_->name() << "\" in \""
           << descriptor_->containing_type()->full_name()
           << "\" is deprecated." << endl;

    /* Generate field descriptor for nested message */
    if (descriptor_->message_type()) {
      printer->Print(variables_,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `descriptor.tag`,\n"
        "  .name  = \"`descriptor.name`\",\n"
        "  .type  = `descriptor.type`,\n"
        "  .label = `descriptor.label`,\n"
        "  .refer = &`descriptor.symbol`_descriptor }");

    /* Generate field descriptor for enum */
    } else if (descriptor_->enum_type()) {
      printer->Print(variables_,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `descriptor.tag`,\n"
        "  .name  = \"`descriptor.name`\",\n"
        "  .type  = `descriptor.type`,\n"
        "  .label = `descriptor.label`,\n"
        "  .refer = &`descriptor.symbol`_descriptor,\n"
        "  .value = &`default.symbol`_default }");

    /* Generate field descriptor with default value */
    } else if (descriptor_->has_default_value()) {
      printer->Print(variables_,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `descriptor.tag`,\n"
        "  .name  = \"`descriptor.name`\",\n"
        "  .type  = `descriptor.type`,\n"
        "  .label = `descriptor.label`,\n"
        "  .value = &`default.symbol`_default }");

    /* Generate field descriptor */
    } else {
      printer->Print(variables_,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `descriptor.tag`,\n"
        "  .name  = \"`descriptor.name`\",\n"
        "  .type  = `descriptor.type`,\n"
        "  .label = `descriptor.label` }");
    }
  }

  /*!
   * Generate definitions.
   *
   * \param[in,out] printer Printer
   */
  void Field::
  GenerateDefinitions(Printer *printer) const {
    assert(printer);

    /* Generate definitions for required fields */
    if (!descriptor_->is_required())
      printer->Print(variables_,
        "/* `signature` : has */\n"
        "#define `define.symbol`_has_`define.name`(message) \\\n"
        "  (`define.symbol`_descriptor_assert(message), \\\n"
        "    (pb_message_has((message), `define.tag`)))\n"
        "\n");

    /* Generate definitions for nested messages */
    if (descriptor_->message_type()) {
      printer->Print(variables_,
        "/* `signature` : create */\n"
        "#define `define.symbol`_create_`define.name`(message) \\\n"
        "  (`define.symbol`_descriptor_assert(message), \\\n"
        "    (pb_message_create_within((message), `define.tag`)))\n"
        "\n");

      /* Generate definitions for repeated nested messages */
      if (descriptor_->is_repeated())
        printer->Print(variables_,
          "/* `signature` : cursor.create */\n"
          "#define `define.symbol`_create_`define.name`_cursor(message) \\\n"
          "  (`define.symbol`_descriptor_assert(message), \\\n"
          "    (pb_cursor_create((message), `define.tag`)))\n"
          "\n");

      /* Create a trace to generate definitions for nested messages */
      if (ShouldTrace()) {
        vector<const FieldDescriptor *> trace;
        trace.push_back(descriptor_);

        /* Follow trace for corresponding message generator */
        scoped_ptr<Message> temp(new Message(descriptor_->message_type()));
        temp->GenerateDefinitions(printer, trace);
      }

    /* Generate definitions for primitive fields */
    } else {
      if (!descriptor_->is_repeated())
        printer->Print(variables_,
          "/* `signature` : get */\n"
          "#define `define.symbol`_get_`define.name`(message, value) \\\n"
          "  (`define.symbol`_descriptor_assert(message), \\\n"
          "    (pb_message_get((message), `define.tag`, (value))))\n"
          "\n");
      printer->Print(variables_,
        "/* `signature` : put */\n"
        "#define `define.symbol`_put_`define.name`(message, value) \\\n"
        "  (`define.symbol`_descriptor_assert(message), \\\n"
        "    (pb_message_put((message), `define.tag`, (value))))\n"
        "\n");

      /* Generate definitions for enum fields */
      if (descriptor_->enum_type()) {
        const EnumDescriptor *descriptor = descriptor_->enum_type();
        for (size_t v = 0; v < descriptor->value_count(); v++) {
          const EnumValueDescriptor *value = descriptor->value(v);
          map<string, string> variables (variables_);

          /* Lowercase enum value name */
          string name = value->name();
          LowerString(&name);

          /* Prepare definition variables */
          variables["define.name"] += "_" + name;
          variables["define.value"] = "(const pb_enum_t []){ "
            + SimpleItoa(value->number()) +
          " }";

          /* Extract enum value signature */
          variables["enum.signature"] = value->name();

          /* Generate definitions for enum value */
          printer->Print(variables,
            "/* `signature` : has(`enum.signature`) */\n"
            "#define `define.symbol`_has_`define.name`(message) \\\n"
            "  (`define.symbol`_descriptor_assert(message), \\\n"
            "    (pb_message_match((message), `define.tag`, \\\n"
            "      `define.value`)))\n"
            "\n"
            "/* `signature` : put(`enum.signature`) */\n"
            "#define `define.symbol`_put_`define.name`(message) \\\n"
            "  (`define.symbol`_descriptor_assert(message), \\\n"
            "    (pb_message_put((message), `define.tag`, \\\n"
            "      `define.value`)))\n"
            "\n");
        }
      }
    }

    /* Generate definitions for optional fields */
    if (descriptor_->is_optional())
      printer->Print(variables_,
        "/* `signature` : erase */\n"
        "#define `define.symbol`_erase_`define.name`(message) \\\n"
        "  (`define.symbol`_descriptor_assert(message), \\\n"
        "    (pb_message_erase((message), `define.tag`)))\n"
        "\n");

    /* Generate definitions for fixed-sized fields */
    switch (descriptor_->type()) {
      case FieldDescriptor::TYPE_FIXED32:
      case FieldDescriptor::TYPE_FIXED64:
      case FieldDescriptor::TYPE_SFIXED32:
      case FieldDescriptor::TYPE_SFIXED64:
      case FieldDescriptor::TYPE_FLOAT:
      case FieldDescriptor::TYPE_DOUBLE:
        printer->Print(variables_,
          "/* `signature` : raw */\n"
          "#define `define.message`_raw_`define.name`(message) \\\n"
          "  (`define.message`_descriptor_assert(message), \\\n"
          "    (pb_message_raw((message), `define.tag`)))\n"
          "\n");
        break;
      default:
        break;
    }
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
  void Field::
  GenerateDefinitions(
      Printer *printer, vector<const FieldDescriptor *> &trace) const {
    assert(printer);
    map<string, string> variables (variables_);

    /* Don't create nested definitions for extensions */
    if (trace.back()->is_extension())
      return;

    /* Extract tags and names from trace */
    vector<string> tags, name;
    for (size_t f = 0; f < trace.size(); f++) {
      tags.push_back(SimpleItoa(trace.at(f)->number()));
      name.push_back(trace.at(f)->lowercase_name());
    }

    /* Push current tag and name to vector */
    tags.push_back(SimpleItoa(descriptor_->number()));
    name.push_back(descriptor_->lowercase_name());

    /* Prepare definition variables */
    variables["define.name"] = JoinStrings(name, "_");
    variables["define.tag"]  = "(const pb_tag_t []){ "
      + JoinStrings(tags, ", ") +
    " }, " + SimpleItoa(tags.size());

    /* Prepare definition symbol */
    variables["define.symbol"] = StringReplace(
      StripSuffixString(trace.front()->full_name(),
        "." + trace.front()->name()), ".", "_", true);
    LowerString(&(variables["define.symbol"]));

    /* Generate definitions for required fields */
    if (!descriptor_->is_required())
      printer->Print(variables,
        "/* `signature` : has */\n"
        "#define `define.symbol`_has_`define.name`(message) \\\n"
        "  (`define.symbol`_descriptor_assert(message), \\\n"
        "    (pb_message_nested_has((message), \\\n"
        "      `define.tag`)))\n"
        "\n");

    /* Generate definitions for nested messages */
    if (descriptor_->message_type()) {
      printer->Print(variables,
        "/* `signature` : create */\n"
        "#define `define.symbol`_create_`define.name`(message) \\\n"
        "  (`define.symbol`_descriptor_assert(message), \\\n"
        "    (pb_message_create_nested((message), \\\n"
        "      `define.tag`)))\n"
        "\n");

      /* Generate definitions for repeated nested messages */
      if (descriptor_->is_repeated())
        printer->Print(variables,
          "/* `signature` : cursor.create */\n"
          "#define `define.symbol`_create_`define.name`_cursor(message) \\\n"
          "  (`define.symbol`_descriptor_assert(message), \\\n"
          "    (pb_cursor_create_nested((message), \\\n"
          "      `define.tag`)))\n"
          "\n");

      /* Push current descriptor to trace */
      if (ShouldTrace()) {
        const Descriptor *message = descriptor_->message_type();
        trace.push_back(descriptor_);

        /* Follow trace for corresponding message generator */
        scoped_ptr<Message> temp(new Message(message));
        temp->GenerateDefinitions(printer, trace);
        trace.pop_back();
      }

    /* Generate definitions for primitive fields */
    } else {
      if (!descriptor_->is_repeated())
        printer->Print(variables,
          "/* `signature` : get */\n"
          "#define `define.symbol`_get_`define.name`(message, value) \\\n"
          "  (`define.symbol`_descriptor_assert(message), \\\n"
          "    (pb_message_nested_get((message), \\\n"
          "      `define.tag`, (value))))\n"
          "\n");
      printer->Print(variables,
        "/* `signature` : put */\n"
        "#define `define.symbol`_put_`define.name`(message, value) \\\n"
        "  (`define.symbol`_descriptor_assert(message), \\\n"
        "    (pb_message_nested_put((message), \\\n"
        "      `define.tag`, (value))))\n"
        "\n");

      /* Generate definitions for enum fields */
      if (descriptor_->enum_type()) {
        const EnumDescriptor *descriptor = descriptor_->enum_type();
        for (size_t v = 0; v < descriptor->value_count(); v++) {
          const EnumValueDescriptor *value = descriptor->value(v);
          map<string, string> variables (variables_);

          /* Lowercase enum value name */
          string name = value->name();
          LowerString(&name);

          /* Prepare definition variables */
          variables["define.name"] += "_" + name;
          variables["define.value"] = "(const pb_enum_t []){ "
            + SimpleItoa(value->number()) +
          " }";

          /* Extract enum value signature */
          variables["enum.signature"] = value->name();

          /* Generate definitions for enum value */
          printer->Print(variables,
            "/* `signature` : has(`enum.signature`) */\n"
            "#define `define.symbol`_has_`define.name`(message) \\\n"
            "  (`define.symbol`_descriptor_assert(message), \\\n"
            "    (pb_message_match((message), \\\n"
            "      `define.tag`, `define.value`)))\n"
            "\n"
            "/* `signature` : put(`enum.signature`) */\n"
            "#define `define.symbol`_put_`define.name`(message) \\\n"
            "  (`define.symbol`_descriptor_assert(message), \\\n"
            "    (pb_message_put((message), \\\n"
            "      `define.tag`, `define.value`)))\n"
            "\n");
        }
      }
    }

    /* Generate definitions for optional fields */
    if (descriptor_->is_optional())
      printer->Print(variables,
        "/* `signature` : erase */\n"
        "#define `define.symbol`_erase_`define.name`(message) \\\n"
        "  (`define.symbol`_descriptor_assert(message), \\\n"
        "    (pb_message_nested_erase((message), \\\n"
        "      `define.tag`)))\n"
        "\n");

    /* Generate definitions for fixed-sized fields */
    switch (descriptor_->type()) {
      case FieldDescriptor::TYPE_FIXED32:
      case FieldDescriptor::TYPE_FIXED64:
      case FieldDescriptor::TYPE_SFIXED32:
      case FieldDescriptor::TYPE_SFIXED64:
      case FieldDescriptor::TYPE_FLOAT:
      case FieldDescriptor::TYPE_DOUBLE:
        printer->Print(variables,
          "/* `signature` : raw */\n"
          "#define `define.symbol`_raw_`define.name`(message) \\\n"
          "  (`define.symbol`_descriptor_assert(message), \\\n"
          "    (pb_message_nested_raw((message), \\\n"
          "      `define.tag`)))\n"
          "\n");
        break;
      default:
        break;
    }
  }

  /*!
   * Check whether a field's nested message type should be traced.
   *
   * \return Test result
   */
  bool Field::
  ShouldTrace() const {
    assert(descriptor_->message_type());
    return !descriptor_->is_repeated() && HasPrefixString(
      descriptor_->message_type()->full_name(),
      descriptor_->containing_type()->full_name());
  }

  /*!
   * Comparator for field generators.
   *
   * \param[in] x Field generator
   * \param[in] y Field generator
   * \return      Test result
   */
  bool
  FieldComparator(const Field *x, const Field *y) {
    return x->descriptor_->number() < y->descriptor_->number();
  }
}
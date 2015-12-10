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

#include "generator/enum.hh"
#include "generator/field.hh"
#include "generator/message.hh"
#include "generator/strutil.hh"

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

    /* Extract native type */
    switch (descriptor_->cpp_type()) {
      case FieldDescriptor::CPPTYPE_INT32:
        variables_["cpp_type"] = "int32_t";
        break;
      case FieldDescriptor::CPPTYPE_INT64:
        variables_["cpp_type"] = "int64_t";
        break;
      case FieldDescriptor::CPPTYPE_UINT32:
        variables_["cpp_type"] = "uint32_t";
        break;
      case FieldDescriptor::CPPTYPE_UINT64:
        variables_["cpp_type"] = "uint64_t";
        break;
      case FieldDescriptor::CPPTYPE_FLOAT:
        variables_["cpp_type"] = "float";
        break;
      case FieldDescriptor::CPPTYPE_DOUBLE:
        variables_["cpp_type"] = "double";
        break;
      case FieldDescriptor::CPPTYPE_BOOL:
        variables_["cpp_type"] = "uint8_t";
        break;
      case FieldDescriptor::CPPTYPE_ENUM:
        variables_["cpp_type"] = "pb_enum_t";
        break;
      case FieldDescriptor::CPPTYPE_STRING:
        variables_["cpp_type"] = "pb_string_t";
        break;
      case FieldDescriptor::CPPTYPE_MESSAGE:
        variables_["cpp_type"] = "pb_encoder_t";
        break;
      default:
        break;
    }

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

    /* Extract defaults */
    if (HasDefault()) {
      vector<string> parts;

      /* Push name of containing type and variable to parts */
      parts.push_back(StringReplace(
        descriptor_->containing_type()->full_name(), ".", "_", true));
      parts.push_back(StringReplace(variables_["name"], ".", "_", true));
      for (size_t f = 0; f < parts.size(); f++)
        LowerString(&(parts.at(f)));

      /* Prepare default type and symbol */
      variables_["default.type"]   = variables_["cpp_type"];
      variables_["default.symbol"] = JoinStrings(parts,
        descriptor_->is_extension() ? "_X_" : "_");

      /* Extract default values */
      switch (descriptor_->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32:
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_int32());
          break;
        case FieldDescriptor::CPPTYPE_INT64:
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_int64()) + "LL";
          break;
        case FieldDescriptor::CPPTYPE_UINT32:
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_uint32()) + "U";
          break;
        case FieldDescriptor::CPPTYPE_UINT64:
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_uint64()) + "ULL";
          break;
        case FieldDescriptor::CPPTYPE_FLOAT:
          variables_["default.value"] = SimpleFtoa(
            descriptor_->default_value_float());
          break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
          variables_["default.value"] = SimpleDtoa(
            descriptor_->default_value_double());
          break;
        case FieldDescriptor::CPPTYPE_BOOL:
          variables_["default.value"] =
            descriptor_->default_value_bool() ? "1" : "0";
          break;
        case FieldDescriptor::CPPTYPE_ENUM:
          variables_["default.value"] = SimpleItoa(
            descriptor_->default_value_enum()->number());
          break;
        case FieldDescriptor::CPPTYPE_STRING:
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

    /* Prepare accessor variables */
    variables_["define.tag"]  = variables_["tag"];
    variables_["define.type"] = variables_["cpp_type"];
    variables_["define.name"] = StringReplace(
      variables_["name"], ".", "_", true);
    LowerString(&(variables_["define.name"]));

    /* Prepare accessor symbol */
    variables_["define.symbol"] = StringReplace(
      descriptor_->containing_type()->full_name(), ".", "_", true);
    LowerString(&(variables_["define.symbol"]));

    /* Prepend extension marker to name */
    if (descriptor_->is_extension())
      variables_["define.name"] = "X_" + variables_["define.name"];

    /* Emit warning if field is deprecated */
    variables_["define.deprecated"] =
      descriptor_->options().deprecated()
        ? "PB_DEPRECATED\n" : "";
  }

  /*!
   * Check whether a field has a default value.
   *
   * \return Test result
   */
  bool Field::
  HasDefault() const {
    return descriptor_->has_default_value() ||
      (descriptor_->enum_type() && descriptor_->is_optional());
  }

  /*!
   * Generate default value.
   *
   * \param[in,out] printer Printer
   */
  void Field::
  GenerateDefault(Printer *printer) const {
    assert(printer);
    if (HasDefault()) {
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

    /* Generate field descriptor for nested message */
    if (descriptor_->message_type()) {
      printer->Print(variables_,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `descriptor.tag`,\n"
        "  .name  = \"`descriptor.name`\",\n"
        "  .type  = `descriptor.type`,\n"
        "  .label = `descriptor.label`,\n"
        "  .refer = &`descriptor.symbol`_descriptor");

    /* Generate field descriptor for enum */
    } else if (descriptor_->enum_type()) {
      if (descriptor_->is_optional()) {
        printer->Print(variables_,
          "\n"
          "/* `label` `type` `name` = `tag` */\n"
          "{ .tag   = `descriptor.tag`,\n"
          "  .name  = \"`descriptor.name`\",\n"
          "  .type  = `descriptor.type`,\n"
          "  .label = `descriptor.label`,\n"
          "  .refer = &`descriptor.symbol`_descriptor,\n"
          "  .value = &`default.symbol`_default");
      } else {
        printer->Print(variables_,
          "\n"
          "/* `label` `type` `name` = `tag` */\n"
          "{ .tag   = `descriptor.tag`,\n"
          "  .name  = \"`descriptor.name`\",\n"
          "  .type  = `descriptor.type`,\n"
          "  .label = `descriptor.label`,\n"
          "  .refer = &`descriptor.symbol`_descriptor");
      }

    /* Generate field descriptor with default value */
    } else if (descriptor_->has_default_value()) {
      printer->Print(variables_,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `descriptor.tag`,\n"
        "  .name  = \"`descriptor.name`\",\n"
        "  .type  = `descriptor.type`,\n"
        "  .label = `descriptor.label`,\n"
        "  .value = &`default.symbol`_default");

    /* Generate field descriptor */
    } else {
      printer->Print(variables_,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `descriptor.tag`,\n"
        "  .name  = \"`descriptor.name`\",\n"
        "  .type  = `descriptor.type`,\n"
        "  .label = `descriptor.label`");
    }

    /* Add flag for packed fields */
    if (descriptor_->is_packed())
      printer->Print(
        ",\n"
        "  .flags = PACKED");

    /* Terminate initializer */
    printer->Print(" }");
  }

  /*!
   * Generate encoder.
   *
   * \param[in,out] printer Printer
   */
  void Field::
  GenerateEncoder(Printer *printer) const {
    assert(printer);
    printer->Print(variables_,
      "/* `signature` : encode */\n"
      "`define.deprecated`"
      "PB_WARN_UNUSED_RESULT\n"
      "PB_INLINE pb_error_t\n"
      "`define.symbol`_encode_`define.name`(\n"
      "    pb_encoder_t *encoder, const `define.type` *value) {\n"
      "  assert(pb_encoder_descriptor(encoder) == \n"
      "    &`define.symbol`_descriptor);\n"
      "  return pb_encoder_encode(encoder, `define.tag`, value);\n"
      "}\n"
      "\n");

    /* Generate encoder for enum fields */
    if (descriptor_->enum_type()) {
      const EnumDescriptor *descriptor = descriptor_->enum_type();
      for (size_t v = 0; v < descriptor->value_count(); v++) {
        const EnumValueDescriptor *value = descriptor->value(v);
        map<string, string> variables (variables_);

        /* Lowercase enum value name */
        string name = value->name();
        LowerString(&name);

        /* Prepare encoder variables */
        variables["define.name"] += "_" + name;
        variables["define.value"] = "(const pb_enum_t []){ "
          + SimpleItoa(value->number()) +
        " }";

        /* Extract enum value signature */
        variables["enum.signature"] = value->name();

        /* Emit warning if enum value is deprecated */
        if (descriptor->options().deprecated() ||
            value->options().deprecated())
          variables["define.deprecated"] = "PB_DEPRECATED\n";

        /* Generate accessors for enum value */
        printer->Print(variables,
          "/* `signature` : encode(`enum.signature`) */\n"
          "`define.deprecated`"
          "PB_WARN_UNUSED_RESULT\n"
          "PB_INLINE pb_error_t\n"
          "`define.symbol`_encode_`define.name`(\n"
          "    pb_encoder_t *encoder) {\n"
          "  assert(pb_encoder_descriptor(encoder) == \n"
          "    &`define.symbol`_descriptor);\n"
          "  return pb_encoder_encode(encoder, `define.tag`,\n"
          "    `define.value`);\n"
          "}\n"
          "\n");
      }
    }
  }

  /*!
   * Generate accessors.
   *
   * \param[in,out] printer Printer
   */
  void Field::
  GenerateAccessors(Printer *printer) const {
    assert(printer);

    /* Generate accessors for required fields */
    if (!descriptor_->is_required())
      printer->Print(variables_,
        "/* `signature` : has */\n"
        "`define.deprecated`"
        "PB_INLINE int\n"
        "`define.symbol`_has_`define.name`(\n"
        "    pb_message_t *message) {\n"
        "  assert(pb_message_descriptor(message) == \n"
        "    &`define.symbol`_descriptor);\n"
        "  return pb_message_has(message, `define.tag`);\n"
        "}\n"
        "\n");

    /* Generate accessors for nested messages */
    if (descriptor_->message_type()) {
      printer->Print(variables_,
        "/* `signature` : create */\n"
        "`define.deprecated`"
        "PB_WARN_UNUSED_RESULT\n"
        "PB_INLINE pb_message_t\n"
        "`define.symbol`_create_`define.name`(\n"
        "    pb_message_t *message) {\n"
        "  assert(pb_message_descriptor(message) == \n"
        "    &`define.symbol`_descriptor);\n"
        "  return pb_message_create_within(message, `define.tag`);\n"
        "}\n"
        "\n");

      /* Generate accessors for repeated nested messages */
      if (descriptor_->is_repeated())
        printer->Print(variables_,
          "/* `signature` : cursor.create */\n"
          "`define.deprecated`"
          "PB_WARN_UNUSED_RESULT\n"
          "PB_INLINE pb_cursor_t\n"
          "`define.symbol`_create_`define.name`_cursor(\n"
          "    pb_message_t *message) {\n"
          "  assert(pb_message_descriptor(message) == \n"
          "    &`define.symbol`_descriptor);\n"
          "  return pb_cursor_create(message, `define.tag`);\n"
          "}\n"
          "\n");

      /* Create a trace to generate accessors for nested messages */
      if (ShouldTrace()) {
        vector<const FieldDescriptor *> trace;
        trace.push_back(descriptor_);

        /* Follow trace for corresponding message generator */
        scoped_ptr<Message> temp(new Message(descriptor_->message_type()));
        temp->GenerateAccessors(printer, trace);
      }

    /* Generate accessors for primitive fields */
    } else {
      if (!descriptor_->is_repeated())
        printer->Print(variables_,
          "/* `signature` : get */\n"
          "`define.deprecated`"
          "PB_WARN_UNUSED_RESULT\n"
          "PB_INLINE pb_error_t\n"
          "`define.symbol`_get_`define.name`(\n"
          "    pb_message_t *message, `define.type` *value) {\n"
          "  assert(pb_message_descriptor(message) == \n"
          "    &`define.symbol`_descriptor);\n"
          "  return pb_message_get(message, `define.tag`, value);\n"
          "}\n"
          "\n");
      printer->Print(variables_,
        "/* `signature` : put */\n"
        "`define.deprecated`"
        "PB_WARN_UNUSED_RESULT\n"
        "PB_INLINE pb_error_t\n"
        "`define.symbol`_put_`define.name`(\n"
        "    pb_message_t *message, const `define.type` *value) {\n"
        "  assert(pb_message_descriptor(message) == \n"
        "    &`define.symbol`_descriptor);\n"
        "  return pb_message_put(message, `define.tag`, value);\n"
        "}\n"
        "\n");

      /* Generate accessors for enum fields */
      if (descriptor_->enum_type()) {
        const EnumDescriptor *descriptor = descriptor_->enum_type();
        for (size_t v = 0; v < descriptor->value_count(); v++) {
          const EnumValueDescriptor *value = descriptor->value(v);
          map<string, string> variables (variables_);

          /* Lowercase enum value name */
          string name = value->name();
          LowerString(&name);

          /* Prepare accessor variables */
          variables["define.name"] += "_" + name;
          variables["define.value"] = "(const pb_enum_t []){ "
            + SimpleItoa(value->number()) +
          " }";

          /* Extract enum value signature */
          variables["enum.signature"] = value->name();

          /* Emit warning if enum value is deprecated */
          if (descriptor->options().deprecated() ||
              value->options().deprecated())
            variables["define.deprecated"] = "PB_DEPRECATED\n";

          /* Generate accessors for enum value */
          printer->Print(variables,
            "/* `signature` : has(`enum.signature`) */\n"
            "`define.deprecated`"
            "PB_INLINE int\n"
            "`define.symbol`_has_`define.name`(\n"
            "    pb_message_t *message) {\n"
            "  assert(pb_message_descriptor(message) == \n"
            "    &`define.symbol`_descriptor);\n"
            "  return pb_message_match(message, `define.tag`,\n"
            "    `define.value`);\n"
            "}\n"
            "\n"
            "/* `signature` : put(`enum.signature`) */\n"
            "`define.deprecated`"
            "PB_WARN_UNUSED_RESULT\n"
            "PB_INLINE pb_error_t\n"
            "`define.symbol`_put_`define.name`(\n"
            "    pb_message_t *message) {\n"
            "  assert(pb_message_descriptor(message) == \n"
            "    &`define.symbol`_descriptor);\n"
            "  return pb_message_put(message, `define.tag`,\n"
            "    `define.value`);\n"
            "}\n"
            "\n");
        }
      }
    }

    /* Generate accessors for optional fields */
    if (descriptor_->is_optional())
      printer->Print(variables_,
        "/* `signature` : erase */\n"
        "`define.deprecated`"
        "PB_WARN_UNUSED_RESULT\n"
        "PB_INLINE pb_error_t\n"
        "`define.symbol`_erase_`define.name`(\n"
        "    pb_message_t *message) {\n"
        "  assert(pb_message_descriptor(message) == \n"
        "    &`define.symbol`_descriptor);\n"
        "  return pb_message_erase(message, `define.tag`);\n"
        "}\n"
        "\n");

    /* Generate accessors for fixed-sized fields */
    switch (descriptor_->type()) {
      case FieldDescriptor::TYPE_FIXED32:
      case FieldDescriptor::TYPE_FIXED64:
      case FieldDescriptor::TYPE_SFIXED32:
      case FieldDescriptor::TYPE_SFIXED64:
      case FieldDescriptor::TYPE_FLOAT:
      case FieldDescriptor::TYPE_DOUBLE:
        printer->Print(variables_,
          "/* `signature` : raw */\n"
          "PB_INLINE `define.type` *\n"
          "`define.symbol`_raw_`define.name`(\n"
          "    pb_message_t *message) {\n"
          "  assert(pb_message_descriptor(message) == \n"
          "    &`define.symbol`_descriptor);\n"
          "  return pb_message_raw(message, `define.tag`);\n"
          "}\n"
          "\n");
        break;
      default:
        break;
    }
  }

  /*!
   * Generate nested accessors.
   *
   * The trace is used to keep track of the fields that are involved from the
   * uppermost level to the accessor of the underlying message.
   *
   * \param[in,out] printer Printer
   * \param[in,out] trace   Trace
   */
  void Field::
  GenerateAccessors(
      Printer *printer, vector<const FieldDescriptor *> &trace) const {
    assert(printer);
    map<string, string> variables (variables_);

    /* Don't create nested accessors for extensions */
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

    /* Prepare accessor variables */
    variables["define.name"] = JoinStrings(name, "_");
    variables["define.tag"]  = "(const pb_tag_t []){ "
      + JoinStrings(tags, ", ") +
    " }, " + SimpleItoa(tags.size());

    /* Prepare accessor symbol */
    variables["define.symbol"] = StringReplace(
      StripSuffixString(trace.front()->full_name(),
        "." + trace.front()->name()), ".", "_", true);
    LowerString(&(variables["define.symbol"]));

    /* Generate accessors for required fields */
    if (!descriptor_->is_required())
      printer->Print(variables,
        "/* `signature` : has */\n"
        "`define.deprecated`"
        "PB_INLINE int\n"
        "`define.symbol`_has_`define.name`(\n"
        "    pb_message_t *message) {\n"
        "  assert(pb_message_descriptor(message) == \n"
        "    &`define.symbol`_descriptor);\n"
        "  return pb_message_nested_has(message,\n"
        "    `define.tag`);\n"
        "}\n"
        "\n");

    /* Generate accessors for nested messages */
    if (descriptor_->message_type()) {
      printer->Print(variables,
        "/* `signature` : create */\n"
        "`define.deprecated`"
        "PB_WARN_UNUSED_RESULT\n"
        "PB_INLINE pb_message_t\n"
        "`define.symbol`_create_`define.name`(\n"
        "    pb_message_t *message) {\n"
        "  assert(pb_message_descriptor(message) == \n"
        "    &`define.symbol`_descriptor);\n"
        "  return pb_message_create_nested(message,\n"
        "    `define.tag`);\n"
        "}\n"
        "\n");

      /* Generate accessors for repeated nested messages */
      if (descriptor_->is_repeated())
        printer->Print(variables,
          "/* `signature` : cursor.create */\n"
          "`define.deprecated`"
          "PB_WARN_UNUSED_RESULT\n"
          "PB_INLINE pb_cursor_t\n"
          "`define.symbol`_create_`define.name`_cursor(\n"
          "    pb_message_t *message) {\n"
          "  assert(pb_message_descriptor(message) == \n"
          "    &`define.symbol`_descriptor);\n"
          "  return pb_cursor_create_nested(message,\n"
          "    `define.tag`);\n"
          "}\n"
          "\n");

      /* Push current descriptor to trace */
      if (ShouldTrace()) {
        const Descriptor *message = descriptor_->message_type();
        trace.push_back(descriptor_);

        /* Follow trace for corresponding message generator */
        scoped_ptr<Message> temp(new Message(message));
        temp->GenerateAccessors(printer, trace);
        trace.pop_back();
      }

    /* Generate accessors for primitive fields */
    } else {
      if (!descriptor_->is_repeated())
        printer->Print(variables,
          "/* `signature` : get */\n"
          "`define.deprecated`"
          "PB_WARN_UNUSED_RESULT\n"
          "PB_INLINE pb_error_t\n"
          "`define.symbol`_get_`define.name`(\n"
          "    pb_message_t *message, `define.type` *value) {\n"
          "  assert(pb_message_descriptor(message) == \n"
          "    &`define.symbol`_descriptor);\n"
          "  return pb_message_nested_get(message,\n"
          "    `define.tag`, value);\n"
          "}\n"
          "\n");
      printer->Print(variables,
        "/* `signature` : put */\n"
        "`define.deprecated`"
        "PB_WARN_UNUSED_RESULT\n"
        "PB_INLINE pb_error_t\n"
        "`define.symbol`_put_`define.name`(\n"
        "    pb_message_t *message, const `define.type` *value) {\n"
        "  assert(pb_message_descriptor(message) == \n"
        "    &`define.symbol`_descriptor);\n"
        "  return pb_message_nested_put(message,\n"
        "    `define.tag`, value);\n"
        "}\n"
        "\n");

      /* Generate accessors for enum fields */
      if (descriptor_->enum_type()) {
        const EnumDescriptor *descriptor = descriptor_->enum_type();
        for (size_t v = 0; v < descriptor->value_count(); v++) {
          const EnumValueDescriptor *value = descriptor->value(v);
          map<string, string> variables (variables_);

          /* Lowercase enum value name */
          string name = value->name();
          LowerString(&name);

          /* Prepare accessor variables */
          variables["define.name"] += "_" + name;
          variables["define.value"] = "(const pb_enum_t []){ "
            + SimpleItoa(value->number()) +
          " }";

          /* Extract enum value signature */
          variables["enum.signature"] = value->name();

          /* Generate accessors for enum value */
          printer->Print(variables,
            "/* `signature` : has(`enum.signature`) */\n"
            "`define.deprecated`"
            "PB_INLINE int\n"
            "`define.symbol`_has_`define.name`(\n"
            "    pb_message_t *message) {\n"
            "  assert(pb_message_descriptor(message) == \n"
            "    &`define.symbol`_descriptor);\n"
            "  return pb_message_nested_match(message,\n"
            "    `define.tag`, `define.value`);\n"
            "}\n"
            "\n"
            "/* `signature` : put(`enum.signature`) */\n"
            "`define.deprecated`"
            "PB_WARN_UNUSED_RESULT\n"
            "PB_INLINE pb_error_t\n"
            "`define.symbol`_put_`define.name`(\n"
            "    pb_message_t *message) {\n"
            "  assert(pb_message_descriptor(message) == \n"
            "    &`define.symbol`_descriptor);\n"
            "  return pb_message_nested_put(message,\n"
            "    `define.tag`, `define.value`);\n"
            "}\n"
            "\n");

        }
      }
    }

    /* Generate accessors for optional fields */
    if (descriptor_->is_optional())
      printer->Print(variables,
        "/* `signature` : erase */\n"
        "`define.deprecated`"
        "PB_WARN_UNUSED_RESULT\n"
        "PB_INLINE pb_error_t\n"
        "`define.symbol`_erase_`define.name`(\n"
        "    pb_message_t *message) {\n"
        "  assert(pb_message_descriptor(message) == \n"
        "    &`define.symbol`_descriptor);\n"
        "  return pb_message_nested_erase(message,\n"
        "    `define.tag`);\n"
        "}\n"
        "\n");

    /* Generate accessors for fixed-sized fields */
    switch (descriptor_->type()) {
      case FieldDescriptor::TYPE_FIXED32:
      case FieldDescriptor::TYPE_FIXED64:
      case FieldDescriptor::TYPE_SFIXED32:
      case FieldDescriptor::TYPE_SFIXED64:
      case FieldDescriptor::TYPE_FLOAT:
      case FieldDescriptor::TYPE_DOUBLE:
        printer->Print(variables,
          "/* `signature` : raw */\n"
          "`define.deprecated`"
          "PB_INLINE `define.type` *\n"
          "`define.symbol`_raw_`define.name`(\n"
          "    pb_message_t *message) {\n"
          "  assert(pb_message_descriptor(message) == \n"
          "    &`define.symbol`_descriptor);\n"
          "  return pb_message_nested_raw(message,\n"
          "    `define.tag`);\n"
          "}\n"
          "\n");
        break;
      default:
        break;
    }
  }

  /*!
   * Check whether a field's nested message type should be traced.
   *
   * In general, nested message types are traced if they adhere to the
   * following three criteria:
   *
   * -# The field is not repeated, as repeated nested messages must be created
   *    using a cursor to ensure proper handling.
   *
   * -# The nested message type is not part of a circular definition, i.e. does
   *    not point to a parent message type.
   *
   * -# The nested message type is defined within the path of the current
   *    message type, meaning within the current or a parent message type.
   *
   * \return Test result
   */
  bool Field::
  ShouldTrace() const {
    assert(descriptor_->message_type());
    if (descriptor_->is_repeated())
      return false;

    /* Check for non-circular message type reference */
    const Descriptor *descriptor = descriptor_->containing_type();
    do {
      if (descriptor_->message_type() == descriptor)
        return false;
    } while ((descriptor = descriptor->containing_type()));

    /* Check if message type is part of a parent message type */
    return HasPrefixString(
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
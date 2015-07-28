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

#include <iostream>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/printer.h>

#include "bin/field.hh"
#include "bin/message.hh"
#include "bin/strutil.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace Protobluff {

  /*!
   * Create a field generator.
   *
   * \param[in] descriptor Field descriptor
   */
  Field::
  Field(const FieldDescriptor *descriptor) :
      descriptor(descriptor) {}

  /*!
   * Generate default value.
   *
   * \param[in,out] printer Printer
   */
  void Field::
  GenerateDefault(io::Printer *printer) const {
    if (descriptor->has_default_value()) {
      map<string, string> variables;

      /* Extract full name for signature */
      variables["signature"] = descriptor->full_name();

      /* Extract specific type and value */
      switch (descriptor->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32:
          variables["type"]  = "int32_t";
          variables["value"] = SimpleItoa(
            descriptor->default_value_int32());
          break;
        case FieldDescriptor::CPPTYPE_INT64:
          variables["type"]  = "int64_t";
          variables["value"] = SimpleItoa(
            descriptor->default_value_int64()) + "L";
          break;
        case FieldDescriptor::CPPTYPE_UINT32:
          variables["type"]  = "uint32_t";
          variables["value"] = SimpleItoa(
            descriptor->default_value_uint32()) + "U";
          break;
        case FieldDescriptor::CPPTYPE_UINT64:
          variables["type"]  = "uint64_t";
          variables["value"] = SimpleItoa(
            descriptor->default_value_uint64()) + "UL";
          break;
        case FieldDescriptor::CPPTYPE_FLOAT:
          variables["type"]  = "float";
          variables["value"] = SimpleFtoa(
            descriptor->default_value_float());
          break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
          variables["type"]  = "double";
          variables["value"] = SimpleDtoa(
            descriptor->default_value_double());
          break;
        case FieldDescriptor::CPPTYPE_BOOL:
          variables["type"]  = "uint8_t";
          variables["value"] = descriptor->default_value_bool() ? "1" : "0";
          break;
        case FieldDescriptor::CPPTYPE_ENUM:
          variables["type"]  = "int32_t";
          variables["value"] = SimpleItoa(
            descriptor->default_value_enum()->number());
          break;
        case FieldDescriptor::CPPTYPE_STRING:
          variables["type"]  = "pb_string_t";
          variables["value"] = "pb_string_const(\""
            + descriptor->default_value_string() + "\"";
          if (descriptor->type() == FieldDescriptor::TYPE_BYTES)
            variables["value"] += ", " + SimpleItoa(
              descriptor->default_value_string().length());
          variables["value"] += ")";
          break;
        default:
          return;
      }

      /* Extract and prepare field name */
      variables["field"] = StringReplace(
        descriptor->full_name(), ".", "_", true);
      LowerString(&(variables["field"]));

      /* Generate definition */
      printer->Print(variables,
        "/* `signature` */\n"
        "static const `type`\n"
        "`field`_default = `value`;\n"
        "\n");
    }
  }

  /*!
   * Generate descriptor.
   *
   * \param[in,out] printer Printer
   */
  void Field::
  GenerateDescriptor(io::Printer *printer) const {
    map<string, string> variables;

    /* Emit warning for deprecated field */
    if (descriptor->options().deprecated())
      std::cerr << descriptor->file()->name()
                << ": WARNING - \""
                << descriptor->name() << "\" in \""
                << descriptor->containing_type()->full_name()
                << "\" is deprecated." << std::endl;

    /* Extract tag, type and name */
    variables["tag"]  = SimpleItoa(descriptor->number());
    variables["type"] = descriptor->type_name();
    variables["name"] = descriptor->name();

    /* Prepare label */
    switch (descriptor->label()) {
      case FieldDescriptor::LABEL_REQUIRED:
        variables["label"] = "required";
        break;
      case FieldDescriptor::LABEL_OPTIONAL:
        variables["label"] = "optional";
        break;
      case FieldDescriptor::LABEL_REPEATED:
        variables["label"] = "repeated";
        break;
    }

    /* Generate constant names for type and label */
    variables["TYPE"]  = variables["type"];
    variables["LABEL"] = variables["label"];
    UpperString(&(variables["TYPE"]));
    UpperString(&(variables["LABEL"]));

    /* Handle enumeration signature */
    if (descriptor->enum_type())
      variables["type"] = descriptor->enum_type()->full_name();

    /* Handle submessage descriptor */
    if (descriptor->message_type()) {
      const Descriptor *message = descriptor->message_type();
      variables["type"] = message->full_name();

      /* Extract and prepare message name */
      variables["message"] = StringReplace(
        message->full_name(), ".", "_", true);
      LowerString(&(variables["message"]));

      /* Generate field descriptor */
      printer->Print(variables,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `tag`,\n"
        "  .name  = \"`name`\",\n"
        "  .type  = `TYPE`,\n"
        "  .label = `LABEL`,\n"
        "  .refer = &`message`_descriptor }");

    /* Handle field descriptor with default value */
    } else if (descriptor->has_default_value()) {

      /* Extract and prepare field name */
      variables["field"] = StringReplace(
        descriptor->full_name(), ".", "_", true);
      LowerString(&(variables["field"]));

      /* Generate field descriptor */
      printer->Print(variables,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `tag`,\n"
        "  .name  = \"`name`\",\n"
        "  .type  = `TYPE`,\n"
        "  .label = `LABEL`,\n"
        "  .value = &`field`_default }");

    /* Generate field descriptor */
    } else {
      printer->Print(variables,
        "\n"
        "/* `label` `type` `name` = `tag` */\n"
        "{ .tag   = `tag`,\n"
        "  .name  = \"`name`\",\n"
        "  .type  = `TYPE`,\n"
        "  .label = `LABEL` }");
    }
  }

  /*!
   * Generate definitions.
   *
   * \param[in,out] printer Printer
   */
  void Field::
  GenerateDefinitions(io::Printer *printer) const {
    map<string, string> variables;

    /* Extract full name for signature */
    variables["signature"] = descriptor->full_name();

    /* Extract tag and name */
    variables["tag"]  = SimpleItoa(descriptor->number());
    variables["name"] = descriptor->lowercase_name();

    /* Extract and prepare message name */
    variables["message"] = StringReplace(
      descriptor->containing_type()->full_name(), ".", "_", true);
    LowerString(&(variables["message"]));

    /* Generate definition for checking a field's existence */
    if (!descriptor->is_required())
      printer->Print(variables,
        "/* `signature` : has */\n"
        "#define `message`_has_`name`(message) \\\n"
        "  (__`message`_descriptor_assert(message), \\\n"
        "    (pb_message_has((message), `tag`)))\n"
        "\n");

    /* Generate definitions for a nested message */
    if (descriptor->message_type()) {
      const Descriptor *message = descriptor->message_type();

      /* Generate constructor for nested message */
      printer->Print(variables,
        "/* `signature` : create */\n"
        "#define `message`_create_`name`(message) \\\n"
        "  (__`message`_descriptor_assert(message), \\\n"
        "    (pb_message_create_within((message), `tag`)))\n"
        "\n");

      /* Generate cursor definitions */
      if (descriptor->is_repeated())
        printer->Print(variables,
          "/* `signature` : cursor.create */\n"
          "#define `message`_create_`name`_cursor(message) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (pb_cursor_create((message), `tag`)))\n"
          "\n");

      /* Create a trace to generate definitions for nested messages */
      if (!descriptor->is_repeated()) {
        vector<const FieldDescriptor *> trace;
        trace.push_back(descriptor);

        /* Follow trace for corresponding message generator */
        scoped_ptr<Message> temp(new Message(message));
        temp->GenerateDefinitions(printer, trace);
      }

    /* Generate definitions for an enumeration field */
    } else if (descriptor->enum_type()) {
      const EnumDescriptor *enumeration = descriptor->enum_type();
      for (size_t v = 0; v < enumeration->value_count(); v++) {
        const EnumValueDescriptor *value = enumeration->value(v);

        /* Extract enumeration value tag and name */
        variables["enum-tag"]  = SimpleItoa(value->number());
        variables["enum-name"] = value->name();
        LowerString(&(variables["enum-name"]));

        /* Generate definitions for a specific enumeration value */
        printer->Print(variables,
          "/* `signature` : has(`enum-name`) */\n"
          "#define `message`_has_`name`_`enum-name`(message) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (pb_message_match((message), `tag`, (const int32_t []){ `enum-tag` })))\n"
          "\n"
          "/* `signature` : put(`enum-name`) */\n"
          "#define `message`_put_`name`_`enum-name`(message) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (pb_message_put((message), `tag`, (const int32_t []){ `enum-tag` })))\n"
          "\n");
      }

    /* Generate definitions for a primitive field */
    } else {
      if (!descriptor->is_repeated())
        printer->Print(variables,
          "/* `signature` : get */\n"
          "#define `message`_get_`name`(message, value) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (assert(value), pb_message_get((message), `tag`, (value))))\n"
          "\n");
      printer->Print(variables,
        "/* `signature` : put */\n"
        "#define `message`_put_`name`(message, value) \\\n"
        "  (__`message`_descriptor_assert(message), \\\n"
        "    (assert(value), pb_message_put((message), `tag`, (value))))\n"
        "\n");
    }

    /* Generate common definitions */
    if (descriptor->is_optional())
      printer->Print(variables,
        "/* `signature` : erase */\n"
        "#define `message`_erase_`name`(message) \\\n"
        "  (__`message`_descriptor_assert(message), \\\n"
        "    (pb_message_erase((message), `tag`)))\n"
        "\n");

    /* Generate definitions for a fixed-sized field */
    switch (descriptor->type()) {
      case FieldDescriptor::TYPE_FIXED32:
      case FieldDescriptor::TYPE_FIXED64:
      case FieldDescriptor::TYPE_SFIXED32:
      case FieldDescriptor::TYPE_SFIXED64:
      case FieldDescriptor::TYPE_FLOAT:
      case FieldDescriptor::TYPE_DOUBLE:
        printer->Print(variables,
          "/* `signature` : raw */\n"
          "#define `message`_raw_`name`(message) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (pb_message_raw((message), `tag`)))\n"
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
      io::Printer *printer, vector<const FieldDescriptor *> &trace) const {
    map<string, string> variables;

    /* Extract full name for signature */
    variables["signature"] = descriptor->full_name();

    /* Extract tags and names from trace */
    vector<string> tags, name;
    for (size_t f = 0; f < trace.size(); f++) {
      tags.push_back(SimpleItoa(trace.at(f)->number()));
      name.push_back(trace.at(f)->lowercase_name());
    }

    /* Push current tag and name to vector */
    tags.push_back(SimpleItoa(descriptor->number()));
    name.push_back(descriptor->lowercase_name());

    /* Assemble final tag and name */
    variables["tags"] = JoinStrings(tags, ", ");
    variables["size"] = SimpleItoa(tags.size());
    variables["name"] = JoinStrings(name, "_");

    /* Extract and prepare message name */
    variables["message"] = StringReplace(
      StripSuffixString(trace.front()->full_name(),
        "." + trace.front()->name()), ".", "_", true);
    LowerString(&(variables["message"]));

    /* Generate definition for checking a field's existence */
    if (!descriptor->is_required())
      printer->Print(variables,
        "/* `signature` : has */\n"
        "#define `message`_has_`name`(message) \\\n"
        "  (__`message`_descriptor_assert(message), \\\n"
        "    (pb_message_nested_has((message), \\\n"
        "      (const pb_tag_t []){ `tags` }, `size`)))\n"
        "\n");

    /* Generate definitions for a nested message */
    if (descriptor->message_type()) {
      const Descriptor *message = descriptor->message_type();

      /* Generate constructor for nested message */
      printer->Print(variables,
        "/* `signature` : create */\n"
        "#define `message`_create_`name`(message) \\\n"
        "  (__`message`_descriptor_assert(message), \\\n"
        "    (pb_message_create_nested((message), \\\n"
        "      (const pb_tag_t []){ `tags` }, `size`)))\n"
        "\n");

      /* Generate cursor definitions */
      if (descriptor->is_repeated())
        printer->Print(variables,
          "/* `signature` : cursor.create */\n"
          "#define `message`_create_`name`_cursor(message) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (pb_cursor_create_nested((message), \\\n"
          "      (const pb_tag_t []){ `tags` }, `size`)))\n"
          "\n");

      /* Push current descriptor to trace */
      if (!descriptor->is_repeated()) {
        trace.push_back(descriptor);

        /* Follow trace for corresponding message generator */
        scoped_ptr<Message> temp(new Message(message));
        temp->GenerateDefinitions(printer, trace);
        trace.pop_back();
      }

    /* Generate definitions for an enumeration field */
    } else if (descriptor->enum_type()) {
      const EnumDescriptor *enumeration = descriptor->enum_type();
      for (size_t v = 0; v < enumeration->value_count(); v++) {
        const EnumValueDescriptor *value = enumeration->value(v);

        /* Extract enumeration value tag and name */
        variables["enum-tag"]  = SimpleItoa(value->number());
        variables["enum-name"] = value->name();
        LowerString(&(variables["enum-name"]));

        /* Generate definitions for a specific enumeration value */
        printer->Print(variables,
          "/* `signature` : has(`enum-name`) */\n"
          "#define `message`_has_`name`_`enum-name`(message) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (pb_message_nested_match((message), \\\n"
          "      (const pb_tag_t []){ `tags` }, `size`, (const int32_t []){ `enum-tag` })))\n"
          "\n"
          "/* `signature` : put(`enum-name`) */\n"
          "#define `message`_put_`name`_`enum-name`(message) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (pb_message_nested_put((message), \\\n"
          "      (const pb_tag_t []){ `tags` }, `size`, (const int32_t []){ `enum-tag` })))\n"
          "\n");
      }

    /* Generate definitions for a primitive field */
    } else {
      if (!descriptor->is_repeated())
        printer->Print(variables,
          "/* `signature` : get */\n"
          "#define `message`_get_`name`(message, value) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (assert(value), pb_message_nested_get((message), \\\n"
          "      (const pb_tag_t []){ `tags` }, `size`, (value))))\n"
          "\n");
      printer->Print(variables,
        "/* `signature` : put */\n"
        "#define `message`_put_`name`(message, value) \\\n"
        "  (__`message`_descriptor_assert(message), \\\n"
        "    (assert(value), pb_message_nested_put((message), \\\n"
        "      (const pb_tag_t []){ `tags` }, `size`, (value))))\n"
        "\n");
    }

    /* Generate common definitions */
    if (descriptor->is_optional())
      printer->Print(variables,
        "/* `signature` : erase */\n"
        "#define `message`_erase_`name`(message) \\\n"
        "  (__`message`_descriptor_assert(message), \\\n"
        "    (pb_message_nested_erase((message), \\\n"
        "      (const pb_tag_t []){ `tags` }, `size`)))\n"
        "\n");

    /* Generate definitions for a fixed-sized field */
    switch (descriptor->type()) {
      case FieldDescriptor::TYPE_FIXED32:
      case FieldDescriptor::TYPE_FIXED64:
      case FieldDescriptor::TYPE_SFIXED32:
      case FieldDescriptor::TYPE_SFIXED64:
      case FieldDescriptor::TYPE_FLOAT:
      case FieldDescriptor::TYPE_DOUBLE:
        printer->Print(variables,
          "/* `signature` : raw */\n"
          "#define `message`_raw_`name`(message) \\\n"
          "  (__`message`_descriptor_assert(message), \\\n"
          "    (pb_message_nested_raw((message), \\\n"
          "      (const pb_tag_t []){ `tags` }, `size`)))\n"
          "\n");
        break;
      default:
        break;
    }
  }
}

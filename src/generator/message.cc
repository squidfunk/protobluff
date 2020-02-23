/*
 * Copyright (c) 2013-2020 Martin Donath <martin.donath@squidfunk.com>
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
#include <set>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

#include "generator/enum.hh"
#include "generator/field.hh"
#include "generator/message.hh"
#include "generator/oneof.hh"
#include "generator/strutil.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::set;
  using ::std::sort;
  using ::std::unique_ptr;
  using ::std::vector;

  using ::google::protobuf::Descriptor;
  using ::google::protobuf::FieldDescriptor;
  using ::google::protobuf::io::Printer;

  using ::google::protobuf::LowerString;
  using ::google::protobuf::SimpleItoa;
  using ::google::protobuf::StringReplace;

  /*!
   * Create a message generator.
   *
   * \param[in] descriptor Descriptor
   */
  Message::
  Message(const Descriptor *descriptor) :
      descriptor_(descriptor),
      fields_(new unique_ptr<Field>[descriptor_->field_count()]),
      enums_(new unique_ptr<Enum>[descriptor_->enum_type_count()]),
      oneofs_(new unique_ptr<Oneof>[descriptor_->oneof_decl_count()]),
      nested_(new unique_ptr<Message>[descriptor_->nested_type_count()]) {

    /* Sort field generators by tag */
    vector<Field *> sorted;
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      sorted.push_back(new Field(descriptor_->field(f)));
    sort(sorted.begin(), sorted.end(), FieldComparator);

    /* Initialize field generators */
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      fields_[f].reset(sorted[f]);

    /* Initialize enum generators */
    for (size_t e = 0; e < descriptor_->enum_type_count(); e++)
    enums_[e].reset(new Enum(descriptor_->enum_type(e)));

    /* Initialize oneof generators */
    for (size_t o = 0; o < descriptor_->oneof_decl_count(); o++)
    oneofs_[o].reset(new Oneof(descriptor_->oneof_decl(o)));

    /* Initialize nested message generators */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      nested_[n].reset(new Message(descriptor_->nested_type(n)));

    /* Build set of unique extended descriptors */
    set<const Descriptor *> unique;
    for (size_t e = 0; e < descriptor_->extension_count(); e++)
      unique.insert(descriptor_->extension(e)->containing_type());

    /* Initialize extension generators */
    for (set<const Descriptor *>::iterator it  = unique.begin();
                                           it != unique.end(); ++it) {
      Extension *extension = new Extension(*it, descriptor_);
      for (size_t e = 0; e < descriptor_->extension_count(); e++)
        if (descriptor_->extension(e)->containing_type() == *it)
          extension->AddField(descriptor_->extension(e));

      /* Add to list of extension generators */
      extensions_.push_back(extension);
    }

    /* Extract full name for signature */
    variables_["signature"] = descriptor_->full_name();

    /* Prepare message symbol */
    variables_["message"] = StringReplace(
      variables_["signature"], ".", "_", true);
    LowerString(&(variables_["message"]));

    /* Emit warning if message is deprecated */
    variables_["deprecated"] =
      descriptor_->options().deprecated()
        ? "PB_DEPRECATED\n" : "";
  }

  /*!
   * Generate tags.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateTags(Printer *printer) const {
    assert(printer);
    printer->Print(variables_,
      "/* `signature` : tags */\n");
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      fields_[f]->GenerateTag(printer);
    printer->Print("\n");

    /* Generate tags for nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      nested_[n]->GenerateTags(printer);
  }

  /*!
   * Generate default values.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateDefaults(Printer *printer) const {
    assert(printer);
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      if (fields_[f]->HasDefault())
        fields_[f]->GenerateDefault(printer);

    /* Generate default values for nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      nested_[n]->GenerateDefaults(printer);
  }

  /*!
   * Generate declaration.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateDeclaration(Printer *printer) const {
    assert(printer);

    /* Generate forward declaration */
    printer->Print(variables_,
      "/* `signature` : descriptor */\n"
      "extern pb_descriptor_t\n"
      "`message`_descriptor;\n"
      "\n");

    /* Generate forward declarations for nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      nested_[n]->GenerateDeclaration(printer);
  }

  /*!
   * Generate descriptor.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateDescriptor(Printer *printer) const {
    assert(printer);

    /* Generate descriptor header */
    if (descriptor_->field_count()) {
      printer->Print(variables_,
        "/* `signature` : descriptor */\n"
        "pb_descriptor_t\n"
        "`message`_descriptor = { {\n"
        "  (const pb_field_descriptor_t []){\n");

      /* Generate field descriptors */
      for (size_t i = 0; i < 2; i++)
        printer->Indent();
      for (size_t f = 0; f < descriptor_->field_count(); f++) {
        fields_[f]->GenerateDescriptor(printer);
        if (f < descriptor_->field_count() - 1)
          printer->Print(",");
        printer->Print("\n");
      }
      for (size_t i = 0; i < 2; i++)
        printer->Outdent();

      /* Generate descriptor footer */
      printer->Print(
        "\n"
        "  }, `fields` } };\n"
        "\n", "fields", SimpleItoa(descriptor_->field_count()));

    /* Print empty descriptor, if message contains no fields */
    } else {
      printer->Print(variables_,
        "/* `signature` : descriptor */\n"
        "pb_descriptor_t\n"
        "`message`_descriptor = {};\n"
        "\n");
    }

    /* Generate descriptors for nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      nested_[n]->GenerateDescriptor(printer);
  }

  /*!
   * Generate decoder.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateDecoder(Printer *printer) const {
    assert(printer);

    /* Generate constructor */
    printer->Print(variables_,
      "/* `signature` : create */\n"
      "`deprecated`"
      "PB_WARN_UNUSED_RESULT\n"
      "PB_INLINE pb_decoder_t\n"
      "`message`_decoder_create(\n"
      "    const pb_buffer_t *buffer) {\n"
      "  return pb_decoder_create(\n"
      "    &`message`_descriptor, buffer);\n"
      "}\n"
      "\n");

    /* Generate decoder */
    printer->Print(variables_,
      "/* `signature` : decode */\n"
      "`deprecated`"
      "PB_WARN_UNUSED_RESULT\n"
      "PB_INLINE pb_error_t\n"
      "`message`_decode(\n"
      "    pb_decoder_t *decoder, pb_decoder_handler_f handler, void *user) {\n"
      "  assert(pb_decoder_descriptor(decoder) == \n"
      "    &`message`_descriptor);\n"
      "  return pb_decoder_decode(decoder, handler, user);\n"
      "}\n"
      "\n");

    /* Generate destructor */
    printer->Print(variables_,
      "/* `signature` : destroy */\n"
      "`deprecated`"
      "PB_INLINE void\n"
      "`message`_decoder_destroy(\n"
      "    pb_decoder_t *decoder) {\n"
      "  assert(pb_decoder_descriptor(decoder) == \n"
      "    &`message`_descriptor);\n"
      "  return pb_decoder_destroy(decoder);\n"
      "}\n"
      "\n");

    /* Generate decoders for nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      nested_[n]->GenerateDecoder(printer);
  }

  /*!
   * Generate encoder.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateEncoder(Printer *printer) const {
    assert(printer);

    /* Generate constructor */
    printer->Print(variables_,
      "/* `signature` : create */\n"
      "`deprecated`"
      "PB_WARN_UNUSED_RESULT\n"
      "PB_INLINE pb_encoder_t\n"
      "`message`_encoder_create(void) {\n"
      "  return pb_encoder_create(\n"
      "    &`message`_descriptor);\n"
      "}\n"
      "\n");

    /* Generate constructor with allocator */
    printer->Print(variables_,
      "/* `signature` : create with allocator */\n"
      "`deprecated`"
      "PB_WARN_UNUSED_RESULT\n"
      "PB_INLINE pb_encoder_t\n"
      "`message`_encoder_create_with_allocator(\n"
      "    pb_allocator_t *allocator) {\n"
      "  return pb_encoder_create_with_allocator(\n"
      "    allocator, &`message`_descriptor);\n"
      "}\n"
      "\n");

    /* Generate destructor */
    printer->Print(variables_,
      "/* `signature` : destroy */\n"
      "`deprecated`"
      "PB_INLINE void\n"
      "`message`_encoder_destroy(\n"
      "    pb_encoder_t *encoder) {\n"
      "  assert(pb_encoder_descriptor(encoder) == \n"
      "    &`message`_descriptor);\n"
      "  return pb_encoder_destroy(encoder);\n"
      "}\n"
      "\n");

    /* Generate encoders for fields */
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      fields_[f]->GenerateEncoder(printer);

    /* Generate encoders for nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      nested_[n]->GenerateEncoder(printer);
  }

  /*!
   * Generate accessors.
   *
   * \param[in,out] printer Printer
   */
  void Message::
  GenerateAccessors(Printer *printer) const {
    assert(printer);

    /* Generate constructor */
    printer->Print(variables_,
      "/* `signature` : create */\n"
      "`deprecated`"
      "PB_WARN_UNUSED_RESULT\n"
      "PB_INLINE pb_message_t\n"
      "`message`_create(\n"
      "    pb_journal_t *journal) {\n"
      "  return pb_message_create(\n"
      "    &`message`_descriptor, journal);\n"
      "}\n"
      "\n");

    /* Generate destructor */
    printer->Print(variables_,
      "/* `signature` : destroy */\n"
      "`deprecated`"
      "PB_INLINE void\n"
      "`message`_destroy(\n"
      "    pb_message_t *message) {\n"
      "  assert(pb_message_descriptor(message) == \n"
      "    &`message`_descriptor);\n"
      "  return pb_message_destroy(message);\n"
      "}\n"
      "\n");

    /* Generate accessors for fields */
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      fields_[f]->GenerateAccessors(printer);

    /* Generate accessors for nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      nested_[n]->GenerateAccessors(printer);
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
  void Message::
  GenerateAccessors(
      Printer *printer, vector<const FieldDescriptor *> &trace) const {
    assert(printer);
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      fields_[f]->GenerateAccessors(printer, trace);
  }

  /*!
   * Check whether a message or its nested messages define default values.
   *
   * \return Test result
   */
  bool Message::
  HasDefaults() const {
    for (size_t f = 0; f < descriptor_->field_count(); f++)
      if (fields_[f]->HasDefault())
        return true;

    /* Check nested messages for default values */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      if (nested_[n]->HasDefaults())
        return true;

    /* No default values */
    return false;
  }

  /*!
   * Check whether a message or its nested messages define enums.
   *
   * \return Test result
   */
  bool Message::
  HasEnums() const {
    if (descriptor_->enum_type_count())
      return true;

    /* Check nested messages for enums */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      if (nested_[n]->HasEnums())
        return true;

    /* No enums */
    return false;
  }

  /*!
   * Check whether a message or its nested messages define oneofs.
   *
   * \return Test result
   */
  bool Message::
  HasOneofs() const {
    if (descriptor_->oneof_decl_count())
      return true;

    /* Check nested messages for oneofs */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      if (nested_[n]->HasOneofs())
        return true;

    /* No oneofs */
    return false;
  }

  /*!
   * Check whether a message or its nested messages define extensions.
   *
   * \return Test result
   */
  bool Message::
  HasExtensions() const {
    if (descriptor_->extension_count())
      return true;

    /* Check nested messages for extensions */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++)
      if (nested_[n]->HasExtensions())
        return true;

    /* No extensions */
    return false;
  }

  /*!
   * Retrieve enum generators.
   *
   * \return Enum generators
   */
  const vector<const Enum *> Message::
  GetEnums() const {
    vector<const Enum *> enums;
    for (size_t e = 0; e < descriptor_->enum_type_count(); e++)
      enums.push_back(enums_[e].get());

    /* Retrieve extensions from nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++) {
      vector<const Enum *> nested = nested_[n]->GetEnums();
      enums.insert(enums.end(), nested.begin(), nested.end());
    }
    return enums;
  }

  /*!
   * Retrieve oneof generators.
   *
   * \return Oneof generators
   */
  const vector<const Oneof *> Message::
  GetOneofs() const {
    vector<const Oneof *> oneofs;
    for (size_t o = 0; o < descriptor_->oneof_decl_count(); o++)
      oneofs.push_back(oneofs_[o].get());

    /* Retrieve oneofs from nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++) {
      vector<const Oneof *> nested = nested_[n]->GetOneofs();
      oneofs.insert(oneofs.end(), nested.begin(), nested.end());
    }
    return oneofs;
  }

  /*!
   * Retrieve extension generators.
   *
   * \return Extension generators
   */
  const vector<const Extension *> Message::
  GetExtensions() const {
    vector<const Extension *> extensions;
    extensions.insert(extensions.end(),
      extensions_.begin(), extensions_.end());

    /* Retrieve extensions from nested messages */
    for (size_t n = 0; n < descriptor_->nested_type_count(); n++) {
      vector<const Extension *> nested = nested_[n]->GetExtensions();
      extensions.insert(extensions.end(), nested.begin(), nested.end());
    }
    return extensions;
  }
}

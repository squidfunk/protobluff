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

#include <cassert>
#include <ctime>
#include <set>
#include <string>
#include <vector>

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/io/printer.h>
#include <google/protobuf/stubs/common.h>

#include <protobluff/core/common.h>

#include "generator/enum.hh"
#include "generator/extension.hh"
#include "generator/file.hh"
#include "generator/message.hh"
#include "generator/strutil.hh"

/* ----------------------------------------------------------------------------
 * Interface
 * ------------------------------------------------------------------------- */

namespace protobluff {

  using ::std::set;
  using ::std::string;
  using ::std::vector;

  using ::google::protobuf::FileDescriptor;
  using ::google::protobuf::FileOptions;
  using ::google::protobuf::io::Printer;
  using ::google::protobuf::scoped_ptr;

  using ::google::protobuf::SimpleItoa;
  using ::google::protobuf::StringReplace;
  using ::google::protobuf::StripSuffixString;
  using ::google::protobuf::UpperString;

  /*!
   * Create a file generator.
   *
   * \param[in] descriptor File descriptor
   */
  File::
  File(const FileDescriptor *descriptor) :
      descriptor_(descriptor),
      mode_(descriptor_->options().has_optimize_for()
        ? descriptor_->options().optimize_for()
        : FileOptions::SPEED),
      enums_(new scoped_ptr<Enum>[descriptor_->enum_type_count()]),
      messages_(new scoped_ptr<Message>[descriptor_->message_type_count()]) {

    /* Initialize enum generators */
    for (size_t e = 0; e < descriptor_->enum_type_count(); e++)
      enums_[e].reset(new Enum(descriptor_->enum_type(e)));

    /* Initialize message generators */
    for (size_t m = 0; m < descriptor_->message_type_count(); m++)
      messages_[m].reset(new Message(descriptor_->message_type(m)));

    /* Build set of unique extended descriptors */
    set<const Descriptor *> unique;
    for (size_t e = 0; e < descriptor_->extension_count(); e++)
      unique.insert(descriptor_->extension(e)->containing_type());

    /* Initialize extension generators */
    for (set<const Descriptor *>::iterator it  = unique.begin();
                                           it != unique.end(); ++it) {
      Extension *extension = new Extension(*it);
      for (size_t e = 0; e < descriptor_->extension_count(); e++)
        if (descriptor_->extension(e)->containing_type() == *it)
          extension->AddField(descriptor_->extension(e));

      /* Add to list of extension generators */
      extensions_.push_back(extension);
    }

    /* Prepare datetime string */
    char datetime[64];
    time_t   unixtime = time(0);
    strftime(datetime, sizeof(datetime),
      "%x %X", localtime(&unixtime));

    /* Extract metadata */
    variables_["datetime"] = datetime;
    variables_["filename"] = descriptor_->name();
    variables_["package"]  = descriptor_->package().size() > 0
      ? descriptor_->package() : "-";

    /* Extract and prepare guard name */
    variables_["guard"] = StringReplace(
      StripSuffixString(descriptor_->name(), ".proto"),
    ".", "_", true);
    UpperString(&(variables_["guard"]));

    /* Prepare current runtime version */
    variables_["version"] = SimpleItoa(PB_VERSION);

    /* Prepare header include */
    variables_["include"] = StripSuffixString(descriptor_->name(), ".proto");
  }

  /*!
   * Generate header file.
   *
   * \param[in,out] printer Printer
   */
  void File::
  GenerateHeader(Printer *printer) const {
    assert(printer);

    /* Generate disclaimer */
    PrintDisclaimer(printer);

    /* Generate guard header and library includes */
    printer->Print(variables_,
      "#ifndef `guard`_PB_H\n"
      "#define `guard`_PB_H\n"
      "\n"
      "#include <protobluff.h>\n"
      "\n"
      "#if PB_VERSION != `version`\n"
      "  #error version mismatch - please regenerate this file using protoc\n"
      "#endif\n"
      "\n");

    /* Generate dependent includes */
    if (descriptor_->dependency_count()) {
      bool includes = false;
      for (size_t d = 0; d < descriptor_->dependency_count(); d++) {
        string dependency = descriptor_->dependency(d)->name();
        printer->Print("#include \"`dependency`.pb.h\"\n", "dependency",
          StripSuffixString(dependency, ".proto"));
        includes = true;
      }
      if (includes)
        printer->Print("\n");
    }

    /* Generate values and descriptors for enums */
    if (HasEnums()) {
      PrintBanner(printer, "Enum values");

      /* Generate values for enums */
      for (size_t e = 0; e < descriptor_->enum_type_count(); e++)
        enums_[e]->GenerateValues(printer);

      /* Generate values for nested enums */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
        const vector<const Enum *> enums = messages_[m]->GetEnums();
        for (size_t e = 0; e < enums.size(); e++)
          enums[e]->GenerateValues(printer);
      }
    }

    /* Generate descriptors for enums */
    if (HasEnums()) {
      PrintBanner(printer, "Enum descriptors");

      /* Generate descriptors for enums */
      for (size_t e = 0; e < descriptor_->enum_type_count(); e++)
        enums_[e]->GenerateDeclaration(printer);

      /* Generate descriptors for nested enums */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
        const vector<const Enum *> enums = messages_[m]->GetEnums();
        for (size_t e = 0; e < enums.size(); e++)
          enums[e]->GenerateDeclaration(printer);
      }
    }

    /* Generate descriptors for oneofs */
    if (HasOneofs()) {
      PrintBanner(printer, "Oneof descriptors");

      /* Generate descriptors for oneofs */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
        const vector<const Oneof *> oneofs = messages_[m]->GetOneofs();
        for (size_t o = 0; o < oneofs.size(); o++)
          oneofs[o]->GenerateDeclaration(printer);
      }
    }

    /* Generate tags for messages */
    if (descriptor_->message_type_count()) {
      PrintBanner(printer, "Tags");

      /* Generate tags for messages and nested messages */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++)
        messages_[m]->GenerateTags(printer);

      /* Generate descriptors for messages */
      PrintBanner(printer, "Descriptors");

      /* Generate descriptors for messages and nested messages */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++)
        messages_[m]->GenerateDeclaration(printer);

      /* Generate decoder banner */
      PrintBanner(printer, "Decoders");

      /* Generate decoders for messages and nested messages */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++)
        messages_[m]->GenerateDecoder(printer);

      /* Generate encoder banner */
      PrintBanner(printer, "Encoders");

      /* Generate encoders for messages and nested messages */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++)
        messages_[m]->GenerateEncoder(printer);
    }

    /* Don't generate accessor code for lite runtime */
    if (mode_ != FileOptions::LITE_RUNTIME) {

      /* Generate accessors for messages */
      if (descriptor_->message_type_count()) {
        PrintBanner(printer, "Accessors");

        /* Generate accessors for messages and nested messages */
        for (size_t m = 0; m < descriptor_->message_type_count(); m++)
          messages_[m]->GenerateAccessors(printer);
      }

      /* Generate accessors for oneofs */
      if (HasOneofs()) {
        PrintBanner(printer, "Oneof accessors");

        /* Generate accessors for oneofs */
        for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
          const vector<const Oneof *> oneofs = messages_[m]->GetOneofs();
          for (size_t o = 0; o < oneofs.size(); o++)
            oneofs[o]->GenerateAccessors(printer);
        }
      }

      /* Generate accessors for extensions */
      if (HasExtensions()) {
        PrintBanner(printer, "Extension accessors");

        /* Generate accessors for extensions */
        for (size_t e = 0; e < extensions_.size(); e++)
          extensions_[e]->GenerateAccessors(printer);

        /* Generate accessors for nested extensions */
        for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
          const vector<const Extension *> extensions =
            messages_[m]->GetExtensions();
          for (size_t e = 0; e < extensions.size(); e++)
            extensions[e]->GenerateAccessors(printer);
        }
      }
    }

    /* Generate guard footer */
    printer->Print(variables_,
      "#endif /* `guard`_PB_H */");
  }

  /*!
   * Generate source file.
   *
   * \param[in,out] printer Printer
   */
  void File::
  GenerateSource(Printer *printer) const {
    assert(printer);

    /* Generate disclaimer */
    PrintDisclaimer(printer);

    /* Generate library includes */
    printer->Print(variables_,
      "#include <protobluff/descriptor.h>\n"
      "\n");

    /* Generate header include */
    printer->Print(variables_,
      "#include \"`include`.pb.h\"\n"
      "\n");

    /* Generate default values */
    if (HasDefaults()) {
      PrintBanner(printer, "Defaults");

      /* Generate default values for messages and nested messages */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++)
        messages_[m]->GenerateDefaults(printer);

      /* Generate default values for extensions */
      for (size_t e = 0; e < extensions_.size(); e++)
        extensions_[e]->GenerateDefaults(printer);

      /* Generate default values for nested extensions */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
        const vector<const Extension *> extensions =
          messages_[m]->GetExtensions();
        for (size_t e = 0; e < extensions.size(); e++)
          extensions[e]->GenerateDefaults(printer);
      }
    }

    /* Generate descriptors for enums */
    if (HasEnums()) {
      PrintBanner(printer, "Enum descriptors");

      /* Generate descriptors for enums */
      for (size_t e = 0; e < descriptor_->enum_type_count(); e++)
        enums_[e]->GenerateDescriptor(printer);

      /* Generate descriptors for nested enums */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
        const vector<const Enum *> enums = messages_[m]->GetEnums();
        for (size_t e = 0; e < enums.size(); e++)
          enums[e]->GenerateDescriptor(printer);
      }
    }

    /* Generate descriptors for oneofs */
    if (HasOneofs()) {
      PrintBanner(printer, "Oneof descriptors");

      /* Generate descriptors for oneofs */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
        const vector<const Oneof *> oneofs = messages_[m]->GetOneofs();
        for (size_t o = 0; o < oneofs.size(); o++)
          oneofs[o]->GenerateDescriptor(printer);
      }
    }

    /* Generate descriptors for messages */
    if (descriptor_->message_type_count()) {
      PrintBanner(printer, "Descriptors");

      /* Generate descriptors for messages and nested messages */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++)
        messages_[m]->GenerateDescriptor(printer);
    }

    /* Generate descriptors and initializers for extensions */
    if (HasExtensions()) {
      PrintBanner(printer, "Extension descriptors");

      /* Generate descriptors for extensions */
      for (size_t e = 0; e < extensions_.size(); e++)
        extensions_[e]->GenerateDescriptor(printer);

      /* Generate descriptors for nested extensions */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
        const vector<const Extension *> extensions =
          messages_[m]->GetExtensions();
        for (size_t e = 0; e < extensions.size(); e++)
          extensions[e]->GenerateDescriptor(printer);
      }

      /* Generate initializer banner */
      PrintBanner(printer, "Extension initializers");

      /* Generate initializer for extensions */
      for (size_t e = 0; e < extensions_.size(); e++)
        extensions_[e]->GenerateInitializer(printer);

      /* Generate initializer for nested extensions */
      for (size_t m = 0; m < descriptor_->message_type_count(); m++) {
        const vector<const Extension *> extensions =
          messages_[m]->GetExtensions();
        for (size_t e = 0; e < extensions.size(); e++)
          extensions[e]->GenerateInitializer(printer);
      }
    }
  }

  /*!
   * Check whether a file's messages define default values.
   *
   * \return Test result
   */
  bool File::
  HasDefaults() const {
    for (size_t m = 0; m < descriptor_->message_type_count(); m++)
      if (messages_[m]->HasDefaults())
        return true;

    /* Check extensions for default values */
    for (size_t e = 0; e < extensions_.size(); e++)
      if (extensions_[e]->HasDefaults())
        return true;

    /* No default values */
    return false;
  }

  /*!
   * Check whether a file or its messages define enums.
   *
   * \return Test result
   */
  bool File::
  HasEnums() const {
    if (descriptor_->enum_type_count())
      return true;

    /* Check messages for enums */
    for (size_t m = 0; m < descriptor_->message_type_count(); m++)
      if (messages_[m]->HasEnums())
        return true;

    /* No enums */
    return false;
  }

  /*!
   * Check whether a file's messages define oneofs.
   *
   * \return Test result
   */
  bool File::
  HasOneofs() const {
    for (size_t m = 0; m < descriptor_->message_type_count(); m++)
      if (messages_[m]->HasOneofs())
        return true;

    /* No oneofs */
    return false;
  }

  /*!
   * Check whether a file or its messages define extensions.
   *
   * \return Test result
   */
  bool File::
  HasExtensions() const {
    if (descriptor_->extension_count())
      return true;

    /* Check messages for extensions */
    for (size_t m = 0; m < descriptor_->message_type_count(); m++)
      if (messages_[m]->HasExtensions())
        return true;

    /* No extensions */
    return false;
  }

  /*!
   * Print metadata and disclaimer.
   *
   * \param[in,out] printer Printer
   */
  void File::
  PrintDisclaimer(Printer *printer) const {
    assert(printer);
    printer->Print(variables_,
      "/*\n"
      " * Generated by the protobluff compiler - do not edit.\n"
      " * ----------------------------------------------------------------------------\n"
      " * Date:     `datetime`\n"
      " * Filename: `filename`\n"
      " * Package:  `package`\n"
      " * ----------------------------------------------------------------------------\n"
      " * THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
      " * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
      " * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE\n"
      " * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
      " * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING\n"
      " * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS\n"
      " * IN THE SOFTWARE.\n"
      " */\n"
      "\n");
  }

  /*!
   * Print a banner.
   *
   * \param[in,out] printer Printer
   * \param[in]     title   Title
   */
  void File::
  PrintBanner(Printer *printer, const char title[]) const {
    assert(printer && title);
    string top (76, '-'), bottom (73, '-');
    printer->Print(
      "/* `top`\n"
      " * `title`\n"
      " * `bottom` */\n"
      "\n", "top", top, "title", title, "bottom", bottom);
  }
}
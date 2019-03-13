// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/toolchain.h"

#include <stddef.h>
#include <string.h>
#include <utility>

#include "base/logging.h"
#include "tools/gn/filesystem_utils.h"
#include "tools/gn/target.h"
#include "tools/gn/value.h"

const char* Toolchain::kToolCc = "cc";
const char* Toolchain::kToolCxx = "cxx";
const char* Toolchain::kToolObjC = "objc";
const char* Toolchain::kToolObjCxx = "objcxx";
const char* Toolchain::kToolRc = "rc";
const char* Toolchain::kToolAsm = "asm";
const char* Toolchain::kToolAlink = "alink";
const char* Toolchain::kToolSolink = "solink";
const char* Toolchain::kToolSolinkModule = "solink_module";
const char* Toolchain::kToolLink = "link";
const char* Toolchain::kToolStamp = "stamp";
const char* Toolchain::kToolCopy = "copy";
const char* Toolchain::kToolCopyBundleData = "copy_bundle_data";
const char* Toolchain::kToolCompileXCAssets = "compile_xcassets";
const char* Toolchain::kToolAction = "action";

Toolchain::Toolchain(const Settings* settings,
                     const Label& label,
                     const std::set<SourceFile>& build_dependency_files)
    : Item(settings, label, build_dependency_files),
      define_switch_("-D"),
      include_switch_("-I"),
      sys_include_switch_("-isystem ") {
}

Toolchain::~Toolchain() = default;

Toolchain* Toolchain::AsToolchain() {
  return this;
}

const Toolchain* Toolchain::AsToolchain() const {
  return this;
}

SourceFileType Toolchain::GetSourceFileType(const SourceFile& file) const {
  base::StringPiece extension = FindExtension(&file.value());
  for (size_t i= TYPE_NONE; i < TYPE_NUMTYPES; i++) {
    const Tool* tool = tools_[i].get();
    if (tool != NULL) {
      for (const auto & ext : tool->source_extensions()) {
        if (extension == ext) {
          ToolType toolType = static_cast<ToolType>(i);
          SourceFileType fileType = GetSourceTypeForToolType(toolType);
          if (fileType != SOURCE_UNKNOWN)
            return fileType;
        }
      }
    }
  }
  for (const auto & ext : object_extensions_) {
    if (extension == ext)
      return SOURCE_O;
  }

  if (!ToolHasSourceExtensions(TYPE_CXX) &&
      (extension == "cc" || extension == "cpp" || extension == "cxx" ||
       extension == "c++" || extension == "C"))
    return SOURCE_CPP;
  if (extension == "h" || extension == "hpp" || extension == "hxx" ||
      extension == "hh" || extension == "h++" || extension == "H")
    return SOURCE_H;
  if (!ToolHasSourceExtensions(TYPE_CC) &&
      extension == "c")
    return SOURCE_C;
  if (!ToolHasSourceExtensions(TYPE_OBJC) &&
      extension == "m")
    return SOURCE_M;
  if (!ToolHasSourceExtensions(TYPE_OBJCXX) &&
      (extension == "mm" || extension == "M"))
    return SOURCE_MM;
  if (!ToolHasSourceExtensions(TYPE_RC) &&
      extension == "rc")
    return SOURCE_RC;
  if (!ToolHasSourceExtensions(TYPE_ASM) &&
      (extension == "S" || extension == "s" || extension == "sx"))
    return SOURCE_S;
  if (!ToolHasSourceExtensions(TYPE_ASM) &&
      extension == "asm")
    return SOURCE_ASM;
  if (object_extensions_.empty() &&
      (extension == "o" || extension == "obj"))
    return SOURCE_O;
  if (!(ToolHasSourceExtensions(TYPE_LINK) ||
        ToolHasSourceExtensions(TYPE_SOLINK) ||
        ToolHasSourceExtensions(TYPE_SOLINK_MODULE)) &&
      extension == "ld")
    return SOURCE_LINK;
  if (extension == "def")
    return SOURCE_DEF;

  return SOURCE_UNKNOWN;
}

// static
Toolchain::ToolType Toolchain::ToolNameToType(const base::StringPiece& str) {
  if (str == kToolCc)
    return TYPE_CC;
  if (str == kToolCxx)
    return TYPE_CXX;
  if (str == kToolObjC)
    return TYPE_OBJC;
  if (str == kToolObjCxx)
    return TYPE_OBJCXX;
  if (str == kToolRc)
    return TYPE_RC;
  if (str == kToolAsm)
    return TYPE_ASM;
  if (str == kToolAlink)
    return TYPE_ALINK;
  if (str == kToolSolink)
    return TYPE_SOLINK;
  if (str == kToolSolinkModule)
    return TYPE_SOLINK_MODULE;
  if (str == kToolLink)
    return TYPE_LINK;
  if (str == kToolStamp)
    return TYPE_STAMP;
  if (str == kToolCopy)
    return TYPE_COPY;
  if (str == kToolCopyBundleData)
    return TYPE_COPY_BUNDLE_DATA;
  if (str == kToolCompileXCAssets)
    return TYPE_COMPILE_XCASSETS;
  if (str == kToolAction)
    return TYPE_ACTION;
  return TYPE_NONE;
}

// static
std::string Toolchain::ToolTypeToName(ToolType type) {
  switch (type) {
    case TYPE_CC:
      return kToolCc;
    case TYPE_CXX:
      return kToolCxx;
    case TYPE_OBJC:
      return kToolObjC;
    case TYPE_OBJCXX:
      return kToolObjCxx;
    case TYPE_RC:
      return kToolRc;
    case TYPE_ASM:
      return kToolAsm;
    case TYPE_ALINK:
      return kToolAlink;
    case TYPE_SOLINK:
      return kToolSolink;
    case TYPE_SOLINK_MODULE:
      return kToolSolinkModule;
    case TYPE_LINK:
      return kToolLink;
    case TYPE_STAMP:
      return kToolStamp;
    case TYPE_COPY:
      return kToolCopy;
    case TYPE_COPY_BUNDLE_DATA:
      return kToolCopyBundleData;
    case TYPE_COMPILE_XCASSETS:
      return kToolCompileXCAssets;
    case TYPE_ACTION:
      return kToolAction;
    default:
      NOTREACHED();
      return std::string();
  }
}

Tool* Toolchain::GetTool(ToolType type) {
  DCHECK(type != TYPE_NONE);
  return tools_[static_cast<size_t>(type)].get();
}

const Tool* Toolchain::GetTool(ToolType type) const {
  DCHECK(type != TYPE_NONE);
  return tools_[static_cast<size_t>(type)].get();
}

void Toolchain::SetTool(ToolType type, std::unique_ptr<Tool> t) {
  DCHECK(type != TYPE_NONE);
  DCHECK(!tools_[type].get());
  t->SetComplete();
  tools_[type] = std::move(t);
}

void Toolchain::ToolchainSetupComplete() {
  // Collect required bits from all tools.
  for (const auto& tool : tools_) {
    if (tool)
      substitution_bits_.MergeFrom(tool->substitution_bits());
  }

  setup_complete_ = true;
}

// static
Toolchain::ToolType Toolchain::GetToolTypeForSourceType(SourceFileType type) {
  switch (type) {
    case SOURCE_C:
      return TYPE_CC;
    case SOURCE_CPP:
      return TYPE_CXX;
    case SOURCE_M:
      return TYPE_OBJC;
    case SOURCE_MM:
      return TYPE_OBJCXX;
    case SOURCE_ASM:
    case SOURCE_S:
      return TYPE_ASM;
    case SOURCE_RC:
      return TYPE_RC;
    case SOURCE_UNKNOWN:
    case SOURCE_H:
    case SOURCE_O:
    case SOURCE_DEF:
    case SOURCE_LINK:
      return TYPE_NONE;
    default:
      NOTREACHED();
      return TYPE_NONE;
  }
}

SourceFileType Toolchain::GetSourceTypeForToolType(ToolType type) {
  switch (type) {
    case TYPE_CC:
      return SOURCE_C;
    case TYPE_CXX:
      return SOURCE_CPP;
    case TYPE_OBJC:
      return SOURCE_M;
    case TYPE_OBJCXX:
      return SOURCE_MM;
    case TYPE_ASM:
      return SOURCE_ASM;
    case TYPE_RC:
      return SOURCE_RC;
    case TYPE_LINK:
    case TYPE_SOLINK:
    case TYPE_SOLINK_MODULE:
      return SOURCE_LINK;
    default:
      return SOURCE_UNKNOWN;
  }
}

const Tool* Toolchain::GetToolForSourceType(SourceFileType type) {
  return tools_[GetToolTypeForSourceType(type)].get();
}

bool Toolchain::ToolHasSourceExtensions(ToolType type) const {
  const Tool* t = tools_[type].get();
  return t != NULL && !t->source_extensions().empty();
}

// static
Toolchain::ToolType Toolchain::GetToolTypeForTargetFinalOutput(
    const Target* target) {
  // The contents of this list might be suprising (i.e. stamp tool for copy
  // rules). See the header for why.
  // TODO(crbug.com/gn/39): Don't emit stamp files for single-output targets.
  switch (target->output_type()) {
    case Target::GROUP:
      return TYPE_STAMP;
    case Target::EXECUTABLE:
      return Toolchain::TYPE_LINK;
    case Target::SHARED_LIBRARY:
      return Toolchain::TYPE_SOLINK;
    case Target::LOADABLE_MODULE:
      return Toolchain::TYPE_SOLINK_MODULE;
    case Target::STATIC_LIBRARY:
      return Toolchain::TYPE_ALINK;
    case Target::SOURCE_SET:
      return TYPE_STAMP;
    case Target::ACTION:
    case Target::ACTION_FOREACH:
    case Target::BUNDLE_DATA:
    case Target::CREATE_BUNDLE:
    case Target::COPY_FILES:
    case Target::GENERATED_FILE:
      return TYPE_STAMP;
    default:
      NOTREACHED();
      return Toolchain::TYPE_NONE;
  }
}

const Tool* Toolchain::GetToolForTargetFinalOutput(const Target* target) const {
  return tools_[GetToolTypeForTargetFinalOutput(target)].get();
}

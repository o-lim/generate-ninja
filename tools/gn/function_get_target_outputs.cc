// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/build_settings.h"
#include "tools/gn/functions.h"
#include "tools/gn/parse_tree.h"
#include "tools/gn/settings.h"
#include "tools/gn/substitution_writer.h"
#include "tools/gn/target.h"
#include "tools/gn/value.h"

namespace functions {

const char kGetTargetOutputs[] = "get_target_outputs";
const char kGetTargetOutputs_HelpShort[] =
    "get_target_outputs: [file list] Get the list of outputs from a target.";
const char kGetTargetOutputs_Help[] =
    R"(get_target_outputs: [file list] Get the list of outputs from a target.

  get_target_outputs(target_label)

  Returns a list of output files for the named target. The named target must
  have been previously defined in the current file before this function is
  called (it can't reference targets in other files because there isn't a
  defined execution order, and it obviously can't reference targets that are
  defined after the function call).

  Copy and action targets are supported, as well as binary targets, which will
  depend on the toolchain definition. Groups have no useful output file and
  are not supported.

Return value

  The names in the resulting list will be absolute file paths (normally like
  "//out/Debug/bar.exe", depending on the build directory).

  action targets: this will just return the files specified in the "outputs"
  variable of the target.

  action_foreach targets: this will return the result of applying the output
  template to the sources (see "gn help source_expansion"). This will be the
  same result (though with guaranteed absolute file paths), as
  process_file_template will return for those inputs (see "gn help
  process_file_template").

  executables and libraries: this will return a list of the resulting binary
  file(s). The "main output" (the actual binary or library) will always be the
  0th element in the result. Depending on the platform and output type, there
  may be other output files as well (like import libraries) which will follow.

  source sets: this will return a list of the resulting object file(s) for each
  source file after it has been compiled. Depending on the platform and file
  type, there may be more than one output file for each source file. However,
  there can only be one object file for each source file. The "main output" for
  each source file (the actual object file) will always be the first output for
  that source file.

Example

  # Say this action generates a bunch of C source files.
  action_foreach("my_action") {
    sources = [ ... ]
    outputs = [ ... ]
  }

  # Compile the resulting source files into a source set.
  source_set("my_lib") {
    sources = get_target_outputs(":my_action")
  }
)";

Value RunGetTargetOutputs(Scope* scope,
                          const FunctionCallNode* function,
                          const std::vector<Value>& args,
                          Err* err) {
  if (args.size() != 1) {
    *err = Err(function, "Expected one argument.");
    return Value();
  }

  // Resolve the requested label.
  Label label = Label::Resolve(scope->GetSourceDir(),
                               ToolchainLabelForScope(scope), args[0], err);
  if (label.is_null())
    return Value();

  // Find the referenced target. The targets previously encountered in this
  // scope will have been stashed in the item collector (they'll be dispatched
  // when this file is done running) so we can look through them.
  const Target* target = nullptr;
  Scope::ItemVector* collector = scope->GetItemCollector();
  if (!collector) {
    *err = Err(function, "No targets defined in this context.");
    return Value();
  }
  for (const auto& item : *collector) {
    if (item->label() != label)
      continue;

    const Target* as_target = item->AsTarget();
    if (!as_target) {
      *err = Err(function, "Label does not refer to a target.",
                 label.GetUserVisibleName(false) + "\nrefers to a " +
                     item->GetItemTypeName());
      return Value();
    }
    target = as_target;
    break;
  }

  if (!target) {
    *err = Err(function, "Target not found in this context.",
               label.GetUserVisibleName(false) +
                   "\nwas not found. get_target_outputs() can only be used for "
                   "targets\n"
                   "previously defined in the current file.");
    return Value();
  }

  // Compute the output list.
  std::vector<SourceFile> files;
  std::vector<OutputFile> source_outputs;
  if (target->output_type() == Target::ACTION ||
      target->output_type() == Target::COPY_FILES ||
      target->output_type() == Target::ACTION_FOREACH) {
    target->action_values().GetOutputsAsSourceFiles(target, &files);
  } else if (target->output_type() == Target::SOURCE_SET) {
    // Compute object files for all sources. Only take the first output from
    // the tool if there is more than one.
    std::vector<OutputFile> outputs;
    for (const SourceFile& source : target->sources()) {
      Toolchain::ToolType tool_type = Toolchain::TYPE_NONE;
      if (target->GetOutputFilesForSource(source, &tool_type, &outputs))
        source_outputs.push_back(outputs[0]);
    }
  } else if (target->output_type() == Target::EXECUTABLE ||
             target->output_type() == Target::LOADABLE_MODULE ||
             target->output_type() == Target::SHARED_LIBRARY ||
             target->output_type() == Target::STATIC_LIBRARY) {
    const Toolchain* toolchain = target->toolchain();
    const Tool* tool = toolchain->GetToolForTargetFinalOutput(target);
    SubstitutionWriter::ApplyListToLinkerAsOutputFile(
        target, tool, tool->outputs(), &source_outputs);
  } else {
    // Other types of targets are not supported.
    *err = Err(args[0], "Target not supported.",
               "Only action, action_foreach, copy, executable, loadable_module,"
               "\nshared_library, source_set, and static_library targets are\n"
               "supported by get_target_outputs.");
    return Value();
  }

  // Convert to source files.
  for (const OutputFile& output : source_outputs) {
    files.push_back(output.AsSourceFile(target->settings()->build_settings()));
  }

  // Convert to Values.
  Value ret(function, Value::LIST);
  ret.list_value().reserve(files.size());
  for (const auto& file : files)
    ret.list_value().push_back(Value(function, file.value()));

  return ret;
}

}  // namespace functions

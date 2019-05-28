// Copyright (c) 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/json_project_writer.h"

#include <iostream>
#include <memory>

#include "base/command_line.h"
#include "base/json/json_writer.h"
#include "base/strings/string_number_conversions.h"
#include "tools/gn/builder.h"
#include "tools/gn/commands.h"
#include "tools/gn/deps_iterator.h"
#include "tools/gn/desc_builder.h"
#include "tools/gn/exec_process.h"
#include "tools/gn/filesystem_utils.h"
#include "tools/gn/settings.h"

// Structure of JSON output file
// {
//   "build_settings" = {
//     "root_path" : "absolute path of project root",
//     "build_dir" : "build directory (project relative)",
//     "default_toolchain" : "name of default toolchain"
//   }
//   "targets" = {
//      "target x name" : { target x properties },
//      "target y name" : { target y properties },
//      ...
//    }
// }
// See desc_builder.cc for overview of target properties

namespace {

void AddTargetDependencies(const Target* target,
                           std::set<const Target*>* deps) {
  for (const auto& pair : target->GetDeps(Target::DEPS_LINKED)) {
    if (deps->find(pair.ptr) == deps->end()) {
      deps->insert(pair.ptr);
      AddTargetDependencies(pair.ptr, deps);
    }
  }
}

// Filters targets according to filter string; Will also recursively
// add dependent targets.
bool FilterTargets(const BuildSettings* build_settings,
                   std::vector<const Target*>& all_targets,
                   std::vector<const Target*>* targets,
                   const std::string& dir_filter_string,
                   Err* err) {
  if (dir_filter_string.empty()) {
    *targets = all_targets;
  } else {
    targets->reserve(all_targets.size());
    std::vector<LabelPattern> filters;
    if (!commands::FilterPatternsFromString(build_settings, dir_filter_string,
                                            &filters, err)) {
      return false;
    }
    commands::FilterTargetsByPatterns(all_targets, filters, targets);

    std::set<const Target*> target_set(targets->begin(), targets->end());
    for (const auto* target : *targets)
      AddTargetDependencies(target, &target_set);

    targets->clear();
    targets->insert(targets->end(), target_set.begin(), target_set.end());
  }

  // Sort the list of targets per-label to get a consistent ordering of them
  // in the generated project (and thus stability of the file generated).
  std::sort(targets->begin(), targets->end(),
            [](const Target* a, const Target* b) {
              return a->label().name() < b->label().name();
            });

  return true;
}

std::string RenderJSON(const BuildSettings* build_settings,
                       const Builder& builder,
                       std::vector<const Target*>& all_targets) {
  Label default_toolchain_label;

  auto targets = std::make_unique<base::DictionaryValue>();
  for (const auto* target : all_targets) {
    if (default_toolchain_label.is_null())
      default_toolchain_label = target->settings()->default_toolchain_label();
    auto description =
        DescBuilder::DescriptionForTarget(target, "", false, false, false);
    // Outputs need to be asked for separately.
    auto outputs = DescBuilder::DescriptionForTarget(target, "source_outputs",
                                                     false, false, false);
    base::DictionaryValue* outputs_value = nullptr;
    if (outputs->GetDictionary("source_outputs", &outputs_value) &&
        !outputs_value->empty()) {
      description->MergeDictionary(outputs.get());
    }
    targets->SetWithoutPathExpansion(
        target->label().GetUserVisibleName(default_toolchain_label),
        std::move(description));
  }

  auto args = std::make_unique<base::DictionaryValue>();
  for (const auto& arg : build_settings->build_args().GetAllArguments()) {
    if (arg.second.has_override) {
      args->SetKey(arg.first,
                   base::Value(arg.second.override_value.ToString(true)));
    } else {
      args->SetKey(arg.first,
                   base::Value(arg.second.default_value.ToString(true)));
    }
  }

  auto settings = std::make_unique<base::DictionaryValue>();
  settings->SetKey("root_path", base::Value(build_settings->root_path_utf8()));
  settings->SetKey("build_dir",
                   base::Value(build_settings->build_dir().value()));
  settings->SetKey(
      "default_toolchain",
      base::Value(default_toolchain_label.GetUserVisibleName(false)));
  settings->SetWithoutPathExpansion("build_args", std::move(args));

  auto output = std::make_unique<base::DictionaryValue>();
  output->SetWithoutPathExpansion("targets", std::move(targets));
  output->SetWithoutPathExpansion("build_settings", std::move(settings));

  std::string s;
  base::JSONWriter::WriteWithOptions(
      *output.get(), base::JSONWriter::OPTIONS_PRETTY_PRINT, &s);
  return s;
}

bool InvokeScript(const BuildSettings* build_settings,
                  const base::FilePath& interpreter_path,
                  const base::FilePath& script_path,
                  const std::string& script_extra_args,
                  const base::FilePath& output_path,
                  bool quiet,
                  Err* err) {
  base::CommandLine cmdline(interpreter_path);
  cmdline.AppendArg("--");
  cmdline.AppendArgPath(script_path);
  cmdline.AppendArgPath(output_path);
  if (!script_extra_args.empty()) {
    cmdline.AppendArg(script_extra_args);
  }
  base::FilePath startup_dir =
      build_settings->GetFullPath(build_settings->build_dir());

  std::string output;
  std::string stderr_output;

  int exit_code = 0;
  if (!internal::ExecProcess(cmdline, startup_dir, &output, &stderr_output,
                             &exit_code)) {
    *err =
        Err(Location(), "Could not execute interpreter.",
            "I was trying to execute \"" + FilePathToUTF8(interpreter_path) + "\".");
    return false;
  }

  if (!quiet) {
    std::cout << output;
    std::cerr << stderr_output;
  }

  if (exit_code != 0) {
    *err = Err(Location(), "Python has quit with exit code " +
                               base::IntToString(exit_code) + ".");
    return false;
  }

  return true;
}

}  // namespace

bool JSONProjectWriter::RunAndWriteFiles(
    const BuildSettings* build_settings,
    const Builder& builder,
    const std::string& file_name,
    const base::FilePath& exec_script_interpreter,
    const std::string& exec_script,
    const std::string& exec_script_extra_args,
    const std::string& dir_filter_string,
    bool quiet,
    Err* err) {
  SourceFile output_file = build_settings->build_dir().ResolveRelativeFile(
      Value(nullptr, file_name), err);
  if (output_file.is_null()) {
    return false;
  }

  base::FilePath output_path = build_settings->GetFullPath(output_file);

  std::vector<const Target*> all_targets = builder.GetAllResolvedTargets();
  std::vector<const Target*> targets;
  if (!FilterTargets(build_settings, all_targets, &targets, dir_filter_string,
                     err)) {
    return false;
  }

  std::string json = RenderJSON(build_settings, builder, targets);
  if (!ContentsEqual(output_path, json)) {
    if (!WriteFileIfChanged(output_path, json, err)) {
      return false;
    }

    if (!exec_script.empty()) {
      SourceFile script_file;
      if (exec_script[0] != '/') {
        // Relative path, assume the base is in build_dir.
        script_file = build_settings->build_dir().ResolveRelativeFile(
            Value(nullptr, exec_script), err);
        if (script_file.is_null()) {
          return false;
        }
      } else {
        script_file = SourceFile(exec_script);
      }
      base::FilePath interpreter_path = exec_script_interpreter.empty() ?
          build_settings->python_path() : exec_script_interpreter;
      base::FilePath script_path = build_settings->GetFullPath(script_file);
      return InvokeScript(build_settings, interpreter_path, script_path,
                          exec_script_extra_args, output_path, quiet, err);
    }
  }

  return true;
}

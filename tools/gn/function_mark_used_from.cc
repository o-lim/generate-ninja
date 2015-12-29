// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/err.h"
#include "tools/gn/functions.h"
#include "tools/gn/parse_tree.h"
#include "tools/gn/scope.h"

namespace functions {

namespace {

void MarkUsedAllValues(const FunctionCallNode* function,
                       Scope* source) {
  source->MarkAllUsed();
}

void MarkUsedFromList(Scope* source,
                      const std::vector<Value>& list,
                      Err* err) {
  for (const Value& cur : list) {
    if (!cur.VerifyTypeIs(Value::STRING, err))
      return;
    source->GetValue(cur.string_value(), true);
  }
}

}  // namespace

const char kMarkUsedFrom[] = "mark_used_from";
const char kMarkUsedFrom_HelpShort[] =
    "mark_used_from: Marks variables as used from a different scope.";
const char kMarkUsedFrom_Help[] =
    "mark_used_from: Marks variables as used from a different scope.\n"
    "\n"
    "  mark_used_from(from_scope, variable_list_or_star)\n"
    "\n"
    "  Marks the given variables from the given scope as used if they exist.\n"
    "  This is normally used in the context of templates to prevent\n"
    "  \"Assignment had no effect\" errors.\n"
    "\n"
    "  The variables in the given variable_list will be marked used if they\n"
    "  exist in the given scope or any enclosing scope. If they do not exist,\n"
    "  nothing will happen.\n"
    "\n"
    "  As a special case, if the variable_list is a string with the value of\n"
    "  \"*\", all variables from the given scope will be marked used. \"*\"\n"
    "  only marks variables used that exist directly on the from_scope, not\n"
    "  enclosing ones. Otherwise it would mark all global variables as used.\n"
    "\n"
    "  See also \"forward_variables_from\" for copying variables from a.\n"
    "  different scope.\n"
    "\n"
    "Examples\n"
    "\n"
    "  # This is a common action template. It would invoke a script with\n"
    "  # some given parameters, and wants to use the various types of deps\n"
    "  # and the visibility from the invoker if it's defined. It also injects\n"
    "  # an additional dependency to all targets depending on the visibility\n"
    "  # flag.\n"
    "  template(\"my_test\") {\n"
    "    action(target_name) {\n"
    "      forward_variables_from(invoker, [ \"data_deps\", \"deps\",\n"
    "                                        \"public_deps\", \"visibility\" "
                                                                         "])\n"
    "      if (defined(visibility) && visibility) {\n"
    "        if (defined(invoker.extra_deps)) {\n"
    "          # Add these extra deps to the dependencies.\n"
    "          # \"deps\" may or may not be defined at this point.\n"
    "          if (defined(deps)) {\n"
    "            deps += invoker.extra_deps\n"
    "          } else {\n"
    "            deps = invoker.extra_deps\n"
    "          }\n"
    "        }\n"
    "      } else {\n"
    "        # Don't do anything with these extra deps.\n"
    "        mark_used_from(invoker, [ \"extra_deps\" ])\n"
    "      }\n"
    "    }\n"
    "  }\n"
    "\n"
    "  # This is a template around a target whose type depends on a global\n"
    "  # variable. It marks all values from the invoker as used.\n"
    "  template(\"my_wrapper\") {\n"
    "    target(my_wrapper_target_type, target_name) {\n"
    "      mark_used_from(invoker, \"*\")\n"
    "    }\n"
    " }\n";

// This function takes a ListNode rather than a resolved vector of values
// both avoid copying the potentially-large source scope, and so the variables
// in the source scope can be marked as used.
Value RunMarkUsedFrom(Scope* scope,
                      const FunctionCallNode* function,
                      const ListNode* args_list,
                      Err* err) {
  const std::vector<const ParseNode*>& args_vector = args_list->contents();
  if (args_vector.size() != 2) {
    *err = Err(function, "Wrong number of arguments.",
               "Expecting exactly two.");
    return Value();
  }

  // Extract the scope identifier. This assumes the first parameter is an
  // identifier. It is difficult to write code where this is not the case, and
  // this saves an expensive scope copy. If necessary, this could be expanded
  // to execute the ParseNode and get the value out if it's not an identifer.
  const IdentifierNode* identifier = args_vector[0]->AsIdentifier();
  if (!identifier) {
    *err = Err(args_vector[0], "Expected an identifier for the scope.");
    return Value();
  }

  // Extract the source scope.
  Value* value = scope->GetMutableValue(identifier->value().value(), true);
  if (!value) {
    *err = Err(identifier, "Undefined identifier.");
    return Value();
  }
  if (!value->VerifyTypeIs(Value::SCOPE, err))
    return Value();
  Scope* source = value->scope_value();

  // Extract the list. If all_values is not set, the what_value will be a list.
  Value what_value = args_vector[1]->Execute(scope, err);
  if (err->has_error())
    return Value();
  if (what_value.type() == Value::STRING) {
    if (what_value.string_value() == "*") {
      MarkUsedAllValues(function, source);
      return Value();
    }
  } else {
    if (what_value.type() == Value::LIST) {
      MarkUsedFromList(source, what_value.list_value(), err);
      return Value();
    }
  }

  // Not the right type of argument.
  *err = Err(what_value, "Not a valid list of variables to mark used.",
             "Expecting either the string \"*\" or a list of strings.");
  return Value();
}

}  // namespace functions

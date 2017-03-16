// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/err.h"
#include "tools/gn/functions.h"
#include "tools/gn/parse_tree.h"
#include "tools/gn/scope.h"

namespace functions {

namespace {

void MarkUsedIdentifier(Scope* source,
                        const Token& ident,
                        Err* err) {
  const Value* value = source->GetValue(ident.value(), true);
  if (!value)
    *err = Err(ident, "Undefined identifier.");
}

void MarkUsedValue(Scope* source,
                   const Value& cur,
                   Err* err) {
  if (!cur.VerifyTypeIs(Value::STRING, err))
    return;
  const Value* value = source->GetValue(cur.string_value(), true);
  if (!value)
    *err = Err(cur, "Undefined identifier.");
}

void MarkUsedFromList(Scope* source,
                      const std::vector<Value>& list,
                      Err* err) {
  for (const Value& cur : list) {
    if (!cur.VerifyTypeIs(Value::STRING, err))
      return;
    const Value* value = source->GetValue(cur.string_value(), true);
    if (!value) {
      *err = Err(cur, "Undefined identifier.");
      return;
    }
  }
}

}  // namespace

const char kMarkUsed[] = "mark_used";
const char kMarkUsed_HelpShort[] =
    "mark_used_from: Marks variables as used from the current scope.";
const char kMarkUsed_Help[] =
    R"(mark_used: Marks variables as used from the current scope.

  mark_used(variable_name_or_variable_list)

  Marks the given variables from the current scope as used. This can be used in
  the context of templates to prevent "Assignment had no effect" errors.

  The variables in the given variable_list will be marked used in the current
  scope or any enclosing scope.

  See also "mark_used_from" for marking variables used from a different scope.
)";

// This function takes a ListNode rather than a resolved vector of values
// both avoid copying the potentially-large source scope, and so the variables
// in the source scope can be marked as used.
Value RunMarkUsed(Scope* scope,
                  const FunctionCallNode* function,
                  const ListNode* args_list,
                  Err* err) {
  const auto& args_vector = args_list->contents();
  if (args_vector.size() != 1) {
    *err = Err(function, "Wrong number of arguments.",
               "Expecting exactly one.");
    return Value();
  }

  // Extract identifier.
  const IdentifierNode* identifier = args_vector[0]->AsIdentifier();
  if (identifier) {
    MarkUsedIdentifier(scope, identifier->value(), err);
    return Value();
  }

  // Extract list or string.
  Value what_value = args_vector[0]->Execute(scope, err);
  if (err->has_error())
    return Value();
  if (what_value.type() == Value::LIST) {
    MarkUsedFromList(scope, what_value.list_value(), err);
    return Value();
  } else {
    if (what_value.type() == Value::STRING) {
      MarkUsedValue(scope, what_value, err);
      return Value();
    }
  }

  // Not the right type of argument.
  *err = Err(what_value, "Not a valid list of variables to mark used.",
             "Expecting an identifier, a string, or a list of strings.");
  return Value();
}

}  // namespace functions

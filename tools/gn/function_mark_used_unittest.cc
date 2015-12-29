// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "tools/gn/scheduler.h"
#include "tools/gn/test_with_scope.h"

TEST(FunctionMarkUsed, List) {
  Scheduler scheduler;
  TestWithScope setup;

  // Defines a template and copy the two x and y, and z values out.
  TestParseInput input(
    "template(\"a\") {\n"
    "  forward_variables_from(invoker, \"*\")\n"
    "  mark_used([\"x\", \"y\"])\n"
    "  print(\"$target_name\")\n"  // Prevent unused var error.
    "}\n"
    "a(\"target\") {\n"
    "  x = 1\n"
    "  y = 2\n"
    "}\n");

  ASSERT_FALSE(input.has_error());

  Err err;
  input.parsed()->Execute(setup.scope(), &err);
  EXPECT_FALSE(err.has_error()) << err.message();

  EXPECT_EQ("target\n", setup.print_output());
  setup.print_output().clear();
}

TEST(FunctionMarkUsed, String) {
  Scheduler scheduler;
  TestWithScope setup;

  TestParseInput input(
    "template(\"a\") {\n"
    "  forward_variables_from(invoker, \"*\")\n"
    "  mark_used(\"x\")\n"
    "  print(\"$target_name\")\n"  // Prevent unused var error.
    "}\n"
    "a(\"target\") {\n"
    "  x = 1\n"
    "}\n");

  ASSERT_FALSE(input.has_error());

  Err err;
  input.parsed()->Execute(setup.scope(), &err);
  EXPECT_FALSE(err.has_error()) << err.message();

  EXPECT_EQ("target\n", setup.print_output());
  setup.print_output().clear();
}

TEST(FunctionMarkUsed, Identifier) {
  Scheduler scheduler;
  TestWithScope setup;

  TestParseInput input(
    "template(\"a\") {\n"
    "  forward_variables_from(invoker, \"*\")\n"
    "  mark_used(x)\n"
    "  mark_used(target_name)\n"
    "}\n"
    "a(\"target\") {\n"
    "  x = 1\n"
    "}\n");

  ASSERT_FALSE(input.has_error());

  Err err;
  input.parsed()->Execute(setup.scope(), &err);
  EXPECT_FALSE(err.has_error()) << err.message();
}

TEST(FunctionMarkUsed, ErrorCases) {
  Scheduler scheduler;
  TestWithScope setup;

  // Type check the input
  TestParseInput invalid_identifier(
    "template(\"a\") {\n"
    "  print(\"$target_name\")\n"  // Prevent unused var error.
    "}\n"
    "a(\"target\") {\n"
    "  mark_used(42)\n"
    "}\n");
  ASSERT_FALSE(invalid_identifier.has_error());
  Err err;
  invalid_identifier.parsed()->Execute(setup.scope(), &err);
  EXPECT_TRUE(err.has_error());
  EXPECT_EQ("Not a valid list of variables to mark used.", err.message());

  // Check the identifier name in the list
  TestParseInput undefined_identifier_list(
    "template(\"b\") {\n"
    "  print(\"$target_name\")\n"  // Prevent unused var error.
    "}\n"
    "b(\"target\") {\n"
    "  mark_used([\"z\"])\n"
    "}\n");
  ASSERT_FALSE(undefined_identifier_list.has_error());
  err = Err();
  undefined_identifier_list.parsed()->Execute(setup.scope(), &err);
  EXPECT_TRUE(err.has_error());
  EXPECT_EQ("Undefined identifier.", err.message());

  // Check the identifier
  TestParseInput undefined_identifier(
    "template(\"c\") {\n"
    "  print(\"$target_name\")\n"  // Prevent unused var error.
    "}\n"
    "c(\"target\") {\n"
    "  mark_used(z)\n"
    "}\n");
  ASSERT_FALSE(undefined_identifier.has_error());
  err = Err();
  undefined_identifier.parsed()->Execute(setup.scope(), &err);
  EXPECT_TRUE(err.has_error());
  EXPECT_EQ("Undefined identifier.", err.message());
}

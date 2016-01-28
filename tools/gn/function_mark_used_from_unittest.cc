// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "tools/gn/scheduler.h"
#include "tools/gn/test_with_scope.h"

TEST(FunctionMarkUsedFrom, List) {
  Scheduler scheduler;
  TestWithScope setup;

  // Defines a template and copy the two x and y, and z values out.
  TestParseInput input(
    "template(\"a\") {\n"
    "  mark_used_from(invoker, [\"x\", \"y\", \"z\"])\n"
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

TEST(FunctionMarkUsedFrom, ErrorCases) {
  Scheduler scheduler;
  TestWithScope setup;

  // Type check the source scope.
  TestParseInput invalid_source(
    "template(\"a\") {\n"
    "  mark_used_from(42, [\"x\"])\n"
    "  print(\"$target_name\")\n"  // Prevent unused var error.
    "}\n"
    "a(\"target\") {\n"
    "}\n");
  ASSERT_FALSE(invalid_source.has_error());
  Err err;
  invalid_source.parsed()->Execute(setup.scope(), &err);
  EXPECT_TRUE(err.has_error());
  EXPECT_EQ("Expected an identifier for the scope.", err.message());

  // Type check the list. We need to use a new template name each time since
  // all of these invocations are executing in sequence in the same scope.
  TestParseInput invalid_list(
    "template(\"b\") {\n"
    "  mark_used_from(invoker, 42)\n"
    "  print(\"$target_name\")\n"
    "}\n"
    "b(\"target\") {\n"
    "}\n");
  ASSERT_FALSE(invalid_list.has_error());
  err = Err();
  invalid_list.parsed()->Execute(setup.scope(), &err);
  EXPECT_TRUE(err.has_error());
  EXPECT_EQ("Not a valid list of variables to mark used.", err.message());

  // Type check the exclusion list.
  TestParseInput invalid_exclusion_list(
    "template(\"c\") {\n"
    "  mark_used_from(invoker, \"*\", 42)\n"
    "  print(\"$target_name\")\n"
    "}\n"
    "c(\"target\") {\n"
    "}\n");
  ASSERT_FALSE(invalid_exclusion_list.has_error());
  err = Err();
  invalid_exclusion_list.parsed()->Execute(setup.scope(), &err);
  EXPECT_TRUE(err.has_error());
  EXPECT_EQ("Not a valid list of variables to exclude.", err.message());

  // Programmatic values should not error.
  TestParseInput prog(
    "template(\"d\") {\n"
    "  mark_used_from(invoker, [\"root_out_dir\"])\n"
    "  print(\"$target_name\")\n"
    "}\n"
    "d(\"target\") {\n"
    "}\n");
  ASSERT_FALSE(prog.has_error());
  err = Err();
  prog.parsed()->Execute(setup.scope(), &err);
  EXPECT_FALSE(err.has_error());
}

TEST(FunctionMarkUsedFrom, Star) {
  Scheduler scheduler;
  TestWithScope setup;

  // Defines a template and marks the two x and y values used using the "*"
  // behavior.
  TestParseInput input(
    "template(\"a\") {\n"
    "  mark_used_from(invoker, \"*\")\n"
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

TEST(FunctionMarkUsedFrom, StarWithExclusion) {
  Scheduler scheduler;
  TestWithScope setup;

  // Defines a template mark all values except z used.
  TestParseInput input(
    "template(\"a\") {\n"
    "  mark_used_from(invoker, \"*\", [\"z\"])\n"
    "  print(\"$target_name\")\n"
    "}\n"
    "a(\"target\") {\n"
    "  x = 1\n"
    "  y = 2\n"
    "  z = 3\n"
    "  print(\"$z\")\n"
    "}\n");

  ASSERT_FALSE(input.has_error());

  Err err;
  input.parsed()->Execute(setup.scope(), &err);
  ASSERT_FALSE(err.has_error()) << err.message();

  EXPECT_EQ("3\ntarget\n", setup.print_output());
  setup.print_output().clear();

  // Instantiates a template that marks all values except z used.
  // z should have an "Assignment had not effect." error.
  TestParseInput input2(
    "a(\"target\") {\n"
    "  x = 1\n"
    "  y = 2\n"
    "  z = 3\n"
    "}\n");

  ASSERT_FALSE(input.has_error());

  err = Err();
  input2.parsed()->Execute(setup.scope(), &err);
  EXPECT_EQ("Assignment had no effect.", err.message());

  EXPECT_EQ("target\n", setup.print_output());
  setup.print_output().clear();
}

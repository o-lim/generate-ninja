// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/gtest/include/gtest/gtest.h"
#include "tools/gn/functions.h"
#include "tools/gn/target.h"
#include "tools/gn/test_with_scope.h"

namespace {

class GetTargetOutputsTest : public testing::Test {
 public:
  GetTargetOutputsTest() {
    setup_.scope()->set_item_collector(&items_);
  }

  Value GetTargetOutputs(const std::string& name, Err* err) {
    FunctionCallNode function;
    std::vector<Value> args;
    args.push_back(Value(nullptr, name));
    return functions::RunGetTargetOutputs(setup_.scope(), &function, args, err);
  }

  // Shortcut to get a label with the current toolchain.
  Label GetLabel(const std::string& dir, const std::string& name) {
    return Label(SourceDir(dir), name, setup_.toolchain()->label().dir(),
                 setup_.toolchain()->label().name());
  }

  // Asserts that the given list contains a single string with the given value.
  void AssertSingleStringEquals(const Value& list,
                                const std::string& expected) {
    ASSERT_TRUE(list.type() == Value::LIST);
    ASSERT_EQ(1u, list.list_value().size());
    ASSERT_TRUE(list.list_value()[0].type() == Value::STRING);
    ASSERT_EQ(expected, list.list_value()[0].string_value());
  }

  void AssertTwoStringsEqual(const Value& list,
                             const std::string& expected1,
                             const std::string& expected2) {
    ASSERT_TRUE(list.type() == Value::LIST);
    ASSERT_EQ(2u, list.list_value().size());
    ASSERT_TRUE(list.list_value()[0].type() == Value::STRING);
    ASSERT_EQ(expected1, list.list_value()[0].string_value());
    ASSERT_TRUE(list.list_value()[1].type() == Value::STRING);
    ASSERT_EQ(expected2, list.list_value()[1].string_value());
  }

 protected:
  TestWithScope setup_;

  Scope::ItemVector items_;
};

}  // namespace

TEST_F(GetTargetOutputsTest, Copy) {
  Target* copy = new Target(setup_.settings(), GetLabel("//foo/", "bar"));
  copy->set_output_type(Target::COPY_FILES);
  copy->sources().push_back(SourceFile("//file.txt"));
  copy->action_values().outputs() =
      SubstitutionList::MakeForTest("//out/Debug/{{source_file_part}}.one");

  items_.push_back(copy);

  Err err;
  Value result = GetTargetOutputs("//foo:bar", &err);
  ASSERT_FALSE(err.has_error());
  AssertSingleStringEquals(result, "//out/Debug/file.txt.one");
}

TEST_F(GetTargetOutputsTest, Action) {
  Target* action = new Target(setup_.settings(), GetLabel("//foo/", "bar"));
  action->set_output_type(Target::ACTION);
  action->action_values().outputs() = SubstitutionList::MakeForTest(
      "//output1.txt",
      "//output2.txt");

  items_.push_back(action);

  Err err;
  Value result = GetTargetOutputs("//foo:bar", &err);
  ASSERT_FALSE(err.has_error());
  AssertTwoStringsEqual(result, "//output1.txt", "//output2.txt");
}

TEST_F(GetTargetOutputsTest, ActionForeach) {
  Target* action = new Target(setup_.settings(), GetLabel("//foo/", "bar"));
  action->set_output_type(Target::ACTION_FOREACH);
  action->sources().push_back(SourceFile("//file.txt"));
  action->action_values().outputs() = SubstitutionList::MakeForTest(
      "//out/Debug/{{source_file_part}}.one",
      "//out/Debug/{{source_file_part}}.two");

  items_.push_back(action);

  Err err;
  Value result = GetTargetOutputs("//foo:bar", &err);
  ASSERT_FALSE(err.has_error());
  AssertTwoStringsEqual(result, "//out/Debug/file.txt.one",
                        "//out/Debug/file.txt.two");
}

TEST_F(GetTargetOutputsTest, SourceSet) {
  Target* source_set = new Target(setup_.settings(), GetLabel("//foo/", "bar"));
  source_set->set_output_type(Target::SOURCE_SET);
  source_set->sources().push_back(SourceFile("//foo/file1.cc"));
  source_set->sources().push_back(SourceFile("//foo/file2.cc"));

  items_.push_back(source_set);

  Err err;
  Value result = GetTargetOutputs("//foo:bar", &err);
  ASSERT_FALSE(err.has_error());
  AssertTwoStringsEqual(result, "//out/Debug/obj/foo/bar.file1.o",
                        "//out/Debug/obj/foo/bar.file2.o");
}

TEST_F(GetTargetOutputsTest, Executable) {
  Target* executable = new Target(setup_.settings(), GetLabel("//foo/", "bar"));
  executable->set_output_type(Target::EXECUTABLE);
  executable->sources().push_back(SourceFile("//file.cc"));

  items_.push_back(executable);

  Err err;
  Value result = GetTargetOutputs("//foo:bar", &err);
  ASSERT_FALSE(err.has_error());
  AssertSingleStringEquals(result, "//out/Debug/bar");
}

TEST_F(GetTargetOutputsTest, LoadableModule) {
  Target* loadable_module = new Target(setup_.settings(), GetLabel("//foo/", "bar"));
  loadable_module->set_output_type(Target::LOADABLE_MODULE);
  loadable_module->sources().push_back(SourceFile("//file.cc"));

  items_.push_back(loadable_module);

  Err err;
  Value result = GetTargetOutputs("//foo:bar", &err);
  ASSERT_FALSE(err.has_error());
  AssertSingleStringEquals(result, "//out/Debug/libbar.so");
}

TEST_F(GetTargetOutputsTest, SharedLibrary) {
  Target* shared_library = new Target(setup_.settings(), GetLabel("//foo/", "bar"));
  shared_library->set_output_type(Target::SHARED_LIBRARY);
  shared_library->sources().push_back(SourceFile("//file.cc"));

  items_.push_back(shared_library);

  Err err;
  Value result = GetTargetOutputs("//foo:bar", &err);
  ASSERT_FALSE(err.has_error());
  AssertSingleStringEquals(result, "//out/Debug/libbar.so");
}

TEST_F(GetTargetOutputsTest, StaticLibrary) {
  Target* static_library = new Target(setup_.settings(), GetLabel("//foo/", "bar"));
  static_library->set_output_type(Target::STATIC_LIBRARY);
  static_library->sources().push_back(SourceFile("//file.cc"));

  items_.push_back(static_library);

  Err err;
  Value result = GetTargetOutputs("//foo:bar", &err);
  ASSERT_FALSE(err.has_error());
  AssertSingleStringEquals(result, "//out/Debug/obj/foo/libbar.a");
}

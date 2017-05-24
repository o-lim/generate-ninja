# Generate-Ninja

[![travis-ci status](https://travis-ci.org/o-lim/generate-ninja.svg?branch=master)](https://travis-ci.org/o-lim/generate-ninja/builds)
[![appveyor-ci status](https://ci.appveyor.com/api/projects/status/al342b4i56m1hs27/branch/master?svg=true)](https://ci.appveyor.com/project/o-lim/generate-ninja/branch/master)

Generate-Ninja, or GN, is a meta-build system that generates [Ninja](https://ninja-build.org)
build files so that you can build your project with Ninja.

GN is, originally, part of the [Chromium](https://chromium.googlesource.com/chromium/src)
source tree. However, this is a fork of the original [GN](https://chromium.googlesource.com/chromium/src/+/master/tools/gn),
which has been extracted from the Chromium tree so that it may be built
standalone.

The original source code was imported from [Chromium](https://chromium.googlesource.com/chromium/src.git)
with the following files/directories exported directly via `git archive`:

 - .gn
 - BUILD.gn
 - LICENSE
 - LICENSE.chromium\_os
 - base
 - build
 - build\_overrides
 - tools/gn
 - testing/test.gni
 - testing/perf
 - testing/libfuzzer
 - testing/\*.h
 - testing/\*.cc
 - testing/\*.mm
 - third\_party/apple\_apsl
 - third\_party/ced
 - third\_party/googletest
 - third\_party/libxml
 - third\_party/modp\_b64
 - third\_party/zlib

and the following git sub-repos exported into their respective directories:

 - testing/gmock
 - testing/gtest
 - third\_party/ced/src
 - third\_party/icu
 - third\_party/googletest/src

Furthermore, this fork adds some minor features as well as some bug fixes:

 - Create build dir when necessary if it does not exist
 - Change ordering of libs and config flags such that dependent libs and config flags have the least precedence
 - Fix bootstrap builds
 - Fix unresolved dependencies for sub-configs
 - Fix invalid characters list for ninja rule names
 - Fix help messages title for GN commands
 - Fix duplicate pch ninja rules
 - Fix action targets not using substitutions
 - Fix errors when root build dir equals source root
 - Fix static library link order to fix unresolved symbol link errors
 - Fix `--all-toolchains` flag for `gn ls` and `gn refs`
 - Support source substitutions for binary targets
 - Support for `command` in action targets
 - Support for `description` in action targets
 - Support for `sys_include_dirs`
 - Support for `cppflags` and `asmppflags`
 - Support for source extension substitution
 - Support specifying tool source file extentions
 - Support specifying object file extentions
 - Support specifying linker script extentions
 - Support custom interpreter for action scripts and exec scripts
 - Support custom interpreter for JSON IDE scripts
 - Support `--all-toolchains` option for QtCreator generator
 - Add `define_switch` and `include_switch` variables to toolchain function
 - Add `color_console` built-in variable
 - Allow `gn desc` to support multiple target labels
 - Allow clobbering in `forward_variables_from`
 - Use console pool to regenerate ninja files
 - Use abs path for sources when output dir is outside root dir

For more information on GN and Ninja, please refer to the following links:

 - [Quick Start Guide](tools/gn/docs/quick_start.md)
 - [FAQ](tools/gn/docs/faq.md)
 - [Language and Operation Details](tools/gn/docs/language.md)
 - [Reference](tools/gn/docs/reference.md) The built-in `gn help` documentation
 - [Style Guide](tools/gn/docs/style_guide.md)
 - [Cross compiling and toolchains](tools/gn/docs/cross_compiles.md)
 - [Ninja Manual](https://ninja-build.org/manual.html)

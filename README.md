# Generate-Ninja

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
 - third\_party/libxml
 - third\_party/modp\_b64
 - third\_party/zlib

and the following git submodules exported into their respective directories:

 - testing/gmock
 - testing/gtest
 - third\_party/ced/src
 - third\_party/icu

Furthermore, this fork adds some minor features as well as some minor bug fixes:

 - Create build dir when necessary if it does not exist
 - Change ordering of libs and config flags such that dependent libs and config flags have the least precedence
 - Fix static library link order to fix unresolved symbol link errors
 - Fix invalid characters list for ninja rule names
 - Fix help messages title for GN commands
 - Fix duplicate pch ninja rules
 - Fix errors when root build dir equals source root
 - Fix `-`(minus) operator
 - Fix `--all-toolchains` flag for `gn ls` and `gn refs`
 - Fix action targets not using substitutions
 - Support source substitutions for binary targets
 - Support for `command` in action targets
 - Support for `description` in action targets
 - Support for `sys_include_dirs`
 - Support for `cppflags` and `asmppflags`
 - Support for source extension substitution
 - Support specifying tool source file extentions
 - Support specifying object file extentions
 - Support specifying linker script extentions
 - Support custom intepreter for action scripts and exec scripts
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

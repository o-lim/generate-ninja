# Generate-Ninja

[![travis-ci status](https://travis-ci.org/o-lim/generate-ninja.svg?branch=master)](https://travis-ci.org/o-lim/generate-ninja/builds)
[![appveyor-ci status](https://ci.appveyor.com/api/projects/status/al342b4i56m1hs27/branch/master?svg=true)](https://ci.appveyor.com/project/o-lim/generate-ninja/branch/master)

Generate-Ninja, or GN, is a meta-build system that generates [Ninja](https://ninja-build.org)
build files so that you can build your project with Ninja.

GN was, originally, part of the [Chromium](https://chromium.googlesource.com/chromium/src)
source tree, and has since been extracted into its own standalone repo. However,
this is a fork of the official [GN](https://gn.googlesource.com/gn), adding some
minor features as well as some bug fixes:

 - Create build dir when necessary if it does not exist
 - Change ordering of libs and config flags such that dependent libs and config flags have the least precedence
 - Separate bootstrap vs integration builds
 - Fix unresolved dependencies for sub-configs
 - Fix invalid characters list for ninja rule names
 - Fix help messages title for GN commands
 - Fix duplicate pch ninja rules
 - Fix action targets not using substitutions
 - Fix errors when root build dir equals source root
 - Fix static library link order to fix unresolved symbol link errors
 - Fix `--all-toolchains` flag for `gn ls` and `gn refs`
 - Fix `gn args` regenerating ninja files with different command-line switches
 - Fix `gn desc` when showing `outputs` only
 - Allow `gn desc` to support multiple target labels
 - Support binary targets for `get_target_outputs` function
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
 - Support separate compilation database per toolchain
 - Add `define_switch` and `include_switch` variables to toolchain function
 - Add `color_console` built-in variable
 - Add `console_pool` built-in variable
 - Add `gn_version` built-in variable
 - Allow `gn desc` to support multiple target labels
 - Allow clobbering in `forward_variables_from`
 - Use console pool to regenerate ninja files
 - Use abs path for sources when output dir is outside root dir

For more information on GN and Ninja, please refer to the following links:

 - [Quick Start Guide](docs/quick_start.md)
 - [FAQ](docs/faq.md)
 - [Language and Operation Details](docs/language.md)
 - [Reference](docs/reference.md) The built-in `gn help` documentation
 - [Style Guide](docs/style_guide.md)
 - [Cross compiling and toolchains](docs/cross_compiles.md)
 - [Ninja Manual](https://ninja-build.org/manual.html)

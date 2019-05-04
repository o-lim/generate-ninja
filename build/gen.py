#!/usr/bin/env python
# Copyright 2014 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

"""Generates build.ninja that will build GN."""

import contextlib
import errno
import optparse
import os
import platform
import re
import subprocess
import sys
import tempfile
import last_commit_position

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
REPO_ROOT = os.path.dirname(SCRIPT_DIR)
GN_ROOT = os.path.join(REPO_ROOT, 'tools', 'gn')


class Platform(object):
  """Represents a host/target platform."""
  def __init__(self, platform):
    self._platform = platform
    if self._platform is not None:
      return
    self._platform = sys.platform
    if self._platform.startswith('linux'):
      self._platform = 'linux'
    elif self._platform.startswith('darwin'):
      self._platform = 'darwin'
    elif self._platform.startswith('mingw'):
      self._platform = 'mingw'
    elif self._platform.startswith('win'):
      self._platform = 'msvc'
    elif self._platform.startswith('aix'):
      self._platform = 'aix'
    elif self._platform.startswith('fuchsia'):
      self._platform = 'fuchsia'
    elif self._platform.startswith('freebsd'):
      self._platform = 'freebsd'

  @staticmethod
  def known_platforms():
    return ['linux', 'darwin', 'msvc', 'aix', 'fuchsia']

  def platform(self):
    return self._platform

  def is_linux(self):
    return self._platform == 'linux'

  def is_mingw(self):
    return self._platform == 'mingw'

  def is_msvc(self):
    return self._platform == 'msvc'

  def is_windows(self):
    return self.is_mingw() or self.is_msvc()

  def is_darwin(self):
    return self._platform == 'darwin'

  def is_aix(self):
    return self._platform == 'aix'

  def is_posix(self):
    return self._platform in ['linux', 'freebsd', 'darwin', 'aix']

def windows_target_build_arch():
  # Target build architecture set by vcvarsall.bat
  target_arch = os.environ.get('Platform')
  if target_arch in ['x64', 'x86']: return target_arch

  if platform.machine().lower() in ['x86_64', 'amd64']: return 'x64'
  return 'x86'

def main(argv):
  parser = optparse.OptionParser(description=sys.modules[__name__].__doc__)
  parser.add_option('-d', '--debug', action='store_true',
                    help='Do a debug build. Defaults to release build.')
  parser.add_option('--platform',
                    help='target platform (' +
                         '/'.join(Platform.known_platforms()) + ')',
                    choices=Platform.known_platforms())
  parser.add_option('--host',
                    help='host platform (' +
                         '/'.join(Platform.known_platforms()) + ')',
                    choices=Platform.known_platforms())
  parser.add_option('--use-lto', action='store_true',
                    help='Enable the use of LTO')
  parser.add_option('--use-icf', action='store_true',
                    help='Enable the use of Identical Code Folding')
  parser.add_option('--no-last-commit-position', action='store_true',
                    help='Do not generate last_commit_position.h.')
  parser.add_option('--out-path',
                    help='The path to generate the build files in.')
  parser.add_option('--no-strip', action='store_true',
                    help='Don\'t strip release build. Useful for profiling.')
  options, args = parser.parse_args(argv)

  if args:
    parser.error('Unrecognized command line arguments: %s.' % ', '.join(args))

  platform = Platform(options.platform)
  if options.host:
    host = Platform(options.host)
  else:
    host = platform

  out_dir = options.out_path or os.path.join(REPO_ROOT, 'out')
  if not os.path.isdir(out_dir):
    os.makedirs(out_dir)
  if not options.no_last_commit_position:
    GenerateLastCommitPosition(host,
                               os.path.join(out_dir, 'tools/gn/last_commit_position.h'), 'TOOLS_GN_LAST_COMMIT_POSITION_H_')
  WriteGNNinja(os.path.join(out_dir, 'build.ninja'), platform, host, options)
  return 0


def GenerateLastCommitPosition(host, header, header_guard):
  version = '$Format:%h$'
  if version.startswith('$'):
    version = last_commit_position.FetchCommitPosition('.')

  last_commit_position.WriteHeader(header, header_guard, version)


def WriteGenericNinja(path, static_libraries, executables,
                      cc, cxx, ar, ld, platform, host, options,
                      cflags=[], cflags_cc=[], arflags=[],
                      ldflags=[], libflags=[],
                      include_dirs=[], solibs=[]):
  ninja_header_lines = [
    'cc = ' + cc,
    'cxx = ' + cxx,
    'ar = ' + ar,
    'ld = ' + ld,
    '',
    'rule regen',
    '  command = %s ../../build/gen.py%s --out-path .' % (
        sys.executable, ' -d' if options.debug else ''),
    '  description = Regenerating ninja files',
    '',
    'build build.ninja: regen',
    '  generator = 1',
    '  depfile = build.ninja.d',
    '',
  ]


  template_filename = os.path.join(SCRIPT_DIR, {
      'msvc': 'build_win.ninja.template',
      'darwin': 'build_mac.ninja.template',
      'linux': 'build_linux.ninja.template',
      'freebsd': 'build_linux.ninja.template',
      'aix': 'build_aix.ninja.template',
  }[platform.platform()])

  with open(template_filename) as f:
    ninja_template = f.read()

  if platform.is_windows():
    executable_ext = '.exe'
    library_ext = '.lib'
    object_ext = '.obj'
  else:
    executable_ext = ''
    library_ext = '.a'
    object_ext = '.o'

  def escape_path_ninja(path):
      return path.replace('$ ', '$$ ').replace(' ', '$ ').replace(':', '$:')

  def src_to_obj(path):
    return escape_path_ninja('%s' % os.path.splitext(path)[0] + object_ext)

  def library_to_a(library):
    return '%s%s' % (library, library_ext)

  ninja_lines = []
  def build_source(src_file, settings):
    ninja_lines.extend([
        'build %s: %s %s' % (src_to_obj(src_file),
                             settings['tool'],
                             escape_path_ninja(
                                 os.path.join(REPO_ROOT, src_file))),
        '  includes = %s' % ' '.join(
            ['-I' + escape_path_ninja(dirname) for dirname in
             include_dirs + settings.get('include_dirs', [])]),
        '  cflags = %s' % ' '.join(cflags + settings.get('cflags', [])),
        '  cflags_cc = %s' %
            ' '.join(cflags_cc + settings.get('cflags_cc', [])),
    ])

  for library, settings in static_libraries.iteritems():
    for src_file in settings['sources']:
      build_source(src_file, settings)

    ninja_lines.append('build %s: alink_thin %s' % (
        library_to_a(library),
        ' '.join([src_to_obj(src_file) for src_file in settings['sources']])))
    ninja_lines.append('  arflags = %s' % ' '.join(arflags))
    ninja_lines.append('  libflags = %s' % ' '.join(libflags))


  for executable, settings in executables.iteritems():
    for src_file in settings['sources']:
      build_source(src_file, settings)

    ninja_lines.extend([
      'build %s%s: link %s | %s' % (
          executable, executable_ext,
          ' '.join([src_to_obj(src_file) for src_file in settings['sources']]),
          ' '.join([library_to_a(library) for library in settings['libs']])),
      '  ldflags = %s' % ' '.join(ldflags),
      '  solibs = %s' % ' '.join(solibs),
      '  libs = %s' % ' '.join(
          [library_to_a(library) for library in settings['libs']]),
    ])

  ninja_lines.append('')  # Make sure the file ends with a newline.

  with open(path, 'w') as f:
    f.write('\n'.join(ninja_header_lines))
    f.write(ninja_template)
    f.write('\n'.join(ninja_lines))

  with open(path + '.d', 'w') as f:
    f.write('build.ninja: ' +
            os.path.relpath(os.path.join(SCRIPT_DIR, 'gen.py'),
                            os.path.dirname(path)) + ' ' +
            os.path.relpath(template_filename, os.path.dirname(path)) + '\n')


def WriteGNNinja(path, platform, host, options):
  if platform.is_msvc():
    cc = os.environ.get('CC', 'cl.exe')
    cxx = os.environ.get('CXX', 'cl.exe')
    ld = os.environ.get('LD', 'link.exe')
    ar = os.environ.get('AR', 'lib.exe')
  elif platform.is_aix():
    cc = os.environ.get('CC', 'gcc')
    cxx = os.environ.get('CXX', 'c++')
    ld = os.environ.get('LD', cxx)
    ar = os.environ.get('AR', 'ar -X64')
  else:
    cc = os.environ.get('CC', 'cc')
    cxx = os.environ.get('CXX', 'c++')
    ld = cxx
    ar = os.environ.get('AR', 'ar')

  cflags = os.environ.get('CFLAGS', '').split()
  cflags_cc = os.environ.get('CXXFLAGS', '').split()
  arflags = os.environ.get('ARFLAGS', '').split()
  ldflags = os.environ.get('LDFLAGS', '').split()
  libflags = os.environ.get('LIBFLAGS', '').split()
  include_dirs = [REPO_ROOT, os.path.abspath(os.path.dirname(path))]
  libs = []

  if not platform.is_msvc():
    if options.debug:
      cflags.extend(['-O0', '-g'])
    else:
      cflags.append('-DNDEBUG')
      cflags.append('-O3')
      if options.no_strip:
        cflags.append('-g')
      ldflags.append('-O3')
      # Use -fdata-sections and -ffunction-sections to place each function
      # or data item into its own section so --gc-sections can eliminate any
      # unused functions and data items.
      cflags.extend(['-fdata-sections', '-ffunction-sections'])
      ldflags.extend(['-fdata-sections', '-ffunction-sections'])
      if platform.is_darwin():
        ldflags.append('-Wl,-dead_strip')
      elif not platform.is_aix():
        # Garbage collection is done by default on aix.
        ldflags.append('-Wl,--gc-sections')

      # Omit all symbol information from the output file.
      if options.no_strip is None:
        if platform.is_darwin():
          ldflags.append('-Wl,-S')
        elif platform.is_aix():
          ldflags.append('-Wl,-s')
        else:
          ldflags.append('-Wl,-strip-all')

      # Enable identical code-folding.
      if options.use_icf and not platform.is_darwin():
        ldflags.append('-Wl,--icf=all')

    cflags.extend([
        '-D_FILE_OFFSET_BITS=64',
        '-D__STDC_CONSTANT_MACROS', '-D__STDC_FORMAT_MACROS',
        '-pthread',
        '-pipe',
        '-fno-exceptions',
        '-fno-rtti',
        '-fdiagnostics-color',
    ])
    cflags_cc.extend(['-std=c++14', '-Wno-narrowing'])

    if platform.is_linux():
      ldflags.extend([
          '-static-libstdc++',
          '-Wl,--as-needed',
      ])
      # This is needed by libc++.
      libs.extend(['-ldl', '-lrt'])
    elif platform.is_darwin():
      min_mac_version_flag = '-mmacosx-version-min=10.9'
      cflags.append(min_mac_version_flag)
      ldflags.append(min_mac_version_flag)
    elif platform.is_aix():
      cflags_cc.append('-maix64')
      ldflags.append('-maix64')

    if platform.is_posix() and not platform.is_darwin():
      ldflags.append('-pthread')

    if options.use_lto:
      cflags.extend(['-flto', '-fwhole-program-vtables'])
      ldflags.extend(['-flto', '-fwhole-program-vtables'])

  elif platform.is_msvc():
    if not options.debug:
      cflags.extend(['/O2', '/DNDEBUG', '/GL'])
      libflags.extend(['/LTCG'])
      ldflags.extend(['/LTCG', '/OPT:REF', '/OPT:ICF'])

    cflags.extend([
        '/DNOMINMAX',
        '/DUNICODE',
        '/DWIN32_LEAN_AND_MEAN',
        '/DWINVER=0x0A00',
        '/D_CRT_SECURE_NO_DEPRECATE',
        '/D_SCL_SECURE_NO_DEPRECATE',
        '/D_UNICODE',
        '/D_WIN32_WINNT=0x0A00',
        '/FS',
        '/W4',
        '/WX',
        '/Zi',
        '/wd4099',
        '/wd4100',
        '/wd4127',
        '/wd4244',
        '/wd4267',
        '/wd4505',
        '/wd4838',
        '/wd4996',
    ])
    cflags_cc.extend([
        '/GR-',
        '/D_HAS_EXCEPTIONS=0',
    ])

    target_arch = windows_target_build_arch()
    if target_arch == 'x64':
      ldflags.extend(['/DEBUG', '/MACHINE:x64'])
    else:
      ldflags.extend(['/DEBUG', '/MACHINE:x86'])

  static_libraries = {
      'base': {'sources': [
        'base/callback_internal.cc',
        'base/command_line.cc',
        'base/environment.cc',
        'base/files/file.cc',
        'base/files/file_enumerator.cc',
        'base/files/file_path.cc',
        'base/files/file_path_constants.cc',
        'base/files/file_util.cc',
        'base/files/scoped_file.cc',
        'base/files/scoped_temp_dir.cc',
        'base/json/json_parser.cc',
        'base/json/json_reader.cc',
        'base/json/json_writer.cc',
        'base/json/string_escape.cc',
        'base/logging.cc',
        'base/md5.cc',
        'base/memory/ref_counted.cc',
        'base/memory/weak_ptr.cc',
        'base/sha1.cc',
        'base/strings/string_number_conversions.cc',
        'base/strings/string_piece.cc',
        'base/strings/string_split.cc',
        'base/strings/string_util.cc',
        'base/strings/string_util_constants.cc',
        'base/strings/stringprintf.cc',
        'base/strings/utf_string_conversion_utils.cc',
        'base/strings/utf_string_conversions.cc',
        'base/third_party/icu/icu_utf.cc',
        'base/timer/elapsed_timer.cc',
        'base/value_iterators.cc',
        'base/values.cc',
      ], 'tool': 'cxx', 'include_dirs': []},
      'gn_lib': {'sources': [
        'tools/gn/action_target_generator.cc',
        'tools/gn/action_values.cc',
        'tools/gn/analyzer.cc',
        'tools/gn/args.cc',
        'tools/gn/binary_target_generator.cc',
        'tools/gn/builder.cc',
        'tools/gn/builder_record.cc',
        'tools/gn/build_settings.cc',
        'tools/gn/bundle_data.cc',
        'tools/gn/bundle_data_target_generator.cc',
        'tools/gn/bundle_file_rule.cc',
        'tools/gn/c_include_iterator.cc',
        'tools/gn/command_analyze.cc',
        'tools/gn/command_args.cc',
        'tools/gn/command_check.cc',
        'tools/gn/command_clean.cc',
        'tools/gn/command_desc.cc',
        'tools/gn/command_format.cc',
        'tools/gn/command_gen.cc',
        'tools/gn/command_help.cc',
        'tools/gn/command_meta.cc',
        'tools/gn/command_ls.cc',
        'tools/gn/command_path.cc',
        'tools/gn/command_refs.cc',
        'tools/gn/commands.cc',
        'tools/gn/compile_commands_writer.cc',
        'tools/gn/config.cc',
        'tools/gn/config_values.cc',
        'tools/gn/config_values_extractors.cc',
        'tools/gn/config_values_generator.cc',
        'tools/gn/copy_target_generator.cc',
        'tools/gn/create_bundle_target_generator.cc',
        'tools/gn/deps_iterator.cc',
        'tools/gn/desc_builder.cc',
        'tools/gn/eclipse_writer.cc',
        'tools/gn/err.cc',
        'tools/gn/escape.cc',
        'tools/gn/exec_process.cc',
        'tools/gn/filesystem_utils.cc',
        'tools/gn/function_exec_script.cc',
        'tools/gn/function_foreach.cc',
        'tools/gn/function_forward_variables_from.cc',
        'tools/gn/function_get_label_info.cc',
        'tools/gn/function_get_path_info.cc',
        'tools/gn/function_get_target_outputs.cc',
        'tools/gn/function_mark_used.cc',
        'tools/gn/function_mark_used_from.cc',
        'tools/gn/function_process_file_template.cc',
        'tools/gn/function_read_file.cc',
        'tools/gn/function_rebase_path.cc',
        'tools/gn/functions.cc',
        'tools/gn/function_set_defaults.cc',
        'tools/gn/function_set_default_toolchain.cc',
        'tools/gn/functions_target.cc',
        'tools/gn/function_template.cc',
        'tools/gn/function_toolchain.cc',
        'tools/gn/function_write_file.cc',
        'tools/gn/generated_file_target_generator.cc',
        'tools/gn/group_target_generator.cc',
        'tools/gn/header_checker.cc',
        'tools/gn/import_manager.cc',
        'tools/gn/inherited_libraries.cc',
        'tools/gn/input_conversion.cc',
        'tools/gn/input_file.cc',
        'tools/gn/input_file_manager.cc',
        'tools/gn/item.cc',
        'tools/gn/json_project_writer.cc',
        'tools/gn/label.cc',
        'tools/gn/label_pattern.cc',
        'tools/gn/lib_file.cc',
        'tools/gn/loader.cc',
        'tools/gn/location.cc',
        'tools/gn/metadata.cc',
        'tools/gn/metadata_walk.cc',
        'tools/gn/ninja_action_target_writer.cc',
        'tools/gn/ninja_binary_target_writer.cc',
        'tools/gn/ninja_build_writer.cc',
        'tools/gn/ninja_bundle_data_target_writer.cc',
        'tools/gn/ninja_copy_target_writer.cc',
        'tools/gn/ninja_create_bundle_target_writer.cc',
        'tools/gn/ninja_generated_file_target_writer.cc',
        'tools/gn/ninja_group_target_writer.cc',
        'tools/gn/ninja_target_command_util.cc',
        'tools/gn/ninja_target_writer.cc',
        'tools/gn/ninja_toolchain_writer.cc',
        'tools/gn/ninja_utils.cc',
        'tools/gn/ninja_writer.cc',
        'tools/gn/operators.cc',
        'tools/gn/output_conversion.cc',
        'tools/gn/output_file.cc',
        'tools/gn/parse_node_value_adapter.cc',
        'tools/gn/parser.cc',
        'tools/gn/parse_tree.cc',
        'tools/gn/path_output.cc',
        'tools/gn/pattern.cc',
        'tools/gn/pool.cc',
        'tools/gn/qt_creator_writer.cc',
        'tools/gn/runtime_deps.cc',
        'tools/gn/scheduler.cc',
        'tools/gn/scope.cc',
        'tools/gn/scope_per_file_provider.cc',
        'tools/gn/settings.cc',
        'tools/gn/setup.cc',
        'tools/gn/source_dir.cc',
        'tools/gn/source_file.cc',
        'tools/gn/source_file_type.cc',
        'tools/gn/standard_out.cc',
        'tools/gn/string_utils.cc',
        'tools/gn/substitution_list.cc',
        'tools/gn/substitution_pattern.cc',
        'tools/gn/substitution_type.cc',
        'tools/gn/substitution_writer.cc',
        'tools/gn/switches.cc',
        'tools/gn/target.cc',
        'tools/gn/target_generator.cc',
        'tools/gn/template.cc',
        'tools/gn/token.cc',
        'tools/gn/tokenizer.cc',
        'tools/gn/tool.cc',
        'tools/gn/toolchain.cc',
        'tools/gn/trace.cc',
        'tools/gn/value.cc',
        'tools/gn/value_extractors.cc',
        'tools/gn/variables.cc',
        'tools/gn/visibility.cc',
        'tools/gn/visual_studio_utils.cc',
        'tools/gn/visual_studio_writer.cc',
        'tools/gn/xcode_object.cc',
        'tools/gn/xcode_writer.cc',
        'tools/gn/xml_element_writer.cc',
        'util/exe_path.cc',
        'util/msg_loop.cc',
        'util/semaphore.cc',
        'util/sys_info.cc',
        'util/ticks.cc',
        'util/worker_pool.cc',
      ], 'tool': 'cxx', 'include_dirs': []},
  }

  executables = {
      'gn': {'sources': [ 'tools/gn/gn_main.cc' ],
      'tool': 'cxx', 'include_dirs': [], 'libs': []},

      'gn_unittests': { 'sources': [
        'tools/gn/action_target_generator_unittest.cc',
        'tools/gn/analyzer_unittest.cc',
        'tools/gn/args_unittest.cc',
        'tools/gn/builder_unittest.cc',
        'tools/gn/c_include_iterator_unittest.cc',
        'tools/gn/command_format_unittest.cc',
        'tools/gn/compile_commands_writer_unittest.cc',
        'tools/gn/config_unittest.cc',
        'tools/gn/config_values_extractors_unittest.cc',
        'tools/gn/escape_unittest.cc',
        'tools/gn/exec_process_unittest.cc',
        'tools/gn/filesystem_utils_unittest.cc',
        'tools/gn/function_foreach_unittest.cc',
        'tools/gn/function_forward_variables_from_unittest.cc',
        'tools/gn/function_get_label_info_unittest.cc',
        'tools/gn/function_get_path_info_unittest.cc',
        'tools/gn/function_get_target_outputs_unittest.cc',
        'tools/gn/function_process_file_template_unittest.cc',
        'tools/gn/function_rebase_path_unittest.cc',
        'tools/gn/function_template_unittest.cc',
        'tools/gn/function_toolchain_unittest.cc',
        'tools/gn/function_write_file_unittest.cc',
        'tools/gn/functions_target_unittest.cc',
        'tools/gn/functions_unittest.cc',
        'tools/gn/header_checker_unittest.cc',
        'tools/gn/inherited_libraries_unittest.cc',
        'tools/gn/input_conversion_unittest.cc',
        'tools/gn/label_pattern_unittest.cc',
        'tools/gn/label_unittest.cc',
        'tools/gn/loader_unittest.cc',
        'tools/gn/metadata_unittest.cc',
        'tools/gn/metadata_walk_unittest.cc',
        'tools/gn/ninja_action_target_writer_unittest.cc',
        'tools/gn/ninja_binary_target_writer_unittest.cc',
        'tools/gn/ninja_build_writer_unittest.cc',
        'tools/gn/ninja_bundle_data_target_writer_unittest.cc',
        'tools/gn/ninja_copy_target_writer_unittest.cc',
        'tools/gn/ninja_create_bundle_target_writer_unittest.cc',
        'tools/gn/ninja_generated_file_target_writer_unittest.cc',
        'tools/gn/ninja_group_target_writer_unittest.cc',
        'tools/gn/ninja_target_writer_unittest.cc',
        'tools/gn/ninja_toolchain_writer_unittest.cc',
        'tools/gn/operators_unittest.cc',
        'tools/gn/output_conversion_unittest.cc',
        'tools/gn/parse_tree_unittest.cc',
        'tools/gn/parser_unittest.cc',
        'tools/gn/path_output_unittest.cc',
        'tools/gn/pattern_unittest.cc',
        'tools/gn/runtime_deps_unittest.cc',
        'tools/gn/scope_per_file_provider_unittest.cc',
        'tools/gn/scope_unittest.cc',
        'tools/gn/setup_unittest.cc',
        'tools/gn/source_dir_unittest.cc',
        'tools/gn/source_file_unittest.cc',
        'tools/gn/string_utils_unittest.cc',
        'tools/gn/substitution_pattern_unittest.cc',
        'tools/gn/substitution_writer_unittest.cc',
        'tools/gn/target_unittest.cc',
        'tools/gn/template_unittest.cc',
        'tools/gn/test_with_scheduler.cc',
        'tools/gn/test_with_scope.cc',
        'tools/gn/tokenizer_unittest.cc',
        'tools/gn/unique_vector_unittest.cc',
        'tools/gn/value_unittest.cc',
        'tools/gn/visibility_unittest.cc',
        'tools/gn/visual_studio_utils_unittest.cc',
        'tools/gn/visual_studio_writer_unittest.cc',
        'tools/gn/xcode_object_unittest.cc',
        'tools/gn/xml_element_writer_unittest.cc',
        'util/test/gn_test.cc',
      ], 'tool': 'cxx', 'include_dirs': [], 'libs': []},
  }

  if platform.is_posix():
    static_libraries['base']['sources'].extend([
        'base/files/file_enumerator_posix.cc',
        'base/files/file_posix.cc',
        'base/files/file_util_posix.cc',
        'base/posix/file_descriptor_shuffle.cc',
        'base/posix/safe_strerror.cc',
        'base/strings/string16.cc',
    ])

  if platform.is_windows():
    static_libraries['base']['sources'].extend([
        'base/files/file_enumerator_win.cc',
        'base/files/file_util_win.cc',
        'base/files/file_win.cc',
        'base/win/registry.cc',
        'base/win/scoped_handle.cc',
        'base/win/scoped_process_information.cc',
    ])

    libs.extend([
        'advapi32.lib',
        'dbghelp.lib',
        'kernel32.lib',
        'ole32.lib',
        'shell32.lib',
        'user32.lib',
        'userenv.lib',
        'version.lib',
        'winmm.lib',
        'ws2_32.lib',
        'Shlwapi.lib',
    ])

  # we just build static libraries that GN needs
  executables['gn']['libs'].extend(static_libraries.keys())
  executables['gn_unittests']['libs'].extend(static_libraries.keys())

  WriteGenericNinja(path, static_libraries, executables, cc, cxx, ar, ld,
                    platform, host, options, cflags, cflags_cc, arflags,
                    ldflags, libflags, include_dirs, libs)


if __name__ == '__main__':
  sys.exit(main(sys.argv[1:]))

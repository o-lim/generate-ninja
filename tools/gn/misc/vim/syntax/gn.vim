" Copyright 2014 The Chromium Authors. All rights reserved.
" Use of this source code is governed by a BSD-style license that can be
" found in the LICENSE file.
"
" gn.vim: Vim syntax file for GN.
"
" Quit when a (custom) syntax file was already loaded
"if exists("b:current_syntax")
  "finish
"endif

syn case match

" Keywords within functions
syn keyword     gnConditional       if else
hi def link     gnConditional       Conditional

" Predefined variables
syn keyword     gnPredefVar console_pool current_cpu current_os
syn keyword     gnPredefVar current_toolchain default_toolchain host_cpu host_os
syn keyword     gnPredefVar root_build_dir root_gen_dir root_out_dir
syn keyword     gnPredefVar target_cpu target_gen_dir target_out_dir
syn keyword     gnPredefVar target_os
syn keyword     gnPredefVar true false
hi def link     gnPredefVar         Constant

" Target declarations
syn keyword     gnTarget action action_foreach copy executable group
syn keyword     gnTarget shared_library source_set static_library
syn keyword     gnTarget loadable_module generated_file
hi def link     gnTarget            Type

" Buildfile functions
syn keyword     gnFunctions assert assert_no_deps config declare_args defined
syn keyword     gnFunctions exec_script foreach forward_variables_from
syn keyword     gnFunctions get_label_info get_path_info
syn keyword     gnFunctions get_target_outputs getenv import print
syn keyword     gnFunctions mark_used mark_used_from not_needed
syn keyword     gnFunctions process_file_template read_file rebase_path
syn keyword     gnFunctions set_default_toolchain set_defaults
syn keyword     gnFunctions set_sources_assignment_filter target template tool
syn keyword     gnFunctions toolchain toolchain_args propagates_configs write_file
hi def link     gnFunctions         Macro

" Variables
syn keyword     gnVariable all_dependent_configs allow_circular_includes_from
syn keyword     gnVariable args arflags asmflags asmppflags bundle_contents_dir
syn keyword     gnVariable bundle_deps_filter bundle_executable_dir
syn keyword     gnVariable bundle_plugins_dir bundle_resources_dir
syn keyword     gnVariable bundle_root_dir cflags cflags_c cflags_cc cflags_objc
syn keyword     gnVariable cflags_objcc check_includes
syn keyword     gnVariable code_signing_args code_signing_outputs
syn keyword     gnVariable code_signing_script code_signing_sources
syn keyword     gnVariable command complete_static_lib configs
syn keyword     gnVariable cppflags cppflags_c cppflags_cc cppflags_objc
syn keyword     gnVariable cppflags_objcc data data_deps defines define_switch
syn keyword     gnVariable default_output_extension default_output_dir
syn keyword     gnVariable depend_output depfile deps depsformat description
syn keyword     gnVariable include_dirs include_switch inputs interpreter
syn keyword     gnVariable ldflags lib_dirs libs lib_dir_switch lib_switch
syn keyword     gnVariable link_output output_extension output_dir output_name
syn keyword     gnVariable output_prefix outputs pool precompiled_header
syn keyword     gnVariable precompiled_header_type precompiled_source
syn keyword     gnVariable restat runtime_link_output public public_configs
syn keyword     gnVariable public_deps rspfile rspfile_content script sources
syn keyword     gnVariable sys_include_dirs sys_include_switch testonly
syn keyword     gnVariable visibility contents output_conversion rebase
syn keyword     gnVariable data_keys walk_keys
hi def link     gnVariable          Keyword

" Strings
syn region      gnString start=+L\="+ skip=+\\\\\|\\"+ end=+"+ contains=@Spell,gnTargetName
syn match       gnTargetName '\v:\w+' contained
hi def link     gnString            String
hi def link     gnTargetName        Special

" Comments
syn keyword     gnTodo              contained TODO FIXME XXX BUG NOTE
syn cluster     gnCommentGroup      contains=gnTodo
syn region      gnComment           start="#" end="$" contains=@gnCommentGroup,@Spell

hi def link     gnComment           Comment
hi def link     gnTodo              Todo

" Operators; I think this is a bit too colourful.
"syn match gnOperator /=/
"syn match gnOperator /!=/
"syn match gnOperator />=/
"syn match gnOperator /<=/
"syn match gnOperator /==/
"syn match gnOperator /+=/
"syn match gnOperator /-=/
"syn match gnOperator /\s>\s/
"syn match gnOperator /\s<\s/
"syn match gnOperator /\s+\s/
"syn match gnOperator /\s-\s/
"hi def link     gnOperator          Operator

syn sync minlines=500

let b:current_syntax = "gn"

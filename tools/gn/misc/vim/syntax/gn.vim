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
syn keyword     gnPredefVar current_cpu current_os current_toolchain
syn keyword     gnPredefVar default_toolchain host_cpu host_os
syn keyword     gnPredefVar root_build_dir root_gen_dir root_out_dir
syn keyword     gnPredefVar target_cpu target_gen_dir target_out_dir
syn keyword     gnPredefVar target_os
syn keyword     gnPredefVar true false
hi def link     gnPredefVar         Constant

" Target declarations
syn keyword     gnTarget action action_foreach copy executable group
syn keyword     gnTarget shared_library source_set static_library
syn keyword     gnTarget loadable_module
hi def link     gnTarget            Type

" Buildfile functions
syn keyword     gnFunctions assert config declare_args defined exec_script
syn keyword     gnFunctions foreach forward_variables_from
syn keyword     gnFunctions get_label_info get_path_info
syn keyword     gnFunctions get_target_outputs getenv import print
syn keyword     gnFunctions mark_used mark_used_from
syn keyword     gnFunctions process_file_template read_file rebase_path
syn keyword     gnFunctions set_default_toolchain set_defaults
syn keyword     gnFunctions set_sources_assignment_filter target template tool
syn keyword     gnFunctions toolchain toolchain_args write_file
hi def link     gnFunctions         Macro

" Variables
syn keyword     gnVariable all_dependent_configs allow_circular_includes_from
syn keyword     gnVariable args asmflags asmppflags cflags cflags_c cflags_cc
syn keyword     gnVariable cflags_objc cflags_objcc check_includes console
syn keyword     gnVariable complete_static_lib cppflags cppflags_c cppflags_cc
syn keyword     gnVariable cppflags_objc cppflags_objcc command configs
syn keyword     gnVariable data data_deps defines define_switch
syn keyword     gnVariable default_output_extension default_output_dir
syn keyword     gnVariable depend_output depfile deps depsformat
syn keyword     gnVariable description include_dirs inputs include_switch
syn keyword     gnVariable interpreter ldflags lib_dirs libs output_extension
syn keyword     gnVariable output_dir output_name outputs link_output
syn keyword     gnVariable output_prefix lib_switch lib_dir_switch
syn keyword     gnVariable precompiled_header precompiled_header_type
syn keyword     gnVariable precompiled_source restat runtime_link_output
syn keyword     gnVariable public public_configs public_deps rspfile
syn keyword     gnVariable rspfile_content script sources sys_include_dirs
syn keyword     gnVariable sys_include_switch testonly visibility
hi def link     gnVariable          Keyword

" Strings
syn region      gnString start=+L\="+ skip=+\\\\\|\\"+ end=+"+ contains=@Spell
hi def link     gnString            String

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

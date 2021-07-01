// Copyright 2021 Aleksandar Radivojević
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// ensures name is not removed/renamed
#define check(x) using x
#define check_namespace(x) using namespace x;

// PUBLIC API //
#include <mtinfo/terminfo/parser.hh>

check(mtinfo::terminfo::parser::parse_compiled_terminfo_file);
check(mtinfo::terminfo::parser::parse_compiled_terminfo_from_directory);
check(mtinfo::terminfo::parser::parse_compiled_terminfo_from_env);
check(mtinfo::terminfo::parser::parse_compiled_terminfo);

#include <mtinfo/terminfo/terminfo.hh>

check(mtinfo::terminfo::Terminfo);

#include <mtinfo/terminfo/constants.hh>

check_namespace(mtinfo::terminfo::constants);
// TODO the stuff inside...

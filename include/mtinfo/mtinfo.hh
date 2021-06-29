// Copyright 2020-2021 Aleksandar Radivojević
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

#pragma once

// #include "mtinfo/terminfo.hh"

// #include "mtinfo/terminfo/terminfo.hh"
#include "mtinfo/terminfo/parser.hh"
#include "mtinfo/terminfo/terminfo.hh"

namespace mtinfo {
    using terminfo::Terminfo;
    using terminfo::parser::parse_compiled_terminfo_file;
    // TODO function that loads terminfo from folder
    // function that loads terminfo using environ TERMINFO
}

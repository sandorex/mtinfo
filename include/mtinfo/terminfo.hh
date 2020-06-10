// Copyright 2020 Aleksandar Radivojević
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

#include <vector>
#include <string>

#include "mtinfo/terminfo_constants.hh"

namespace mtinfo::terminfo {
    const int MAGIC_NUMBER_16 = 0432;
    // const int MAGIC_NUMBER_32 = 01036; // i havent encountered this yet

    // contains all the raw terminfo data
    struct Terminfo {
        std::array<int8_t, TERMINFO_BOOLEANS_LEN> bools;
        std::array<int16_t, TERMINFO_NUMBERS_LEN> numbers;
        std::array<int16_t, TERMINFO_STRINGS_LEN> string_offsets;
        std::array<std::string, TERMINFO_STRINGS_LEN> strings;
    };

    Terminfo parse_terminfo_file(const std::string_view&);
}

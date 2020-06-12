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

#include "mtinfo_export.h"

#include <vector>
#include <optional>
#include <string>

#include "mtinfo/terminfo_constants.hh"

namespace mtinfo::terminfo {
    const int MAGIC_NUMBER_16 = 0x011A; //0432;
    // const int MAGIC_NUMBER_32 = 01036; // i havent encountered this yet

    // contains all the raw terminfo data
    struct MTINFO_EXPORT Terminfo {
        std::vector<std::string> names;
        std::array<int8_t, TERMINFO_BOOLEANS_LEN> bools; // TODO default everything to -1
        std::array<int16_t, TERMINFO_NUMBERS_LEN> numbers;
        // std::array<int16_t, TERMINFO_STRINGS_LEN> string_offsets;
        std::array<std::optional<std::string>, TERMINFO_STRINGS_LEN> strings;

        Terminfo()
            : names()
            , bools()
            , numbers()
            , strings({}) {
            // fill everything with undefined value
            std::fill(bools.begin(), bools.end(), -1);
            std::fill(numbers.begin(), numbers.end(), -1);
        }
    };

    template <class BeginIt, class EndIt>
    MTINFO_EXPORT Terminfo parse_terminfo(BeginIt begin, EndIt end);
    MTINFO_EXPORT Terminfo parse_terminfo(const std::vector<uint8_t>& data);
    MTINFO_EXPORT Terminfo parse_terminfo_file(const std::string_view& path);
}

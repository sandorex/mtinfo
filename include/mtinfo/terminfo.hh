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

#include "mtinfo/terminfo_constants.hh"
#include "mtinfo_export.h"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace mtinfo
{
    // TODO MAKE EVERYTHING signed instead of unsigned cause why not

    // contains all the terminfo data
    struct MTINFO_EXPORT Terminfo {
        std::vector<std::string>                                        names;
        std::array<bool, internal::constants::TERMINFO_BOOLEANS_LENGTH> bools;
        std::array<std::optional<int16_t>,
                   internal::constants::TERMINFO_NUMBERS_LENGTH>
          numbers;
        std::array<std::optional<std::string>,
                   internal::constants::TERMINFO_STRINGS_LENGTH>
                                                strings;
        std::map<std::string_view, bool>        extended_bools;
        std::map<std::string_view, int16_t>     extended_numbers;
        std::map<std::string_view, std::string> extended_strings;

        Terminfo()
          : names()
          , bools()
          , numbers()
          , strings()
        {
            // fill everything with undefined value
            std::fill (bools.begin(), bools.end(), false);
            std::fill (numbers.begin(), numbers.end(), std::nullopt);
            std::fill (strings.begin(), strings.end(), std::nullopt);
        }
    };

    MTINFO_EXPORT Terminfo
    parse_terminfo (const std::vector<uint8_t>& data);

    MTINFO_EXPORT Terminfo
    parse_terminfo (const uint8_t* data, size_t length);

    MTINFO_EXPORT Terminfo
    parse_terminfo_file (const std::string_view& path);
} // namespace mtinfo

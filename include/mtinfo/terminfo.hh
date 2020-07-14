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

#include "mtinfo/export.hh"
#include "mtinfo/terminfo_constants.hh"

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace mtinfo
{
    // contains all the terminfo data
    class MTINFO_EXPORT Terminfo
    {
    public:
        std::vector<std::string> aliases;
        std::string              description;
        std::vector<bool>        bools;
        // TODO use uint8_t cause only the positive part is actually read
        std::vector<std::optional<uint16_t>>    numbers;
        std::vector<std::optional<std::string>> strings;
        std::map<std::string_view, bool>        extended_bools;
        std::map<std::string_view, uint16_t>    extended_numbers;
        std::map<std::string_view, std::string> extended_strings;

        Terminfo() = default;

        Terminfo (Terminfo&& terminfo)
        {
            this->aliases     = std::move (terminfo.aliases);
            this->description = std::move (terminfo.description);
            this->bools       = std::move (terminfo.bools);
            this->numbers     = std::move (terminfo.numbers);
            this->strings     = std::move (terminfo.strings);

            this->extended_bools   = std::move (terminfo.extended_bools);
            this->extended_numbers = std::move (terminfo.extended_numbers);
            this->extended_strings = std::move (terminfo.extended_strings);
        }

        Terminfo&
        operator= (Terminfo&& terminfo)
        {
            if (this == &terminfo)
                return *this;

            this->aliases     = std::move (terminfo.aliases);
            this->description = std::move (terminfo.description);
            this->bools       = std::move (terminfo.bools);
            this->numbers     = std::move (terminfo.numbers);
            this->strings     = std::move (terminfo.strings);

            this->extended_bools   = std::move (terminfo.extended_bools);
            this->extended_numbers = std::move (terminfo.extended_numbers);
            this->extended_strings = std::move (terminfo.extended_strings);

            return *this;
        }
    };

    MTINFO_EXPORT Terminfo
    parse_terminfo (const uint8_t* data, size_t length);

    MTINFO_EXPORT Terminfo
    parse_terminfo_file (const std::string_view& path);
} // namespace mtinfo

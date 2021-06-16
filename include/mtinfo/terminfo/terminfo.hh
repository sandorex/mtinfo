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

#include "mtinfo/export.hh"

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace mtinfo::terminfo
{
    // contains all the terminfo data
    class MTINFO_EXPORT Terminfo
    {
    public:
        std::optional<std::string>              name;
        std::vector<std::string>                aliases;
        std::vector<bool>                       bools;
        std::vector<std::optional<int32_t>>     numbers;
        std::vector<std::optional<std::string>> strings;
        std::map<std::string, bool>             extended_bools;
        std::map<std::string, int32_t>          extended_numbers;
        std::map<std::string, std::string>      extended_strings;
        bool                                    is_extended;
        bool                                    is_32bit;

        Terminfo()
        :   name(),
            aliases(),
            bools(),
            numbers(),
            strings(),
            extended_bools(),
            extended_numbers(),
            extended_strings(),
            is_extended(false),
            is_32bit(false)
        {}

        Terminfo (Terminfo&& terminfo)
        {
            this->name      = std::move (terminfo.name);
            this->aliases   = std::move (terminfo.aliases);
            this->bools     = std::move (terminfo.bools);
            this->numbers   = std::move (terminfo.numbers);
            this->strings   = std::move (terminfo.strings);

            this->extended_bools   = std::move (terminfo.extended_bools);
            this->extended_numbers = std::move (terminfo.extended_numbers);
            this->extended_strings = std::move (terminfo.extended_strings);

            this->is_extended   = terminfo.is_extended;
            this->is_32bit      = terminfo.is_32bit;
        }

        Terminfo&
        operator= (Terminfo&& terminfo)
        {
            if (this == &terminfo)
                return *this;

            this->name      = std::move (terminfo.name);
            this->aliases   = std::move (terminfo.aliases);
            this->bools     = std::move (terminfo.bools);
            this->numbers   = std::move (terminfo.numbers);
            this->strings   = std::move (terminfo.strings);

            this->extended_bools   = std::move (terminfo.extended_bools);
            this->extended_numbers = std::move (terminfo.extended_numbers);
            this->extended_strings = std::move (terminfo.extended_strings);

            this->is_extended   = terminfo.is_extended;
            this->is_32bit      = terminfo.is_32bit;

            return *this;
        }

        bool get_bool(size_t index, bool default_value = false) const {
            if (index >= this->bools.size())
                return default_value;

            return this->bools.at(index);
        }

        std::optional<int32_t> get_number(size_t index, std::optional<int32_t> default_value = {}) const {
            if (index >= this->numbers.size())
                return default_value;

            return this->numbers.at(index);
        }

        std::optional<std::string_view> get_string(size_t index, std::optional<std::string_view> default_value = {}) const {
            if (index >= this->strings.size())
                return default_value;

            auto item = this->strings.at(index);
            if (item.has_value())
                return std::string_view(item.value());
            else
                return default_value;
        }

        std::optional<bool> get_extended_bool(const std::string& prop_name, std::optional<bool> default_value = {}) const {
            auto it = this->extended_bools.find(prop_name);
            if (it != this->extended_bools.end())
                return it->second;

            return default_value;
        }

        std::optional<int32_t> get_extended_number(const std::string& prop_name, std::optional<int32_t> default_value = {}) const {
            auto it = this->extended_numbers.find(prop_name);
            if (it != this->extended_numbers.end())
                return it->second;

            return default_value;
        }

        std::optional<std::string_view> get_extended_string(const std::string& prop_name, std::optional<std::string_view> default_value = {}) const {
            auto it = this->extended_strings.find(prop_name);
            if (it != this->extended_strings.end())
                return std::string_view(it->second);

            return default_value;
        }
    };
} // namespace mtinfo

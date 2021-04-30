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

#pragma once

#include <cstdint>
#include <iterator>
#include <vector>
#include <string>

namespace mtinfo::terminfo::parser {
    template <typename T>
    inline char i8(T& iterator) {
        return *(iterator++);
    }

    template <typename T>
    inline int16_t i16(T& iterator) {
        const auto a = *iterator++;
        const auto b = *(iterator++);

        // little endian short
        return static_cast<int16_t> ((b << 8) | a);
    }

    template <typename T, typename K = char>
    bool is_out_of_bounds(T& begin, T end, size_t amount = 1) {
        return (std::distance(begin, end) - (sizeof(K) * amount)) <= 0;
    }

    // template <typename T>
    // class SafeIterator {
    // public:

    // };

    // splits string by a delimiter
    std::vector<std::string> split_string (std::string input, const std::string_view& delimiter);
}

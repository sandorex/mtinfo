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

#include "mtinfo/errors.hh"
#include "mtinfo/terminfo.hh"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

const int MAGIC_NUMBER = 0x11A;

// // converts two u8 to i16
// inline int16_t
// u8_to_i16_le (uint8_t a, uint8_t b)
// {
//     return static_cast<int16_t> ((b << 8) | a);
// }

// get a signed short using little endian byte order
inline int16_t
i16_le (uint8_t*& iter)
{
    const auto a = *iter++;
    const auto b = *iter++;

    return static_cast<int16_t> ((b << 8) | a);
}

// gets a vector of signed short using little endian byte order
std::vector<int16_t>
i16_le (uint8_t*& iter, size_t amount)
{
    std::vector<int16_t> buffer (amount);

    for (size_t i = 0; i < amount; i++)
        buffer[i] = i16_le (iter);

    return buffer;
}

// gets an std::array of signed shorts using little endian byte order
template <size_t Amount>
std::array<int16_t, Amount>
i16_le (uint8_t*& iter)
{
    std::array<int16_t, Amount> buffer;

    for (size_t i = 0; i < Amount; i++)
        buffer[i] = i16_le (iter);

    return buffer;
}

// splits string by delimiter
std::vector<std::string>
split (std::string input, const std::string_view& delimiter);

// parses header of terminfo
std::array<int16_t, 5>
parse_header_section (uint8_t*& begin, const uint8_t* end);

// parses name section of terminfo
std::vector<std::string>
parse_names (uint8_t*& begin, const uint8_t* end, size_t length);

// parses bool section of terminfo
std::vector<bool>
parse_bool_section (uint8_t*& begin, const uint8_t* end, size_t length);

// parses numbers section of terminfo
std::vector<std::optional<uint16_t>>
parse_numbers_section (uint8_t*& begin, const uint8_t* end, size_t length);

// parses string offsets section of terminfo
std::vector<bool>
parse_string_offsets_section (uint8_t*&      begin,
                              const uint8_t* end,
                              size_t         length_shorts);

// parses string table section of terminfo
std::vector<std::optional<std::string>>
parse_string_table (uint8_t*&                begin,
                    const uint8_t*           end,
                    const std::vector<bool>& offsets,
                    size_t                   length_bytes);

// parses ncurses extended terminfo
void
parse_extended_terminfo (
  uint8_t*&                                begin,
  const uint8_t*                           end,
  std::map<std::string_view, bool>&        extended_bools,
  std::map<std::string_view, uint16_t>&    extended_numbers,
  std::map<std::string_view, std::string>& extended_strings);

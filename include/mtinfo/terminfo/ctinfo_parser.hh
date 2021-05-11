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

#include "mtinfo/export.hh"
#include "mtinfo/terminfo/ctinfo_parser_util.hh"
#include "mtinfo/terminfo/terminfo.hh"
#include "mtinfo/errors.hh"

#include <array>
#include <iostream>
#include <iterator>
#include <optional>
#include <algorithm>

namespace mtinfo::terminfo::parser {
    const int MAGIC_NUMBER_16BIT = 0x11A;
    const int MAGIC_NUMBER_32BIT = 0x21E;

    struct Header {
        // magic number, should always be equal to 0x11A or 282 DEC
        int16_t magic_number;

        // size of names / description section in bytes
        int16_t names_size;

        // each bool is 1 byte
        int16_t bools_count;

        // each number is 2 bytes (short)
        int16_t numbers_count;

        // each offset is 2 bytes (short)
        int16_t offsets_count;

        // size of string table in bytes
        int16_t strings_table_size;
    };

    struct ExtendedHeader {
        // each bool is 1 byte
        int16_t bools_count;

        // each number is 2 bytes (short)
        int16_t numbers_count;

        // number of strings defined
        int16_t strings_count;

        // TODO i think this is actually total number of strings, both property
        // names and string values
        //
        // number of names of properties
        int16_t property_name_count;

        // last offset which is value of a string not name of a property
        int16_t last_string_offset;
    };

    MTINFO_EXPORT Terminfo
    parse_compiled_terminfo_file (const std::string_view& path);

    // parses terminfo header
    Header parse_header_section (ByteIterator& iter);

    ExtendedHeader parse_extended_header_section (ByteIterator& iter);

    std::vector<std::string> parse_names (ByteIterator& iter, size_t length);

    std::vector<bool> parse_bool_section (ByteIterator& iter, size_t length);

    std::vector<std::optional<int32_t>> parse_numbers_section (ByteIterator& iter, size_t length);

    std::vector<std::optional<std::string>> parse_string_table (ByteIterator& iter, size_t offsets_length, size_t string_table_length);

    template <typename T>
    std::vector<std::string> parse_extended_string_table (T& begin, T end, size_t strings_count, size_t last_string_offset) {
        /* TODO */

        if (is_out_of_bounds(begin, end, last_string_offset))
            throw error::EOFError ("extended string table");

        std::vector<std::string> string_table (strings_count);
        std::string string_table_raw (begin, begin + last_string_offset);

        size_t position = 0;
        for (size_t i = 0; i < strings_count; ++i) {
            const size_t null_ch_pos = string_table_raw.find_first_of('\0', position);
            if (null_ch_pos == std::string::npos)
                throw mtinfo::error::ParsingError("String " + std::to_string(i) + " is out of bounds, this probably"
                "means that one of the strings is not terminated properly");

            string_table.emplace_back(string_table_raw, position, null_ch_pos);
            position = null_ch_pos;

            // string_table.push_back(std::string(string_table_raw.begin()));
        }
        // std::vector<std::string> property_name_table (strings_count);
        // std::string str_table_raw (begin, begin + length);
        // begin += length;

        // for (size_t i = 0; i < offsets.size(); ++i) {
        //     if (!offsets[i].has_value())
        //         string_table[i] = {};

        //     if (static_cast<size_t>(offsets[i].value()) > length)
        //         throw mtinfo::error::ParsingError("Offset " + std::to_string(i) + " is out of bounds, this probably"
        //         "means that one of the strings is not terminated properly");

        //     string_table[i] = std::string(str_table_raw, offsets[i].value());
        // }

        return string_table;
    }

    Terminfo parse_compiled_terminfo(ByteIterator iter, bool parse_extended = true);
}

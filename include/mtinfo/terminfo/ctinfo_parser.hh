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

// temp
#include <cstdio>

namespace mtinfo::terminfo::parser {
    const int MAGIC_NUMBER = 0x11A;

    struct Header {
        // magic number, should always be equal to 0x11A or 282 DEC
        int magic_number;

        // size of names / description section in bytes
        int names_size;

        // each bool is 1 byte
        int bools_count;

        // each number is 2 bytes (short)
        int numbers_count;

        // each offset is 2 bytes (short)
        int offsets_count;

        // size of string table in bytes
        int strings_table_size;
    };

    struct ExtendedHeader {
        // each bool is 1 byte
        int bools_count;

        // each number is 2 bytes (short)
        int numbers_count;

        // number of strings defined
        int strings_count;

        // number of names of properties
        int property_name_count;

        // last offset which is value of a string not name of a property
        int last_string_offset;
    };

    MTINFO_EXPORT Terminfo
    parse_compiled_terminfo_file (const std::string_view& path);

    // parses terminfo header
    template <typename T>
    Header parse_header_section (T& begin, T end) {
        /*
            terminfo header consists of 6 shorts, for more detail go to Header
            struct
        */
        if (is_out_of_bounds<T, int16_t>(begin, end, 5))
            throw error::EOFError ("the header");

        Header header;
        header.magic_number = i16(begin);
        header.names_size = i16(begin);
        header.bools_count = i16(begin);
        header.numbers_count = i16(begin);
        header.offsets_count = i16(begin);
        header.strings_table_size = i16(begin);

        return header;
    }

    // parses terminfo extended header
    template <typename T>
    ExtendedHeader parse_extended_header_section (T& begin, T end) {
        /*
            terminfo header consists of 5 shorts, for more detail go to ExtendedHeader
            struct
        */
        if (is_out_of_bounds<T, int16_t>(begin, end, 5))
            throw error::EOFError ("the header");

        ExtendedHeader header;
        header.bools_count = i16(begin);
        header.numbers_count = i16(begin);
        header.strings_count = i16(begin);
        header.property_name_count = i16(begin);
        header.last_string_offset = i16(begin);

        return header;
    }

    // parses names / description section
    template <typename T>
    std::pair<std::vector<std::string>, std::optional<std::string>> parse_names (T& begin, T end, size_t length) {
        /*
            names are delimited by character '|' and end with null char, last segment
            is the description

            originally the maximum size was 128 bytes but i see no reason to follow
            such a limitation
        */

        if (is_out_of_bounds<T, char>(begin, end, length))
            throw mtinfo::error::EOFError("the names section");

        std::vector names = split_string(std::string(begin, begin + length), "|");
        begin += length;

        std::optional<std::string> desc = {};

        if (names.size() > 1) {
            desc = names.back();
            names.pop_back();
        }

        return {names, desc};
    }

    // parses bool section
    template <typename T>
    std::vector<bool> parse_bool_section (T& begin, T end, size_t length) {
        /*
            bools are a single byte signed integers

            originally 0 means it's missing and 1 means it has that capability

            im implementing it so <= 0 means it's missing, > 0 means it has the capability
        */

        if (is_out_of_bounds<T, bool>(begin, end, length))
            throw mtinfo::error::EOFError ("the bool section");

        std::vector<bool> buffer (length);

        for (size_t i = 0; i < length; i++)
            buffer[i] = *(begin++) > 0;

        return buffer;
    }

    // parses numbers section
    template <typename T>
    std::vector<std::optional<int16_t>> parse_numbers_section (T& begin, T end, size_t length) {
        /*
            numbers are signed shorts, any negative number is invalid, -1 means it's
            undefined but im not gonna bother
        */

        if (is_out_of_bounds<T, int16_t>(begin, end, length))
            throw mtinfo::error::EOFError ("the numbers section");

        std::vector<std::optional<int16_t>> numbers(length);
        std::generate_n(numbers.begin(), length, [&]() -> std::optional<int16_t> {
            const auto value = i16(begin);

            if (value >= 0)
                return value;

            return {};
        });

        return numbers;
    }

    // parses string table offsets section
    template <typename T>
    std::vector<std::optional<int16_t>> parse_string_offsets_section (T& begin, T end, size_t length) {
        /* TODO */
        // offsets section contains short integers (2 bytes) that specify where a
        // string starts relative to start of the string table
        // negative offsets are invalid and -1 means the string isnt defined
        //
        // i've changed it to be a bool, it signifies if the string at the index
        // is defined or not

        if (is_out_of_bounds<T, int16_t>(begin, end, length))
            throw mtinfo::error::EOFError ("the string offsets section");

        std::vector<std::optional<int16_t>> offsets (length);
        std::generate_n(offsets.begin(), length, [&]() -> std::optional<int16_t> {
            const auto value = i16(begin);

            if (value < 0)
                return {};

            return value;
        });

        return offsets;
    }

    template <typename T>
    std::vector<std::optional<std::string>> parse_string_table (T& begin, T end, const std::vector<std::optional<int16_t>>& offsets, size_t length) {
        /* TODO */
        // strings are gathered by using offsets from previous section that specify
        // if the string exists, and if it does where does it start relative to
        // start of the strings table
        //
        // offsets are not offsets anymore but simply signify if the string at the
        // index exists at all

        if (is_out_of_bounds<T, int8_t>(begin, end, length))
            throw mtinfo::error::EOFError ("the string table");

        std::vector<std::optional<std::string>> string_table (offsets.size());
        std::string str_table_raw (begin, begin + length);
        begin += length;

        for (size_t i = 0; i < offsets.size(); ++i) {
            if (!offsets[i].has_value())
                string_table[i] = {};

            if (static_cast<size_t>(offsets[i].value()) > length)
                throw mtinfo::error::ParsingError("Offset " + std::to_string(i) + " is out of bounds, this probably"
                "means that one of the strings is not terminated properly");

            string_table[i] = std::string(str_table_raw, offsets[i].value());
        }

        return string_table;
    }

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

    template <typename T>
    Terminfo parse_compiled_terminfo(T _begin, T end, bool parse_extended = true) {
        Terminfo terminfo;
        auto begin = _begin;

        terminfo.is_extended = false;




        const Header header = parse_header_section(begin, end);

        if (header.magic_number != MAGIC_NUMBER)
            throw mtinfo::error::Error("Invalid magic number"); // TODO should it continue?

        {
            const auto names_pair = parse_names(begin, end, header.names_size);
            terminfo.aliases = names_pair.first;
            terminfo.description = names_pair.second;
        }

        terminfo.bools = parse_bool_section(begin, end, header.bools_count);

        //
        // TODO THESE FUCKING PADDINGS ARE BREAKING EVERYTHING MAKE A FUNCTION FOR IT
        // ----------------------------------------------------------------------------
        // padding byte so numbers start on an even address
        // don't ask me why
        if (std::distance(begin, end) % 2 > 0)
            i8(begin);

        terminfo.numbers = parse_numbers_section(begin, end, header.numbers_count);

        {
            const auto offsets = parse_string_offsets_section(begin, end, header.offsets_count);
            terminfo.strings = parse_string_table(begin, end, offsets, header.strings_table_size);
        }

        // parse extended if this is not eof
        if (parse_extended && !is_out_of_bounds(begin, end)) {
            terminfo.is_extended = true;

            // TODO is this padding always present or only in case of uneven address??
            // begin += 1;
            if (std::distance(begin, end) % 2 > 0)
                i8(begin);

            const ExtendedHeader ext_header = parse_extended_header_section(begin, end);

            std::vector<bool> bools;
            if (ext_header.bools_count > 0)
                bools = parse_bool_section(begin, end, ext_header.bools_count);

            // padding byte so numbers start on an even address
            if (std::distance(begin, end) % 2 == 0)
                i8(begin);

            std::vector<std::optional<int16_t>> numbers;
            if (ext_header.numbers_count > 0)
                numbers = parse_numbers_section(begin, end, ext_header.numbers_count);

            // const auto offsets = parse_string_offsets_section(begin, end, ext_header.);
            // const auto strings = parse_string_table(begin, end, offsets, header.strings_table_size);

            terminfo.numbers = numbers;
        }

        return terminfo;
    }
}

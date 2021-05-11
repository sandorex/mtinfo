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

#include <mtinfo/errors.hh>
#include <mtinfo/terminfo/ctinfo_parser_util.hh>
#include "mtinfo/terminfo/ctinfo_parser.hh"
#include "mtinfo/terminfo/terminfo.hh"

#include <string>
#include <fstream>
#include <algorithm>

namespace mtinfo::terminfo::parser {
    std::vector<std::string> split_string (std::string input, const std::string_view& delimiter) {
        std::vector<std::string> buffer;
        size_t pos = 0;
        while ((pos = input.find (delimiter)) != std::string::npos) {
            buffer.push_back (input.substr (0, pos));
            input.erase (0, pos + delimiter.length());
        }

        // the leftovers
        buffer.push_back (input);

        return buffer;
    }

    Terminfo parse_compiled_terminfo_file (const std::string_view& path) {
        // TODO limit file size somehow?
        std::ifstream file (path.data(), std::ios::binary);
        if (!file.good())
            throw mtinfo::error::Error ("Error reading file '" + std::string (path) + "'");

        std::vector<int8_t> buffer { std::istreambuf_iterator<char> (file),
                                     std::istreambuf_iterator<char>() };

        return parse_compiled_terminfo (ByteIterator(buffer.data(), buffer.size()));
    }

    Header parse_header_section (ByteIterator& iter) {
        /*
            terminfo header consists of 6 shorts, for more detail go to Header
            struct
        */
        if (iter.is_out_of_bounds<int16_t>(6))
            throw error::EOFError ("the header");

        Header header;
        header.magic_number = iter.i16();
        header.names_size = iter.i16();
        header.bools_count = iter.i16();
        header.numbers_count = iter.i16();
        header.offsets_count = iter.i16();
        header.strings_table_size = iter.i16();

        return header;
    }

    ExtendedHeader parse_extended_header_section (ByteIterator& iter) {
        /*
            terminfo header consists of 5 shorts, for more detail go to ExtendedHeader
            struct
        */
        if (iter.is_out_of_bounds<int16_t>(5))
            throw error::EOFError ("the extended header");

        ExtendedHeader header;
        header.bools_count = iter.i16();
        header.numbers_count = iter.i16();
        header.strings_count = iter.i16();
        header.property_name_count = iter.i16();
        header.last_string_offset = iter.i16();

        return header;
    }

    std::vector<std::string> parse_names (ByteIterator& iter, size_t length) {
        /*
            names are delimited by character '|' and end with null char, last segment
            is the description

            originally the maximum size was 128 bytes but i see no reason to follow
            such a limitation
        */

        if (iter.is_out_of_bounds(length))
            throw mtinfo::error::EOFError("the names section");

        // -1 is for the \0 character
        std::vector names = split_string(std::string(&iter, &(iter + length - 1)), "|");
        iter += length;

        return names;
    }

    std::vector<bool> parse_bool_section (ByteIterator& iter, size_t length) {
        /*
            bools are a single byte signed integers

            originally 0 means it's missing and 1 means it has that capability

            im implementing it so <= 0 means it's missing, > 0 means it has the capability
        */

        if (iter.is_out_of_bounds(length))
            throw mtinfo::error::EOFError ("the bool section");

        std::vector<bool> buffer (length);

        for (size_t i = 0; i < length; i++)
            buffer[i] = *(iter++) > 0;

        return buffer;
    }

    std::vector<std::optional<int32_t>> parse_numbers_section (ByteIterator& iter, size_t length) {
        /*
            numbers are signed shorts, any negative number is invalid, -1 means it's
            undefined but im not gonna bother
        */
        // TODO 32bit mode
        if (iter.is_out_of_bounds<int16_t>(length))
            throw mtinfo::error::EOFError ("the numbers section");

        std::vector<std::optional<int32_t>> numbers(length);
        std::generate_n(numbers.begin(), length, [&]() -> std::optional<int32_t> {
            const auto value = iter.i16();

            if (value >= 0)
                return value;

            return {};
        });

        return numbers;
    }

    std::vector<std::optional<std::string>> parse_string_table (ByteIterator& iter, size_t offsets_length, size_t string_table_length) {
        /* TODO */
        // offsets section contains short integers (2 bytes) that specify where a
        // string starts relative to start of the string table
        // negative offsets are invalid and -1 means the string isnt defined
        //
        // i've changed it to be a bool, it signifies if the string at the index
        // is defined or not

        if (iter.is_out_of_bounds<int16_t>(offsets_length))
            throw mtinfo::error::EOFError ("the string offsets section");

        std::vector<std::optional<int16_t>> offsets (offsets_length);
        std::generate_n(offsets.begin(), offsets_length, [&]() -> std::optional<int16_t> {
            const auto value = iter.i16();

            if (value < 0)
                return {};

            return value;
        });

        // strings are gathered by using offsets from previous section that specify
        // if the string exists, and if it does where does it start relative to
        // start of the strings table
        //
        // offsets are not offsets anymore but simply signify if the string at the
        // index exists at all

        if (iter.is_out_of_bounds(string_table_length))
            throw mtinfo::error::EOFError ("the string table");

        std::vector<std::optional<std::string>> string_table (offsets.size());
        std::string str_table_raw (&iter, &(iter + string_table_length - 1)); // -1 is for the \0 character
        iter += string_table_length;

        for (size_t i = 0; i < offsets.size(); ++i) {
            if (!offsets[i].has_value())
                string_table[i] = {};

            if (static_cast<size_t>(offsets[i].value()) > string_table_length)
                throw mtinfo::error::ParsingError("Offset " + std::to_string(i) + " is out of bounds, this probably"
                "means that one of the strings is not terminated properly");

            string_table[i] = std::string(str_table_raw, offsets[i].value());
        }

        return string_table;
    }

    Terminfo parse_compiled_terminfo(ByteIterator iter, bool parse_extended) {
        Terminfo terminfo;
        bool _32bit_mode = false; // TODO add mode to terminfo?

        const Header header = parse_header_section(iter);

        if (header.magic_number == MAGIC_NUMBER_32BIT)
            _32bit_mode = true;
        else if (header.magic_number == MAGIC_NUMBER_16BIT)
            _32bit_mode = false;
        else
            error::Error("Invalid magic number"); // TODO add it to the error message

        // TODO check if header is proper, all values above zero

        // TODO figure out description, also check if names are valid
        terminfo.aliases = parse_names(iter, header.names_size);

        terminfo.bools = parse_bool_section(iter, header.bools_count);

        iter.align_short_boundary();

        terminfo.numbers = parse_numbers_section(iter, header.numbers_count);

        terminfo.strings = parse_string_table(iter, header.offsets_count, header.strings_table_size);

        // parse extended if this is not eof
        if (parse_extended && !iter.is_out_of_bounds(1)) {
            terminfo.is_extended = true;

        //     const ExtendedHeader ext_header = parse_extended_header_section(begin, end);

        //     std::vector<bool> bools;
        //     if (ext_header.bools_count > 0)
        //         bools = parse_bool_section(begin, end, ext_header.bools_count);

        //     align_short_boundary(begin, end);

        //     std::vector<std::optional<int16_t>> numbers;
        //     if (ext_header.numbers_count > 0)
        //         numbers = parse_numbers_section(begin, end, ext_header.numbers_count);

        //     std::vector<std::string> strings;
        //     if (ext_header.strings_count > 0)
        //         strings = parse_extended_string_table(begin, end, ext_header.strings_count, ext_header.last_string_offset);

        //     // // const auto offsets = parse_string_offsets_section(begin, end, ext_header.);
        //     // // const auto strings = parse_string_table(begin, end, offsets, header.strings_table_size);

        //     // terminfo.numbers = numbers;
        //     // (void)0;
        }

        return terminfo;
    }
}

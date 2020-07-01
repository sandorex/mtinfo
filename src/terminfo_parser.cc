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

#include "terminfo_parser.hh"

#include "mtinfo/errors.hh"
#include "mtinfo/terminfo.hh"

#include <fstream>
#include <iostream>
#include <iterator>

namespace mtinfo
{
    Terminfo
    parse_terminfo (int8_t* begin, size_t length)
    {
        // TODO save some debug info and the section sizes in the terminfo struct

        using namespace mtinfo::internal::constants;

        mtinfo::Terminfo terminfo;

        const auto end = begin + length;

        if (begin >= end)
            throw mtinfo::ErrorEOF ("the magic number");

        // header must start with the magic number (don't ask just believe)
        if (i16_le (begin) != MAGIC_NUMBER)
            throw mtinfo::ErrorParsing ("invalid magic number");

        {
            const auto header = ::parse_header_section (begin, end);

            const int16_t names_size              = header[0];
            const int16_t bools_size              = header[1];
            const int16_t nums_size_shorts        = header[2];
            const int16_t str_offsets_size_shorts = header[3];
            const int16_t str_table_size          = header[4];

            // read the names
            if (names_size > 0)
                terminfo.names = ::parse_names (begin, end, names_size);

            // -- bools --
            if (bools_size > 0) {
                auto bools_raw = ::parse_bool_section (begin, end, bools_size);

                // ensure size doesnt exceed number of defined booleans
                if (bools_raw.size() > TERMINFO_BOOLEANS_LENGTH)
                    bools_raw.resize (TERMINFO_BOOLEANS_LENGTH);

                std::copy (bools_raw.cbegin(),
                           bools_raw.cend(),
                           terminfo.bools.begin());
            }

            // for legacy reasons there's a padding byte if numbers start on a
            // uneven address
            if ((names_size + bools_size + 1) % 2 != 1)
                ++begin;

            // -- numbers --
            if (nums_size_shorts > 0) {
                auto numbers_raw
                  = ::parse_numbers_section (begin, end, nums_size_shorts);

                // ensure size doesnt exceed number of defined numbers
                if (numbers_raw.size() > TERMINFO_NUMBERS_LENGTH)
                    numbers_raw.resize (TERMINFO_NUMBERS_LENGTH);

                std::copy (numbers_raw.cbegin(),
                           numbers_raw.cend(),
                           terminfo.numbers.begin());
            }

            // -- strings --
            if (str_offsets_size_shorts > 0) {
                auto strings_offsets
                  = ::parse_string_offsets_section (begin,
                                                    end,
                                                    str_offsets_size_shorts);

                // ensure size doesnt exceed number of defined strings
                if (strings_offsets.size() > TERMINFO_STRINGS_LENGTH)
                    strings_offsets.resize (TERMINFO_STRINGS_LENGTH);

                const auto string_table = ::parse_string_table (begin,
                                                                end,
                                                                strings_offsets,
                                                                str_table_size);

                // only write if there's anything to write
                if (string_table.size() > 0)
                    std::copy (string_table.cbegin(),
                               string_table.cend(),
                               terminfo.strings.begin());
            }
        }

        // TODO mention in the exception that its parsing the extended info

        // parse extended ncurses table
        // if (begin < end) {
        //     const auto extended_header = ::parse_header_section (begin, end);

        //     const int16_t bools_size              = extended_header[0];
        //     const int16_t nums_size_shorts        = extended_header[1];
        //     const int16_t str_offsets_size_shorts = extended_header[2];

        //     // the order of last two is confusing
        //     const int16_t str_table_size  = extended_header[3];
        //     const int16_t last_str_offset = extended_header[4];

        //     const auto bools = ::parse_bool_section (begin, end, bools_size);

        //     // for legacy reasons there's a padding byte if numbers start on a
        //     // uneven address
        //     if ((bools_size + 1) % 2 != 1)
        //         ++begin;

        //     const auto numbers
        //       = ::parse_numbers_section (begin, end, nums_size_shorts);

        //     const auto str_offsets
        //       = ::parse_string_offsets_section (begin,
        //                                         end,
        //                                         str_offsets_size_shorts);

        int _ = 0;
        //     // // is this needed ???
        //     // if (str_offsets.back() != last_str_table_offset_bytes)
        //     //     throw std::runtime_error (
        //     //       "last string table offset does not match between the extended "
        //     //       "terminfo header and string offsets section");

        //     // const auto string_table
        //     //   = ::parse_string_table (begin, end, str_offsets, str_table_size);
        // }

        return terminfo;
    }

    Terminfo
    parse_terminfo_file (const std::string_view& path)
    {
        std::ifstream file (path.data(), std::ios::binary);
        if (!file.good())
            throw Error ("File '" + std::string (path) + "' does not exist");

        std::vector<int8_t> buffer { std::istreambuf_iterator<char> (file),
                                     std::istreambuf_iterator<char>() };

        return parse_terminfo (buffer.data(), buffer.size());
    }
} // namespace mtinfo

std::vector<std::string>
split (std::string input, const std::string_view& delimiter)
{
    std::vector<std::string> buffer;
    size_t                   pos = 0;
    while ((pos = input.find (delimiter)) != std::string::npos) {
        buffer.push_back (input.substr (0, pos));
        input.erase (0, pos + delimiter.length());
    }

    // the leftovers
    buffer.push_back (input);

    return buffer;
}

std::array<int16_t, 5>
parse_header_section (int8_t*& begin, const int8_t* end)
{
    if (begin + 5 > end)
        throw mtinfo::ErrorEOF ("the header");

    return i16_le<5> (begin);
}

std::vector<std::string>
parse_names (int8_t*& begin, const int8_t* end, size_t length)
{
    // names are delimited by char '|' and ended with null char ('\0'),
    //
    // originally maximum size was 128 bytes but i do not see a reason to follow
    // such limitation

    if (begin + length > end)
        throw mtinfo::ErrorEOF ("the names");

    const std::vector names = split (std::string (begin, begin + length), "|");
    begin += length;

    return names;
}

std::vector<bool>
parse_bool_section (int8_t*& begin, const int8_t* end, size_t length)
{
    // bools are 1 byte signed integers where originally
    //      0 means that the capability is missing
    //      1 means that the capabiliy is found
    //
    // i am simplifying this so
    //      <= 0 means that the capability is missing
    //      > 0 means that the capability is found

    if (begin + length > end)
        throw mtinfo::ErrorEOF ("the bool section");

    std::vector<bool> buffer (length);

    for (size_t i = 0; i < length; i++) {
        const auto value = *begin++;
        buffer[i]        = value > 0;
        std::cout << "i: " << i << ", val: " << value << '\n';
    }
    // buffer[i] = *begin++ > 0;

    return buffer;
}

std::vector<std::optional<uint16_t>>
parse_numbers_section (int8_t*& begin, const int8_t* end, size_t length)
{
    // numbers are signed shorts (2 bytes), any negative number is invalid
    // except -1 which means the number is not defined
    //
    // im gonna simplify it so that any number < 0 means that it's undefined

    if (begin + (length * 2) > end)
        throw mtinfo::ErrorEOF ("the numbers section");

    std::vector<std::optional<uint16_t>> numbers (length);
    std::vector<int16_t>                 numbers_raw = ::i16_le (begin, length);

    for (size_t i = 0; i < length; ++i) {
        if (numbers_raw[i] < 0)
            numbers[i] = {};
        else
            numbers[i] = static_cast<unsigned> (numbers_raw[i]);
    }

    return numbers;
}

std::vector<std::optional<uint16_t>>
parse_string_offsets_section (int8_t*&      begin,
                              const int8_t* end,
                              size_t        length_shorts)
{
    // offsets section contains short integers (2 bytes) that specify where a
    // string starts relative to start of the string table
    // negative offsets are invalid and -1 means the string isnt defined
    //
    // I am simplifying this as everything else so an offset is undefined if
    // it's < 0

    if (begin + (length_shorts * 2) > end)
        throw mtinfo::ErrorEOF ("the string offsets section");

    std::vector<std::optional<uint16_t>> offsets (length_shorts);
    std::vector<int16_t> offsets_raw = ::i16_le (begin, length_shorts);

    for (size_t i = 0; i < length_shorts; ++i) {
        if (offsets_raw[i] < 0)
            offsets[i] = {};
        else
            offsets[i] = static_cast<unsigned> (offsets_raw[i]);
    }

    return offsets;
}

std::vector<std::optional<std::string>>
parse_string_table (int8_t*&                                    begin,
                    const int8_t*                               end,
                    const std::vector<std::optional<uint16_t>>& offsets,
                    size_t                                      length_bytes)
{
    if (begin + length_bytes > end)
        throw mtinfo::ErrorEOF ("the string table");

    std::vector<std::optional<std::string>> string_table (offsets.size());
    const std::string strings_table_raw (begin, begin + length_bytes);
    begin += length_bytes;

    for (size_t i = 0; i < offsets.size(); ++i) {
        if (!offsets[i].has_value()) {
            string_table[i] = {};
            continue;
        }

        if (*offsets[i] > length_bytes)
            throw mtinfo::ErrorParsing (
              "string offset is larger than strings table");

        // std::string constructor finds the first \0 character
        string_table[i] = std::string (strings_table_raw.data() + *offsets[i]);
    }

    return string_table;
}

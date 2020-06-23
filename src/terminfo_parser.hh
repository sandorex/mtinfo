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

// get a signed short using little endian byte order
template <class T>
inline int16_t
i16_le (T& iter)
{
    const auto a = *iter++;
    const auto b = *iter++;

    return (b << 8) | a;
}

// gets a vector of signed short using little endian byte order
template <class T>
std::vector<int16_t>
i16_le (T& iter, size_t amount)
{
    std::vector<int16_t> buffer (amount);

    for (size_t i = 0; i < amount; i++)
        buffer[i] = i16_le (iter);

    return buffer;
}

// gets an std::array of signed shorts using little endian byte order
template <size_t Amount, class T>
std::array<int16_t, Amount>
i16_le (T& iter)
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
template <class BeginIt, class EndIt>
std::array<int16_t, 5>
parse_header_section (BeginIt& begin, EndIt& end)
{
    if (begin + 5 > end)
        throw mtinfo::ErrorEOF ("the header");

    return i16_le<5> (begin);
}

// parses name section of terminfo
template <class BeginIt, class EndIt>
std::vector<std::string>
parse_names (BeginIt& begin, EndIt& end, size_t length)
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

// parses bool section of terminfo
template <class BeginIt, class EndIt>
std::vector<bool>
parse_bool_section (BeginIt& begin, EndIt& end, size_t length)
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

    for (size_t i = 0; i < length; i++)
        buffer[i] = *begin++ > 0;

    return buffer;
}

// parses numbers section of terminfo
template <class BeginIt, class EndIt>
std::vector<std::optional<uint16_t>>
parse_numbers_section (BeginIt& begin, EndIt& end, size_t length)
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

// parses string offsets section of terminfo
template <class BeginIt, class EndIt>
std::vector<std::optional<uint16_t>>
parse_string_offsets_section (BeginIt& begin, EndIt& end, size_t length_shorts)
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

#include <iostream> // temp

// parses string table section of terminfo
template <class BeginIt, class EndIt>
std::vector<std::optional<std::string>>
parse_string_table (BeginIt&                                    begin,
                    EndIt&                                      end,
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

template <class BeginIt, class EndIt>
mtinfo::Terminfo
parse_terminfo (BeginIt begin, EndIt end)
{
    using namespace mtinfo::internal::constants;

    mtinfo::Terminfo terminfo;

    if (begin >= end)
        throw mtinfo::ErrorEOF ("the magic number");

    // header must start with the magic number (don't ask just believe)
    if (i16_le (begin) != MAGIC_NUMBER)
        throw mtinfo::ErrorParsing ("invalid magic number");

    {
        // TODO put this information somewhere appropriate
        // the format begins with a header that contains the magic number and
        // sizes of each section
        // all 6 integers in the header are shorts (2 bytes) in little endian
        //
        // 1. the magic number (0x11A)
        // 2. size of names section in bytes
        // 3. size of boolean section in bytes
        // 4. size of numbers section in shorts
        // 5. size of offsets section in shorts
        // 6. size of the string table in bytes

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

            // ensure no undefined bools are written
            if (bools_raw.size() > TERMINFO_BOOLEANS_LENGTH)
                bools_raw.resize (TERMINFO_BOOLEANS_LENGTH);

            std::copy (bools_raw.cbegin(),
                       bools_raw.cend(),
                       terminfo.bools.begin());
        }

        // for legacy reasons there's a padding byte if numbers start on a
        // uneven address
        if ((names_size + bools_size) % 2 == 1)
            ++begin;

        // -- numbers --
        if (nums_size_shorts > 0) {
            auto numbers_raw
              = ::parse_numbers_section (begin, end, nums_size_shorts);

            // ensure no undefined numbers are written
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

            // ensure no undefined bools are written
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

    // // parse extended ncurses table
    // if (begin < end) {
    //     const auto extended_header = ::parse_header_section (begin, end);

    //     const int16_t num_of_ext_bools   = extended_header[0];
    //     const int16_t num_of_ext_nums    = extended_header[1];
    //     const int16_t num_of_ext_strings = extended_header[2];
    //     const int16_t string_table_size  = extended_header[3];
    //     const int16_t last_string_offset = extended_header[4];

    //     const auto bools
    //       = ::parse_bool_section (begin, end, bools_size);
    //     const auto numbers
    //       = ::parse_numbers_section (begin, end, nums_size_shorts);
    //     const auto str_offsets
    //       = ::parse_string_offsets_section (begin,
    //                                         end,
    //                                         str_offsets_size_shorts);

    //     // is this needed ???
    //     if (str_offsets.back() != last_str_table_offset_bytes)
    //         throw std::runtime_error (
    //           "last string table offset does not match between the extended "
    //           "terminfo header and string offsets section");

    //     const auto string_table
    //       = ::parse_string_table (begin,
    //                               end,
    //                               str_offsets,
    //                               str_table_size);
    // }

    return terminfo;
}

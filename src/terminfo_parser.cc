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

#include <execution>
#include <fstream>
#include <iostream>
#include <iterator>

namespace mtinfo
{
    Terminfo
    parse_terminfo (uint8_t* begin, size_t length)
    {
        using namespace mtinfo::internal::constants;

        mtinfo::Terminfo terminfo;

        const auto end = begin + length;

        if (begin + 2 > end)
            throw mtinfo::ErrorEOF ("the magic number");

        // header must start with the magic number (don't ask just believe)
        if (i16_le (begin) != MAGIC_NUMBER)
            // TODO add the magic number to error
            throw mtinfo::ErrorParsing ("invalid magic number");

        {
            const auto header = ::parse_header_section (begin, end);

            const int16_t names_size              = header[0];
            const int16_t bools_size              = header[1];
            const int16_t nums_size_shorts        = header[2];
            const int16_t str_offsets_size_shorts = header[3];
            const int16_t str_table_size          = header[4];

            int _aa = 0;

            if (names_size <= 0)
                throw Error ("size of names section is zero or less");

            // -- aliases & description --
            {
                auto names = ::parse_names (begin, end, names_size);

                if (names.size() == 0)
                    throw Error ("no name has been parsed");

                if (names.size() > 1) {
                    // last one should always be the description
                    terminfo.description
                      = *std::make_move_iterator (names.rbegin());
                    names.pop_back();

                    terminfo.aliases
                      = std::vector (std::make_move_iterator (names.begin()),
                                     std::make_move_iterator (names.end()));
                } else
                    terminfo.aliases = std::move (names);
            }

            // -- bools --
            if (bools_size > 0) {
                auto bools     = ::parse_bool_section (begin, end, bools_size);
                terminfo.bools = std::move (bools);
            }

            // for legacy reasons there's a padding byte if numbers start on a
            // uneven address
            if ((names_size + bools_size + 1) % 2 != 1)
                ++begin;

            // -- numbers --
            if (nums_size_shorts > 0) {
                auto numbers
                  = ::parse_numbers_section (begin, end, nums_size_shorts);

                terminfo.numbers = std::move (numbers);
            }

            // -- strings --
            if (str_offsets_size_shorts > 0) {
                const auto strings_offsets
                  = ::parse_string_offsets_section (begin,
                                                    end,
                                                    str_offsets_size_shorts);

                auto string_table = ::parse_string_table (begin,
                                                          end,
                                                          strings_offsets,
                                                          str_table_size);

                terminfo.strings = std::move (string_table);
            }
        }

        // TODO mention in the exception that its parsing the extended info

        // parse extended ncurses table
        // if (begin < end) {
        //     parse_extended_terminfo (begin,
        //                              end,
        //                              terminfo.extended_bools,
        //                              terminfo.extended_numbers,
        //                              terminfo.extended_strings);
        //     //     const auto extended_header = ::parse_header_section (begin, end);

        //     //     const int16_t bools_size              = extended_header[0];
        //     //     const int16_t nums_size_shorts        = extended_header[1];
        //     //     const int16_t str_offsets_size_shorts = extended_header[2];

        //     //     // the order of last two is confusing
        //     //     const int16_t str_table_size  = extended_header[4];
        //     //     const int16_t last_str_offset = extended_header[3];

        //     //     // -- bools --
        //     //     if (bools_size > 0) {
        //     //         auto bools = ::parse_bool_section (begin, end, bools_size);
        //     //         int  _x    = 0;
        //     //     }

        //     //     // for legacy reasons there's a padding byte if numbers start on a
        //     //     // uneven address
        //     //     if ((bools_size + 1) % 2 != 1)
        //     //         ++begin;

        //     //     // -- numbers --
        //     //     if (nums_size_shorts > 0) {
        //     //         auto numbers
        //     //           = ::parse_numbers_section (begin, end, nums_size_shorts);
        //     //     }

        //     //     // -- strings --
        //     //     if (str_offsets_size_shorts > 0) {
        //     //         const auto strings_offsets
        //     //           = ::parse_string_offsets_section (begin,
        //     //                                             end,
        //     //                                             str_offsets_size_shorts);

        //     //         int _y = 0;

        //     //         auto string_table = ::parse_string_table (begin,
        //     //                                                   end,
        //     //                                                   strings_offsets,
        //     //                                                   str_table_size);

        //     //         int _x = 0;
        //     //     }

        //     //     // // is this needed ???
        //     //     // if (str_offsets.back() != last_str_table_offset_bytes)
        //     //     //     throw std::runtime_error (
        //     //     //       "last string table offset does not match between the extended "
        //     //     //       "terminfo header and string offsets section");

        //     //     // const auto string_table
        //     //     //   = ::parse_string_table (begin, end, str_offsets, str_table_size);
        // }

        return terminfo;
    }

    Terminfo
    parse_terminfo_file (const std::string_view& path)
    {
        // TODO limit file size somehow?
        std::ifstream file (path.data(), std::ios::binary);
        if (!file.good())
            throw Error ("File '" + std::string (path) + "' does not exist");

        std::vector<uint8_t> buffer { std::istreambuf_iterator<char> (file),
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
parse_header_section (uint8_t*& begin, const uint8_t* end)
{
    if (begin + 5 > end)
        throw mtinfo::ErrorEOF ("the header");

    return i16_le<5> (begin);
}

std::vector<std::string>
parse_names (uint8_t*& begin, const uint8_t* end, size_t length)
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
parse_bool_section (uint8_t*& begin, const uint8_t* end, size_t length)
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

std::vector<std::optional<uint16_t>>
parse_numbers_section (uint8_t*& begin, const uint8_t* end, size_t length)
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

std::vector<bool>
parse_string_offsets_section (uint8_t*&      begin,
                              const uint8_t* end,
                              size_t         length_shorts)
{
    // offsets section contains short integers (2 bytes) that specify where a
    // string starts relative to start of the string table
    // negative offsets are invalid and -1 means the string isnt defined
    //
    // i've changed it to be a bool, it signifies if the string at the index
    // is defined or not

    if (begin + (length_shorts * 2) > end)
        throw mtinfo::ErrorEOF ("the string offsets section");

    std::vector<bool>    offsets (length_shorts);
    std::vector<int16_t> offsets_raw = ::i16_le (begin, length_shorts);

    for (size_t i = 0; i < length_shorts; ++i)
        offsets[i] = offsets_raw[i] >= 0;

    return offsets;
}

std::vector<std::optional<std::string>>
parse_string_table (uint8_t*&                begin,
                    const uint8_t*           end,
                    const std::vector<bool>& offsets,
                    size_t                   length_bytes)
{
    // strings are gathered by using offsets from previous section that specify
    // if the string exists, and if it does where does it start relative to
    // start of the strings table
    //
    // offsets are not offsets anymore but simply signify if the string at the
    // index exist at all

    if (begin + length_bytes > end)
        throw mtinfo::ErrorEOF ("the string table");

    std::vector<std::optional<std::string>> string_table (offsets.size());
    std::string str_table_raw (begin, begin + length_bytes);
    begin += length_bytes;

    {
        auto begin = str_table_raw.begin();
        auto end   = str_table_raw.end();

        for (size_t i = 0; i < offsets.size(); ++i) {
            if (!offsets[i]) {
                string_table[i] = {};
                continue;
            }

            // find the \0 char aka end of the string
            auto null_ch_iter
              = std::find (std::execution::par, begin, end, '\0');

            if (null_ch_iter == end)
                throw mtinfo::ErrorParsing (
                  "null character not found in strings table (index "
                  + std::to_string (i) + ")");

            // populate the string table
            string_table[i] = std::string (begin, null_ch_iter);

            if (null_ch_iter + 1 <= end) {
                begin = null_ch_iter + 1;
                end   = str_table_raw.end();
            } else
                throw mtinfo::ErrorParsing (
                  "the string table has less strings than requested ("
                  + std::to_string (i) + " < " + std::to_string (offsets.size())
                  + ")");
        }
    }

    return string_table;
}

// void
// parse_extended_terminfo (
//   uint8_t*&                                 begin,
//   const uint8_t*                            end,
//   std::map<std::string_view, bool>&        extended_bools,
//   std::map<std::string_view, uint16_t>&    extended_numbers,
//   std::map<std::string_view, std::string>& extended_strings);

// // refractor to collect the strings by looking for the \0 char not offsets
// std::vector<std::string>
// parse_ext_string_table (uint8_t*&      begin,
//                         const uint8_t* end,
//                         size_t        count,
//                         size_t        length_bytes)
// {
//     if (begin + length_bytes > end)
//         throw mtinfo::ErrorEOF ("the string table");

//     std::vector<std::optional<std::string>> string_table (count);
//     const std::string strings_table_raw (begin, begin + length_bytes);
//     begin += length_bytes;

//     for (size_t i = 0; i < count; ++i) {
//         // std::string constructor finds the first \0 character
//         string_table[i] = std::string (strings_table_raw.data() + *offsets[i]);
//     }

//     return string_table;
// }

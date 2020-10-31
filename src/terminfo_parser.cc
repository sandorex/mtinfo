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
#include "util.hh"

#include <cassert>
#include <execution>
#include <fstream>
#include <iostream>
#include <iterator>
#include <utility>

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

            for (auto&& i : header)
                if (i < 0)
                    throw mtinfo::Error ("negative number in the header");

            const uint8_t names_size       = static_cast<uint8_t> (header[0]);
            const uint8_t bools_size       = static_cast<uint8_t> (header[1]);
            const uint8_t nums_size_shorts = static_cast<uint8_t> (header[2]);
            const uint8_t str_offsets_size_shorts
              = static_cast<uint8_t> (header[3]);
            const uint8_t str_table_size = static_cast<uint8_t> (header[4]);

            if (names_size == 0)
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
            // TODO should this be table size instead ??
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
        if (begin < end)
            parse_extended_terminfo (begin,
                                     end,
                                     terminfo.extended_bools,
                                     terminfo.extended_numbers,
                                     terminfo.extended_strings);

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
    // index exists at all

    if (begin + length_bytes > end)
        throw mtinfo::ErrorEOF ("the string table");

    std::vector<std::optional<std::string>> string_table (offsets.size());
    std::string str_table_raw (begin, begin + length_bytes);
    begin += length_bytes;

    {
        auto raw_begin = str_table_raw.begin();
        auto raw_end   = str_table_raw.end();

        for (size_t i = 0; i < offsets.size(); ++i) {
            if (!offsets[i]) {
                string_table[i] = {};
                continue;
            }

            // find the \0 char aka end of the string
            auto null_ch_iter
              = std::find (std::execution::par, raw_begin, raw_end, '\0');

            if (null_ch_iter == raw_end)
                throw mtinfo::ErrorParsing (
                  "null character not found in strings table (index "
                  + std::to_string (i) + ")");

            // populate the string table
            string_table[i] = std::string (raw_begin, null_ch_iter);

            if (null_ch_iter + 1 <= raw_end) {
                raw_begin = null_ch_iter + 1;
                raw_end   = str_table_raw.end();
            } else
                throw mtinfo::ErrorParsing (
                  "the string table has less strings than requested ("
                  + std::to_string (i) + " < " + std::to_string (offsets.size())
                  + ")");
        }
    }

    return string_table;
}

void
parse_extended_terminfo (
  uint8_t*&                                begin,
  const uint8_t*                           end,
  std::map<std::string_view, bool>&        extended_bools,
  std::map<std::string_view, uint16_t>&    extended_numbers,
  std::map<std::string_view, std::string>& extended_strings)
{
    const auto extended_header = ::parse_header_section (begin, end);

    for (auto&& i : extended_header)
        if (i < 0)
            throw mtinfo::Error ("negative number in the extended header");

    const uint8_t bools_size       = static_cast<uint8_t> (extended_header[0]);
    const uint8_t nums_size_shorts = static_cast<uint8_t> (extended_header[1]);
    const uint8_t str_offsets_size_shorts
      = static_cast<uint8_t> (extended_header[2]);
    const uint8_t str_table_length = static_cast<uint8_t> (extended_header[3]);
    const uint8_t str_table_size   = static_cast<uint8_t> (extended_header[4]);

    auto bools = ::parse_bool_section (begin, end, bools_size);

    // for legacy reasons there's a padding byte if numbers start on a
    // uneven address
    if ((bools_size + 1) % 2 != 1)
        ++begin;

    std::vector<uint16_t> numbers;
    {
        auto numbers_raw
          = ::parse_numbers_section (begin, end, nums_size_shorts);

        if (!unpack_optional (numbers_raw, numbers))
            throw mtinfo::Error (
              "An empty number found in the extended numbers section");
    }

    // TODO
    // if (str_offsets_size_shorts > 0) {

    // FIXME there is no check if the string is actaully defined, cause it can't
    // be left undefined in extended ncurses table

    // skip str offsets
    begin += str_offsets_size_shorts * 2;

    // skip names str offsets (best guess, i do not actually know what this is)
    begin += (bools.size() + numbers.size() + str_offsets_size_shorts) * 2;

    std::vector<bool>        offsets (str_table_length, true);
    std::vector<std::string> str_table (str_offsets_size_shorts);
    std::vector<std::string> names_table (str_table_length
                                          - str_offsets_size_shorts);

    // <-- MEMORY ISSUE HERE
    // im now fucking up the strings, im suspecting the unknown bytes above

    // parse full string table, split into the real string table and names table
    {
        auto table_raw_raw
          = ::parse_string_table (begin, end, offsets, str_table_size);
        std::vector<std::string> table_raw;

        if (!unpack_optional (table_raw_raw, table_raw))
            throw mtinfo::Error (
              "An empty string found in the extended string table");

        // FIXME TODO these static casts may cause issues for bigger vectors
        std::move (table_raw.begin(),
                   table_raw.begin()
                     + static_cast<signed> (str_table.capacity()),
                   str_table.begin());

        std::move (table_raw.begin()
                     + static_cast<signed> (str_table.capacity()),
                   table_raw.end(),
                   names_table.begin());
    }

    size_t index = 0;
    for (auto&& i : bools)
        extended_bools.insert_or_assign (std::move (names_table[index++]), i);

    for (auto&& i : numbers)
        extended_numbers.insert_or_assign (std::move (names_table[index++]), i);

    for (auto&& i : str_table)
        extended_strings.insert_or_assign (std::move (names_table[index++]),
                                           std::move (i));
}

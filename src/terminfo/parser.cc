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

#define _CRT_SECURE_NO_WARNINGS

#include "mtinfo/errors.hh"
#include "mtinfo/terminfo/parser_util.hh"
#include "mtinfo/terminfo/parser.hh"
#include "mtinfo/terminfo/terminfo.hh"

#include <cstdlib>
#include <string>
#include <fstream>
#include <algorithm>
#include <filesystem>

using namespace std::literals::string_view_literals;

namespace mtinfo::terminfo::parser::internal {
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

    std::optional<std::string> get_env(const std::string_view& key) {
        const auto pointer = std::getenv(key.data());
        if (pointer != nullptr)
            return std::string(pointer);

        return {};
    }

    Terminfo parse_compiled_terminfo_file (const std::string_view& path, bool parse_extended_terminfo) {
        namespace fs = std::filesystem;

        if (!fs::is_regular_file(path))
            throw error::Error("The path either does not exist or is not a regular file"); // TODO: another error type? better msg?

        std::ifstream file (path.data(), std::ios::binary);
        if (!file.good())
            throw mtinfo::error::Error ("Error reading file '" + std::string (path) + "'");

        std::vector<int8_t> buffer;
        buffer.reserve(1000);

        const auto end = std::istreambuf_iterator<char>();
        for (auto begin = std::istreambuf_iterator<char> (file); begin != end;) {
            if (buffer.size() > 5000) // TODO: define global maximum file size
                throw error::Error("The file is above the file size limit of 10kb");

            buffer.push_back(*begin++);
        }

        // close the file
        file.close();

        return parse_compiled_terminfo (ByteIterator(buffer.data(), buffer.size()), parse_extended_terminfo);
    }

    std::optional<Terminfo> parse_compiled_terminfo_from_directory (const std::string_view& directory, const std::string_view& name, bool parse_extended_terminfo) {
        namespace fs = std::filesystem;

        const fs::path dir(directory);

        // TODO: global define for env names
        const std::string terminfo_name { name.empty() ? get_env("TERM").value_or("") : name };

        // NOTE: is_directory is false if the path does not exist
        if (!fs::is_directory(dir) || terminfo_name.empty())
            return {}; // the path is not a directory, does not exist or the name is empty

        // compiled terminfo files are stored in directory with name of the first character
        const auto letter_dir = dir / name.substr(0, 1);

        // check if directory with first character of the name exists
        if (!fs::is_directory(letter_dir))
            return {};

        const auto file_path = letter_dir / terminfo_name;

        if (!fs::is_regular_file(file_path))
            return {};

        return parse_compiled_terminfo_file(file_path.string(), parse_extended_terminfo);
    }

    std::optional<Terminfo> parse_compiled_terminfo_from_env(const std::string_view& name, bool parse_extended_terminfo) {
        namespace fs = std::filesystem;

        // prefer the supplied name over the environ one
        auto terminfo_name = name;
        if (terminfo_name.empty())
            terminfo_name = get_env("TERM").value_or("");

        // the terminfo name cannot be empty
        if (terminfo_name.empty())
            return {};

        // check the $TERMINFO
        const auto terminfo_dir = get_env("TERMINFO");
        if (terminfo_dir.has_value()) {
            auto result = parse_compiled_terminfo_from_directory(terminfo_name, terminfo_dir.value(), parse_extended_terminfo);

            if (result.has_value())
                return result;
        }

        // check the $HOME/.terminfo
        const auto home_dir = get_env("HOME");
        if (home_dir.has_value()) {
            auto result = parse_compiled_terminfo_from_directory(terminfo_name, (fs::path(terminfo_name) / ".terminfo").string(), parse_extended_terminfo);

            if (result.has_value())
                return result;
        }

        // TODO: colon seperated list..
        // empty directory name means compiled-in default location @TERMINFO@

        // check the $TERMINFO_DIRS
        // const auto terminfo_dirs = get_env("TERMINFO_DIRS");
        // if (home_dir.has_value()) {
        //     result = parse_compiled_terminfo_from_directory((path(terminfo_name) / ".terminfo").string(), home_dir.value(), parse_extended_terminfo);

        //     if (result.has_value())
        //         return result;
        // }

        // check compiled-in paths @TERMINFO_DIRS@ and @TERMINFO@

        return {};
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
        header.total_strings_count = iter.i16();
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

    std::vector<std::optional<int32_t>> parse_numbers_section (ByteIterator& iter, size_t length, bool is_32bit) {
        /*
            numbers are signed shorts, any negative number is invalid, -1 means it's
            undefined but im not gonna bother
        */

        if ((is_32bit && iter.is_out_of_bounds<int32_t>(length))
            || (!is_32bit && iter.is_out_of_bounds<int16_t>(length)))
            throw mtinfo::error::EOFError ("the numbers section");

        std::vector<std::optional<int32_t>> numbers(length);
        std::generate_n(numbers.begin(), length, [&]() -> std::optional<int32_t> {
            const int32_t value = is_32bit ? iter.i32() : iter.i16();

            if (value >= 0)
                return value;

            return {};
        });

        return numbers;
    }

    std::vector<std::optional<uint16_t>> parse_string_offsets(ByteIterator& iter, size_t count) {
        /* TODO */
        // offsets section contains short integers (2 bytes) that specify where a
        // string starts relative to start of the string table
        // negative offsets are invalid and -1 means the string isnt defined
        //
        // i've changed it to be a bool, it signifies if the string at the index
        // is defined or not

        if (iter.is_out_of_bounds<uint16_t>(count))
            throw mtinfo::error::EOFError ("the string offsets section");

        std::vector<std::optional<uint16_t>> offsets (count);
        std::generate_n(offsets.begin(), count, [&]() -> std::optional<uint16_t> {
            const auto value = iter.i16();

            if (value < 0)
                return {};

            return static_cast<uint16_t>(value);
        });

        return offsets;
    }

    std::vector<std::optional<std::string>> parse_string_table (ByteIterator& iter, std::vector<std::optional<uint16_t>> offsets) {
        /* TODO */
        // strings are gathered by using offsets from previous section that specify
        // if the string exists, and if it does where does it start relative to
        // start of the strings table
        //
        // offsets are not offsets anymore but simply signify if the string at the
        // index exists at all

        std::vector<std::optional<std::string>> string_table (offsets.size());

        // in case there is no offsets just skip everything
        if (offsets.size() == 0)
            return string_table;

        ByteIterator iter_start, iter_end;

        // iterates over offsets and collects strings until \0 character
        // if it reached the end without finding the character an error will be thrown
        for (size_t i = 0; i < offsets.size(); ++i) {
            if (!offsets[i].has_value()) {
                string_table[i] = {};
                continue;
            }

            bool found_end = false;
            iter_start = iter + offsets[i].value();
            iter_end = iter_start;

            while (!iter_end.is_out_of_bounds(1)) {
                if (*iter_end++ == '\0') {
                    found_end = true;
                    string_table[i] = std::string(&iter_start, &iter_end - 1); // -1 is for '\0' character
                    break;
                }
            }

            if (!found_end)
                throw error::ParsingError("Could not find the end of a string"); // TODO more clear error message?
        }

        // advance the main iterator
        iter = iter_end;

        return string_table;
    }

    Terminfo parse_compiled_terminfo(ByteIterator iter, bool parse_extended) {
        Terminfo terminfo;
        const Header header = parse_header_section(iter);

        if (header.magic_number == MAGIC_NUMBER_32BIT)
            terminfo.is_32bit = true;
        else if (header.magic_number == MAGIC_NUMBER_16BIT)
            terminfo.is_32bit = false;
        else
            throw error::Error("Invalid magic number"); // TODO add it to the error message

        // TODO check if header is proper, all values above zero

        if (header.names_size <= 0 || header.bools_count < 0 || header.numbers_count < 0 || header.strings_table_size < 0)
            throw error::Error("Invalid header values");

        // TODO figure out description, also check if names are valid
        terminfo.aliases = parse_names(iter, header.names_size);

        if (terminfo.aliases.size() > 1) {
            terminfo.name = terminfo.aliases.back();
            terminfo.aliases.pop_back();
        }

        if (header.bools_count > 0)
            terminfo.bools = parse_bool_section(iter, header.bools_count);

        iter.align_short_boundary();

        if (header.numbers_count > 0)
            terminfo.numbers = parse_numbers_section(iter, header.numbers_count, terminfo.is_32bit);

        if (header.offsets_count > 0) {
            const auto string_offsets = parse_string_offsets(iter, header.offsets_count);
            terminfo.strings = parse_string_table(iter, string_offsets);
        }

        // parse extended if this is not eof
        if (parse_extended && !iter.is_out_of_bounds(1)) {
            terminfo.is_extended = true;

            iter.align_short_boundary();

            const ExtendedHeader ext_header = parse_extended_header_section(iter);

            // TODO check if header is valid

            std::vector<bool> bools;
            if (ext_header.bools_count > 0)
                bools = parse_bool_section(iter, ext_header.bools_count);

            iter.align_short_boundary();

            // TODO should these also change for 32bit mode?
            std::vector<std::optional<int32_t>> numbers;
            if (ext_header.numbers_count > 0)
                numbers = parse_numbers_section(iter, ext_header.numbers_count, terminfo.is_32bit);

            const int prop_name_count = ext_header.total_strings_count - ext_header.strings_count;

            const auto offsets_strings = parse_string_offsets(iter, ext_header.strings_count);
            const auto offsets_prop_names = parse_string_offsets(iter, prop_name_count);

            // TODO should these offsets be checked?

            const auto strings = parse_string_table(iter, offsets_strings);
            auto prop_names = parse_string_table(iter, offsets_prop_names);

            size_t index = 0;

            for (auto&& i : bools) {
                if (index >= prop_names.size())
                    throw error::ParsingError("Not enough property names found"); // TODO better error msg lul

                assert(prop_names[index].has_value());

                terminfo.extended_bools[prop_names[index++].value()] = i;
            }

            for (auto&& i : numbers) {
                if (index >= prop_names.size())
                    throw error::ParsingError("Not enough property names found"); // TODO better error msg lul

                assert(prop_names[index].has_value());
                assert(i.has_value());

                terminfo.extended_numbers[prop_names[index++].value()] = i.value();
            }

            for (auto&& i : strings) {
                if (index >= prop_names.size())
                    throw error::ParsingError("Not enough property names found"); // TODO better error msg lul

                assert(prop_names[index].has_value());
                assert(i.has_value());

                terminfo.extended_strings[prop_names[index++].value()] = i.value();
            }
        }

        return terminfo;
    }
}

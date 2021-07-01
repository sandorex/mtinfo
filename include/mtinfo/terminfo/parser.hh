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
#include "mtinfo/terminfo/parser_util.hh"
#include "mtinfo/terminfo/terminfo.hh"
#include "mtinfo/errors.hh"

#include <array>
#include <iostream>
#include <iterator>
#include <optional>
#include <algorithm>

namespace mtinfo::terminfo::parser::internal {
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

        // total count of strings including property names
        int16_t total_strings_count;

        // last offset which is value of a string not name of a property
        int16_t last_string_offset;
    };

    MTINFO_EXPORT Terminfo
    parse_compiled_terminfo_file (const std::string_view& path, bool parse_extended_terminfo = true);

    /// tries to find terminfo in directory
    MTINFO_EXPORT std::optional<Terminfo>
    parse_compiled_terminfo_from_directory (const std::string_view& directory, const std::string_view& name = "", bool parse_extended_terminfo = true);

    /// tries to find terminfo in common paths
    /// if it cannot be found it returns std::nullopt if there is any problem reading, parsing it
    /// then an exception will be thrown
    MTINFO_EXPORT std::optional<Terminfo>
    parse_compiled_terminfo_from_env(const std::string_view& name = "", bool parse_extended_terminfo = true);

    // parses terminfo header
    Header parse_header_section (ByteIterator& iter);

    ExtendedHeader parse_extended_header_section (ByteIterator& iter);

    std::vector<std::string> parse_names (ByteIterator& iter, size_t length);

    std::vector<bool> parse_bool_section (ByteIterator& iter, size_t length);

    std::vector<std::optional<int32_t>> parse_numbers_section (ByteIterator& iter, size_t length);

    std::vector<std::optional<uint16_t>> parse_string_offsets(ByteIterator& iter, size_t count);

    std::vector<std::optional<std::string>> parse_string_table (ByteIterator& iter, std::vector<std::optional<uint16_t>> offsets);

    MTINFO_EXPORT Terminfo
    parse_compiled_terminfo(ByteIterator iter, bool parse_extended = true);
}

// PUBLIC API
namespace mtinfo::terminfo::parser {
    using internal::parse_compiled_terminfo_file;
    using internal::parse_compiled_terminfo_from_directory;
    using internal::parse_compiled_terminfo_from_env;
    using internal::parse_compiled_terminfo;
}

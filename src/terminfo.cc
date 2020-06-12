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

#include <iostream>
#include <fstream>
#include <iterator>

#include "mtinfo/terminfo.hh"

// get a signed short (2 bytes) using little endian byte order
template <class T>
inline int16_t i16_le(T& iter) {
    const auto a = *iter++;
    const auto b = *iter++;

    return (b << 8) | a;
}

template <class T>
inline std::vector<int16_t> i16_le(T& iter, size_t amount) {
    std::vector<int16_t> buffer(amount);

    for (size_t i = 0; i < amount; i++)
        buffer[i] = i16_le(iter);

    return buffer;
}

// splits string by delimiter
std::vector<std::string> split(std::string input, const std::string_view& delimiter) {
    std::vector<std::string> buffer;
    size_t pos = 0;
    while ((pos = input.find(delimiter)) != std::string::npos) {
        buffer.push_back(input.substr(0, pos));
        input.erase(0, pos + delimiter.length());
    }

    // the leftovers
    buffer.push_back(input);

    return buffer;
}

namespace mtinfo::terminfo {
    template <class BeginIt, class EndIt>
    Terminfo parse_terminfo(BeginIt begin, EndIt end) {
        Terminfo terminfo;

        /* TERMINFO FORMAT (taken from https://linux.die.net/man/5/term)

        The format has been chosen so that it will be the same on all hardware. An 8 or more bit byte is assumed, but no assumptions about byte ordering or sign extension are made.

        The compiled file is created with the tic program, and read by the routine setupterm. The file is divided into six parts: the header, terminal names, boolean flags, numbers, strings, and string table.

        The header section begins the file. This section contains six short integers in the format described below. These integers are
            (1) the magic number (octal 0432);
            (2) the size, in bytes, of the names section;
            (3) the number of bytes in the boolean section;
            (4) the number of short integers in the numbers section;
            (5) the number of offsets (short integers) in the strings section;
            (6) the size, in bytes, of the string table.
        Short integers are stored in two 8-bit bytes. The first byte contains the least significant 8 bits of the value, and the second byte contains the most significant 8 bits. (Thus, the value represented is 256*second+first.) The value -1 is represented by the two bytes 0377, 0377; other negative values are illegal. This value generally means that the corresponding capability is missing from this terminal. Note that this format corresponds to the hardware of the VAX and PDP -11 (that is, little-endian machines). Machines where this does not correspond to the hardware must read the integers as two bytes and compute the little-endian value.

        The terminal names section comes next. It contains the first line of the terminfo description, listing the various names for the terminal, separated by the '|' character. The section is terminated with an ASCII NUL character.

        The boolean flags have one byte for each flag. This byte is either 0 or 1 as the flag is present or absent. The capabilities are in the same order as the file <term.h>.

        Between the boolean section and the number section, a null byte will be inserted, if necessary, to ensure that the number section begins on an even byte (this is a relic of the PDP-11's word-addressed architecture, originally designed in to avoid IOT traps induced by addressing a word on an odd byte boundary). All short integers are aligned on a short word boundary.

        The numbers section is similar to the flags section. Each capability takes up two bytes, and is stored as a little-endian short integer. If the value represented is -1, the capability is taken to be missing.

        The strings section is also similar. Each capability is stored as a short integer, in the format above. A value of -1 means the capability is missing. Otherwise, the value is taken as an offset from the beginning of the string table. Special characters in ^X or \c notation are stored in their interpreted form, not the printing representation. Padding information $<nn> and parameter information %x are stored intact in uninterpreted form.

        The final section is the string table. It contains all the values of string capabilities referenced in the string section. Each string is null terminated.

        EXTENDED STORAGE FORMAT

        The previous section describes the conventional terminfo binary format. With some minor variations of the offsets (see PORTABILITY), the same binary format is used in all modern UNIX systems. Each system uses a predefined set of boolean, number or string capabilities.

        The ncurses libraries and applications support extended terminfo binary format, allowing users to define capabilities which are loaded at runtime. This extension is made possible by using the fact that the other implementations stop reading the terminfo data when they have reached the end of the size given in the header. ncurses checks the size, and if it exceeds that due to the predefined data, continues to parse according to its own scheme.

        First, it reads the extended header (5 short integers):
            (1)

            count of extended boolean capabilities

            (2)

            count of extended numeric capabilities

            (3)

            count of extended string capabilities

            (4)

            size of the extended string table in bytes.

            (5)

            last offset of the extended string table in bytes.
        Using the counts and sizes, ncurses allocates arrays and reads data for the extended capabilties in the same order as the header information.

        The extended string table contains values for string capabilities. After the end of these values, it contains the names for each of the extended capabilities in order, e.g., booleans, then numbers and finally strings.

        */

        // TODO iterator checks, check if it has gone out of bounds

        // header must start with the magic number (don't ask just believe)
        const int16_t magic_number = ::i16_le(begin);
        if (magic_number != MAGIC_NUMBER_16)
            throw std::runtime_error("Invalid magic number"); // TODO errors

        // sizes
        const int16_t names_section_size_bytes = ::i16_le(begin);
        const int16_t bool_section_size_bytes = ::i16_le(begin);

        const int16_t num_section_size_shorts = ::i16_le(begin);
        const int16_t str_offsets_section_size_shorts = ::i16_le(begin);

        const int16_t str_table_section_size_bytes = ::i16_le(begin);

        // temp
        std::cout << "names " << names_section_size_bytes << " bytes" << '\n'
                  << "bool " << bool_section_size_bytes << " bytes" << '\n'
                  << "num " << num_section_size_shorts << " shorts (2 bytes)" << '\n'
                  << "str offsets " << str_offsets_section_size_shorts << " shorts (2 bytes)" << '\n'
                  << "str table " << str_table_section_size_bytes << " bytes" << '\n';

        // if (begin + names_section_size_bytes >= end) ...

        // -- names --
        // read the names
        terminfo.names = split(std::string(begin, begin + names_section_size_bytes), "|");
        begin += names_section_size_bytes;

        // -- bools --
        // read the bools
        std::vector<int8_t> bools_raw(begin, begin + bool_section_size_bytes);
        begin += bool_section_size_bytes;

        // ensure no undefined bools are written
        if (bools_raw.size() > TERMINFO_BOOLEANS_LEN)
            bools_raw.resize(TERMINFO_BOOLEANS_LEN);

        // only write if there's anything to write
        if (bools_raw.size() > 0)
            // write over the defaults
            std::copy(bools_raw.begin(), bools_raw.end(), terminfo.bools.begin());

        // for legacy reasons there's a padding byte if numbers start on a
        // uneven address
        if ((names_section_size_bytes + bool_section_size_bytes) % 2 == 1) {
            std::cout << "uneven address, skipping a byte" << '\n';
            ++begin;
        }

        // -- numbers --
        // read the numbers
        std::vector<int16_t> numbers_raw = ::i16_le(begin, num_section_size_shorts);

        // ensure no undefined bools are written
        if (numbers_raw.size() > TERMINFO_NUMBERS_LEN)
            numbers_raw.resize(TERMINFO_NUMBERS_LEN);

        // supported copy them over default values
        std::copy(numbers_raw.begin(), numbers_raw.end(), terminfo.numbers.begin());

        // -- strings --
        // read the offsets
        std::vector<int16_t> str_offsets = ::i16_le(begin, str_offsets_section_size_shorts);

        // ensure no undefined bools are written
        if (str_offsets.size() > TERMINFO_STRINGS_LEN)
            str_offsets.resize(TERMINFO_STRINGS_LEN);

        // read the strings table
        const std::string strings_table(begin, begin + str_table_section_size_bytes);
        begin += str_table_section_size_bytes;

        {
            size_t count = 0;
            for (auto &&offset : str_offsets) { // TODO error handling
                if (offset == -1) {
                    ++count;
                    continue;
                }

                std::cout << "offset " << offset << '\n';

                const auto null_ch_position = strings_table.find('\0', offset);
                if (null_ch_position == std::string::npos)
                    throw std::runtime_error("cannot find the end of a string in the strings table");

                terminfo.strings[count++] = strings_table.substr(offset, null_ch_position);
            }
        }

        for (auto &&i : terminfo.strings)
            if (i.has_value())
                std::cout << "'" << i.value() << "'" << '\n';
        std::cout << '\n';

        // TODO parse extended curses table thingy

        return terminfo;
    }

    Terminfo parse_terminfo(const std::vector<uint8_t>& data) {
        return parse_terminfo(data.cbegin(), data.cend());
    }

    Terminfo parse_terminfo_file(const std::string_view& path) {
        std::ifstream file(path.data(), std::ios::binary); // TODO error if file doesnt exist

        std::vector<unsigned char> buffer {
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        };

        return parse_terminfo(buffer.cbegin(), buffer.cend());
    }
}

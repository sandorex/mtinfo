// Copyright 2020-2021 Aleksandar Radivojević
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

// #include "terminfo_parser.hh"

// #include <mtinfo/new_terminfo_parser.hh>
// #include <mtinfo/errors.hh>

#include <mtinfo/terminfo/ctinfo_parser.hh>

#include <string>
#include <cassert>
#include <iostream>
#include <vector>
#include <fstream>

using mtinfo::terminfo::parser::parse_compiled_terminfo_file;

int
main()
{
    // TODO limit file size somehow?
    // const auto path = "F:\\Workspace\\mtinfo\\test\\old\\xterm-kitty";
    // const auto path = "F:\\Workspace\\mtinfo\\test\\output\\w\\with-description";
    const auto path = "F:\\Workspace\\mtinfo\\test\\output\\e\\extended";

    auto terminfo = parse_compiled_terminfo_file (path);

    // std::ifstream file(path, std::ios::binary);
    // if (!file.good())
    //     throw mtinfo::error::Error("File does not exist");

    // std::vector<char> buffer {
    //     std::istreambuf_iterator<char>(file),
    //     std::istreambuf_iterator<char>()
    // };

    // auto iter = buffer.begin();
    // auto end = buffer.end();

    // std::cout << "size: " << buffer.size() << " bytes" << '\n';

    // auto header = mtinfo::parse_header_section(iter, end);

    // if (header.magic_number != mtinfo::MAGIC_NUMBER)
    //     throw mtinfo::error::Error("Invalid magic number");

    // const auto names_pair = mtinfo::parse_names(iter, end, header.names_section_size);
    // const auto names = names_pair.first;
    // const auto description = names_pair.second;
    std::cout << "desc: " << terminfo.description.value_or("BLANK") << "\n\n";

    std::cout << "names (" << terminfo.aliases.size() << "): " << '\n';
    for (auto &&i : terminfo.aliases)
        std::cout << i << '\n';

    std::cout << '\n';

    // const auto bools = mtinfo::parse_bool_section(iter, end, header.bool_section_size);
    std::cout << "bools (" << terminfo.bools.size() << "): " << '\n';
    for (size_t i = 0; i < terminfo.bools.size(); ++i)
        std::cout << i << ": " << terminfo.bools[i] << '\n';

    std::cout << '\n';

    // const auto numbers = mtinfo::parse_numbers_section(iter, end, header.num_section_size);
    std::cout << "numbers (" << terminfo.numbers.size() << "): " << '\n';
    for (size_t i = 0; i < terminfo.numbers.size(); ++i)
        std::cout << i << ": " << terminfo.numbers[i].value_or(-1) << '\n';

    std::cout << '\n';

    // const auto offsets = mtinfo::parse_string_offsets_section(iter, end, header.offset_section_size);
    // const auto strings = mtinfo::parse_string_table(iter, end, offsets, header.string_table_size);
    std::cout << "strings (" << terminfo.strings.size() << "): " << '\n';
    for (size_t i = 0; i < terminfo.strings.size(); ++i)
        std::cout << i << ": " << terminfo.strings[i].value_or("") << '\n';


    if (terminfo.is_extended) {
        std::cout << '\n' << "EXTENDED" << "\n\n";
    }


    // for (auto &&i : header) {
    //     std::cout << i << '\n';
    // }


    // return parse_terminfo (buffer.data(), buffer.size());

    // const auto a = "\x01\x02\x03\x04\x05\x06";
    // auto iter = a;
    // const auto end = a + 5;


    // std::cout << u8(iter) << '\n' << *iter << '\n';
    // try {
        // auto header = parse_header_section(iter, a.cend());

        // if (is_out_of_bounds<int16_t, 6>(iter, end)) {
        //     std::cout << "true" << '\n';
        // } else {
        //     std::cout << "false" << '\n';
        // }
    // } catch (std::exception ex) {
    //     std::cout << ex.what() << '\n';
    // }

    // if (argc <= 1) {
    //     std::cout << "please enter path to the terminfo file" << '\n';
    //     return 1;
    // }

    // using namespace mtinfo::internal::constants;

    // mtinfo::Terminfo terminfo;
    // // try {
    // terminfo = mtinfo::parse_terminfo_file ("test/output/e/extended");
    // } catch (const std::exception& e) {
    //     std::cerr << e.what() << '\n';
    //     return 1;
    // }

    // std::cout << "description: " << '\n' << terminfo.description << '\n';

    // std::cout << '\n' << "aliases (" << terminfo.aliases.size() << "):" << '\n';
    // for (auto&& i : terminfo.aliases) {
    //     std::cout << "'" << i << "'" << '\n';
    // }

    // std::cout << '\n' << "bools (" << terminfo.bools.size() << "):" << '\n';
    // for (size_t i = 0; i < terminfo.bools.size(); ++i) {
    //     auto value = terminfo.bools.at (i);
    //     if (!value)
    //         continue;

    //     if (i >= TERMINFO_BOOL_LEN)
    //         std::cout << "unknown (" << std::to_string (i) << ")" << '='
    //                   << value << '\n';
    //     else
    //         std::cout << TERMINFO_BOOL_NAMES_SHORT[i] << " ("
    //                   << std::to_string (i) << ")" << '=' << value << '\n';
    // }

    // std::cout << '\n' << "numbers (" << terminfo.numbers.size() << "):" << '\n';
    // for (size_t i = 0; i < terminfo.numbers.size(); ++i) {
    //     auto value = terminfo.numbers.at (i);
    //     if (!value.has_value())
    //         continue;

    //     if (i >= TERMINFO_NUM_LEN)
    //         std::cout << "unknown (" << std::to_string (i) << ")" << '='
    //                   << value.value() << '\n';
    //     else
    //         std::cout << TERMINFO_NUM_NAMES_SHORT[i] << " ("
    //                   << std::to_string (i) << ")" << '=' << value.value()
    //                   << '\n';
    // }

    // std::cout << '\n' << "strings (" << terminfo.strings.size() << "):" << '\n';
    // for (size_t i = 0; i < terminfo.strings.size(); ++i) {
    //     auto value = terminfo.strings.at (i);
    //     if (!value.has_value())
    //         continue;

    //     if (i >= TERMINFO_STR_LEN)
    //         std::cout << "unknown (" << std::to_string (i) << ")" << '='
    //                   << value.value() << '\n';
    //     else
    //         std::cout << TERMINFO_STR_NAMES_SHORT[i] << " ("
    //                   << std::to_string (i) << ")" << '=' << value.value()
    //                   << '\n';
    // }

    // std::cout << '\n' << "extended bools:" << '\n';
    // for (auto&& i : terminfo.extended_bools) {
    //     std::cout << i.first << '\n';
    // }

    // std::cout << '\n' << "extended numbers:" << '\n';
    // for (auto&& i : terminfo.extended_numbers) {
    //     std::cout << i.first << '=' << i.second << '\n';
    // }

    // std::cout << '\n' << "extended strings:" << '\n';
    // for (auto&& i : terminfo.extended_strings) {
    //     std::cout << i.first << '=' << i.second << '\n';
    // }

    return 0;
}

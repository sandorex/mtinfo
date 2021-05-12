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

#include <mtinfo/terminfo/ctinfo_parser_util.hh>
#include <mtinfo/terminfo/terminfo.hh>
#include <mtinfo/terminfo/ctinfo_parser.hh>
#include <mtinfo/terminfo/terminfo_constants.hh>

#include <iterator>
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
    // const auto path = "F:\\Workspace\\mtinfo\\test\\output\\e\\extended-string-two";
    // const auto path = "F:\\Workspace\\mtinfo\\test\\output\\f\\firsts-n-extended";
    // const auto path = "F:\\Workspace\\mtinfo\\test\\output\\f\\firsts-n-extended-dd";
    const auto path = "F:\\Workspace\\mtinfo\\test\\output\\f\\firsts";
    // const auto path = "F:\\Workspace\\mtinfo\\test\\output\\a\\all";

    using namespace mtinfo::terminfo::constants;

    mtinfo::terminfo::Terminfo terminfo;
    try {
        terminfo = parse_compiled_terminfo_file (path);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    std::cout << "description: " << '\n' << terminfo.description.value_or("BLANK") << '\n';

    std::cout << '\n' << "aliases (" << terminfo.aliases.size() << "):" << '\n';
    for (auto&& i : terminfo.aliases) {
        std::cout << "'" << i << "'" << '\n';
    }

    std::cout << '\n' << "bools (" << terminfo.bools.size() << "):" << '\n';
    for (size_t i = 0; i < terminfo.bools.size(); ++i) {
        auto value = terminfo.bools.at (i);
        if (!value)
            continue;

        if (i >= TERMINFO_BOOL_LEN)
            std::cout << "unknown (" << std::to_string (i) << ")" << '='
                      << value << '\n';
        else
            std::cout << TERMINFO_BOOL_NAMES_SHORT[i] << " ("
                      << std::to_string (i) << ")" << '=' << value << '\n';
    }

    std::cout << '\n' << "numbers (" << terminfo.numbers.size() << "):" << '\n';
    for (size_t i = 0; i < terminfo.numbers.size(); ++i) {
        auto value = terminfo.numbers.at (i);
        if (!value.has_value())
            continue;

        if (i >= TERMINFO_NUM_LEN)
            std::cout << "unknown (" << std::to_string (i) << ")" << '='
                      << value.value() << '\n';
        else
            std::cout << TERMINFO_NUM_NAMES_SHORT[i] << " ("
                      << std::to_string (i) << ")" << '=' << value.value()
                      << '\n';
    }

    std::cout << '\n' << "strings (" << terminfo.strings.size() << "):" << '\n';
    for (size_t i = 0; i < terminfo.strings.size(); ++i) {
        const auto value = terminfo.strings.at (i);
        if (!value.has_value())
            continue;

        if (i >= TERMINFO_STR_LEN)
            std::cout << "unknown (" << std::to_string (i) << ")" << '='
                      << "REDACTED"/*value.value()*/ << '\n';
        else
            std::cout << TERMINFO_STR_NAMES_SHORT[i] << " ("
                      << std::to_string (i) << ")" << '=' << "REDACTED"/*value.value()*/
                      << '\n';
    }

    std::cout << '\n' << "extended bools:" << '\n';
    for (auto&& i : terminfo.extended_bools) {
        std::cout << i.first << (i.second ? "=true" : "=false") << '\n';
    }

    std::cout << '\n' << "extended numbers:" << '\n';
    for (auto&& i : terminfo.extended_numbers) {
        std::cout << i.first << '=' << i.second << '\n';
    }

    std::cout << '\n' << "extended strings:" << '\n';
    for (auto&& i : terminfo.extended_strings) {
        std::cout << i.first << '=' << "REDACTED"/*i.second*/ << '\n';
    }

    return 0;
}

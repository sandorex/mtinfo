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
#include <mtinfo/terminfo.hh>
#include <string>

void
generate_full_terminfo()
{
    using namespace mtinfo::internal::constants;

    std::cout << "# AUTOMATICALLY GENERATED FILE" << '\n';

    std::cout << "all|All caps" << ',' << '\n';

    std::cout << "#   bools" << '\n';
    for (size_t i = 0; i < TERMINFO_BOOL_LEN; i++)
        std::cout << "    " << TERMINFO_BOOL_NAMES_SHORT[i] << "," << '\n';
    std::cout << '\n';

    std::cout << "#   numbers" << '\n';
    for (size_t i = 0; i < TERMINFO_NUM_LEN; i++)
        std::cout << "    " << TERMINFO_NUM_NAMES_SHORT[i] << "#"
                  << std::to_string (i) << "," << '\n';
    std::cout << '\n';

    std::cout << "#   strings" << '\n';
    for (size_t i = 0; i < TERMINFO_STR_LEN; i++)
        std::cout << "    " << TERMINFO_STR_NAMES_SHORT[i] << "=a"
                  << std::to_string (i) << "," << '\n';
}

int
main (int argc, char const* argv[])
{
    using namespace mtinfo::internal::constants;

    if (argc <= 1) {
        generate_full_terminfo();

        return 0;
    }

    mtinfo::Terminfo terminfo;
    try {
        terminfo = mtinfo::parse_terminfo_file (argv[2]);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }

    // TODO messages when capabilities arent defined
    // TODO extended stuff

    std::cout << "# RECREATED FROM " << argv[2] << '\n';

    std::cout << terminfo.aliases.at (0) << "|TODO"
              << "," << '\n';

    std::cout << '\n';
    for (size_t i = 0; i < terminfo.bools.size(); i++) {
        const auto value = terminfo.bools.at (i);
        if (!value)
            continue;

        std::cout << "    " << TERMINFO_BOOL_NAMES_SHORT[i] << "," << '\n';
    }

    std::cout << '\n';

    for (size_t i = 0; i < terminfo.numbers.size(); i++) {
        const auto value = terminfo.numbers.at (i);
        if (!value.has_value())
            continue;

        std::cout << "    " << TERMINFO_NUM_NAMES_SHORT[i] << "#"
                  << std::to_string (value.value()) << "," << '\n';
    }

    std::cout << '\n';

    for (size_t i = 0; i < terminfo.strings.size(); i++) {
        const auto value = terminfo.strings.at (i);
        if (!value.has_value())
            continue;

        std::cout << "    " << TERMINFO_STR_NAMES_SHORT[i] << "#"
                  << value.value() << "," << '\n';
    }

    std::cout << '\n';

    for (size_t i = 0; i < terminfo.extended_bools.size(); i++) {
        const auto value = terminfo.strings.at (i);
        if (!value.has_value())
            continue;

        std::cout << "    " << TERMINFO_STR_NAMES_SHORT[i] << "#"
                  << value.value() << "," << '\n';
    }

    std::cout << '\n';

    for (auto&& i : terminfo.extended_bools)
        std::cout << "    " << i.first << '\n';

    std::cout << '\n';

    for (auto&& i : terminfo.extended_numbers)
        std::cout << "    " << i.first << "#" << i.second << '\n';

    std::cout << '\n';

    for (auto&& i : terminfo.extended_strings)
        std::cout << "    " << i.first << "=" << i.second << '\n';
}

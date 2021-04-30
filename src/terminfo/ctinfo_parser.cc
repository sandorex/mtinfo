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

#include <mtinfo/errors.hh>
#include <mtinfo/terminfo/ctinfo_parser_util.hh>
#include "mtinfo/terminfo/ctinfo_parser.hh"
#include "mtinfo/terminfo/terminfo.hh"

#include <string>
#include <fstream>
#include <algorithm>

namespace mtinfo::terminfo::parser {
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

    Terminfo parse_compiled_terminfo_file (const std::string_view& path) {
        // TODO limit file size somehow?
        std::ifstream file (path.data(), std::ios::binary);
        if (!file.good())
            throw mtinfo::error::Error ("File '" + std::string (path) + "' does not exist");

        std::vector<uint8_t> buffer { std::istreambuf_iterator<char> (file),
                                      std::istreambuf_iterator<char>() };

        return parse_compiled_terminfo (buffer.begin(), buffer.end());
    }
}

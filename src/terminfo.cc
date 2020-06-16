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

#include "mtinfo/terminfo.hh"

#include "mtinfo/errors.hh"
#include "terminfo_parser.hh"

#include <fstream>
#include <iostream>
#include <iterator>

namespace mtinfo
{
    Terminfo
    parse_terminfo (const std::vector<uint8_t>& data)
    {
        return ::parse_terminfo (data.cbegin(), data.cend());
    }

    Terminfo
    parse_terminfo (const uint8_t* data, size_t length)
    {
        return ::parse_terminfo (data, data + length); // UNTESTED!
    }

    Terminfo
    parse_terminfo_file (const std::string_view& path)
    {
        // TODO error if file doesnt exist
        std::ifstream file (path.data(), std::ios::binary);

        std::vector<unsigned char> buffer { std::istreambuf_iterator<char> (
                                              file),
                                            std::istreambuf_iterator<char>() };

        return ::parse_terminfo (buffer.cbegin(), buffer.cend());
    }
} // namespace mtinfo

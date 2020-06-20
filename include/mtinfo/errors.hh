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

#pragma once

#include <exception>
#include <string>

namespace mtinfo
{
    class Error : public std::exception
    {
        const std::string _msg;

    public:
        Error (const std::string_view& msg)
          : _msg (msg)
        {
        }

        virtual ~Error() noexcept
        {
        }

        virtual const char*
        what() const noexcept
        {
            return _msg.c_str();
        }
    };

    struct ErrorParsing : public Error {
        ErrorParsing (const std::string_view& msg)
          : Error ("Error while parsing: " + std::string (msg))
        {
        }
    };

    struct ErrorEOF : public ErrorParsing {
        ErrorEOF (const std::string_view& section)
          : ErrorParsing ("reached EOF while parsing " + std::string (section))
        {
        }
    };
} // namespace mtinfo

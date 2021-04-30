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

#pragma once

#include "mtinfo/export.hh"

#include <exception>
#include <string>

namespace mtinfo::error
{
    class MTINFO_EXPORT Error : public std::exception
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

    // an error that happend while parsing
    struct MTINFO_EXPORT ParsingError : public Error {
        ParsingError (const std::string_view& msg)
          : Error ("Error while parsing: " + std::string (msg))
        {
        }

    protected:
        // constructor without any message prefix
        ParsingError (const std::string_view& msg, int)
          : Error (msg)
        {
        }
    };

    // error that happends if a section is invalid
    struct MTINFO_EXPORT SectionError : public ParsingError {
        SectionError (const std::string_view& msg)
          : ParsingError ("Error while parsing " + std::string (msg)
                            + " section",
                          0)
        {
        }
    };

    // error that happends if the parser reaches end of file before parsing
    // enough information
    struct MTINFO_EXPORT EOFError : public ParsingError {
        EOFError (const std::string_view& section)
          : ParsingError ("reached EOF while parsing " + std::string (section),
                          0)
        {
        }
    };
} // namespace mtinfo::error

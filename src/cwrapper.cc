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

#include "mtinfo/mtinfo.h"
#include "mtinfo/mtinfo.hh"

extern "C" {
Terminfo*
terminfo_from_file (char* path)
{
    return reinterpret_cast<Terminfo*> (
      new mtinfo::Terminfo (mtinfo::parse_terminfo_file (path)));
}

Terminfo*
terminfo_from_buffer (int8_t* buffer, size_t length)
{
    return reinterpret_cast<Terminfo*> (
      new mtinfo::Terminfo (mtinfo::parse_terminfo (buffer, length)));
}

void
terminfo_destroy (Terminfo* terminfo)
{
    delete reinterpret_cast<mtinfo::Terminfo*> (terminfo);
}
}

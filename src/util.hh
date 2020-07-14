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

#include <optional>
#include <vector>

// unpacks a container<std::optional<T>> to container<T>, returns success, src
// will be empty afterwards
template <template <class...> class C, class T>
bool
unpack_optional (C<std::optional<T>>& src, C<T>& dest)
{
    dest.resize (src.size());

    for (size_t i = 0; i < src.size(); i++) {
        if (!src[i].has_value())
            return false;

        dest[i] = std::move (*src[i]);
    }

    src.clear();

    return true;
}

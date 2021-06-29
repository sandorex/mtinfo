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

#pragma once

#include <type_traits>
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <stdexcept>
#include <vector>
#include <string>
#include <cassert>

namespace mtinfo::terminfo::parser {
    class ByteIterator {
    public:
        const int8_t* start;
        size_t length;
        size_t position;

        ByteIterator()
            : start(nullptr),
              length(0),
              position(0)
        {}

        ByteIterator(const int8_t* start, size_t length, size_t position = 0)
            : start(start),
              length(length),
              position(position)
        {}

        int8_t operator*() {
            assert(position <= length);
            assert(start != nullptr);

            return *(start + position);
        }

        const int8_t* operator&() const {
            assert(position <= length);
            assert(start != nullptr);

            return start + position;
        }

        ByteIterator operator+(size_t a) {
            return ByteIterator(this->start, this->length, this->position + a);
        }

        // postfix
        ByteIterator operator++(int) {
            return ByteIterator(this->start, this->length, this->position++);
        }

        ByteIterator& operator++() {
            ++this->position;
            return *this;
        }

        ByteIterator& operator+=(size_t a) {
            this->position += a;
            return *this;
        }

        ByteIterator operator-(size_t a) {
            return ByteIterator(this->start, this->length, this->position - a);
        }

        // postfix
        ByteIterator operator--(int) {
            return ByteIterator(this->start, this->length, this->position--);
        }

        ByteIterator& operator--() {
            --this->position;
            return *this;
        }

        ByteIterator& operator-=(size_t a) {
            this->position -= a;
            return *this;
        }

        // helper functions
        template <typename K = int8_t>
        bool is_out_of_bounds(size_t amount = 0) const {
            return position + (sizeof(K) * amount) > length;
        }

        int8_t i8() {
            return *(*this)++;
        }

        int16_t i16() {
            // don't know why casting fucked me over here, but it did..
            const auto a = static_cast<uint8_t>(this->i8());
            const auto b = static_cast<uint8_t>(this->i8());

            return static_cast<int16_t> ((b << 8) | a);
        }

        int32_t i32() {
            const auto a = static_cast<uint16_t>(this->i16());
            const auto b = static_cast<uint16_t>(this->i16());

            return static_cast<int32_t> ((b << 16) | a);
        }

        ByteIterator& align_short_boundary() {
            if (position % 2 > 0) {
                const auto padding = i8();

                // this prevented so much headache it's unbelievable..
                // padding should always be 0x0
                assert(padding == 0x0);
            }

            return *this;
        }
    };

    // splits string by a delimiter
    std::vector<std::string> split_string (std::string input, const std::string_view& delimiter);
}

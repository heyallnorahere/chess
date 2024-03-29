/*
   Copyright 2022-2023 Nora Beda

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#pragma once

namespace libchess {
    // on a chess board, (0, 0) maps to a1, and (7, 7) maps to h8
    struct coord {
        int32_t x, y;

        coord() { x = y = 0; }

        coord(int32_t _x, int32_t _y) {
            x = _x;
            y = _y;
        }

        int32_t taxicab_length() const { return std::abs(x) + std::abs(y); }

        bool operator==(const coord& other) const { return x == other.x && y == other.y; }
        bool operator!=(const coord& other) const { return x != other.x || y != other.y; }

        coord operator+(const coord& other) const { return coord(x + other.x, y + other.y); }
        coord& operator+=(const coord& other) { return *this = *this + other; }

        coord operator-() const { return coord(-x, -y); }
        coord operator-(const coord& other) const { return *this + -other; }
        coord& operator-=(const coord& other) { return *this = *this - other; }

        coord operator*(const coord& other) const { return coord(x * other.x, y * other.y); }
        coord& operator*=(const coord& other) { return *this = *this * other; }
        
        coord operator*(int32_t scalar) const { return coord(x * scalar, y * scalar); }
        coord& operator*=(int32_t scalar) { return *this = *this * scalar; }
    };
} // namespace libchess

namespace std {
    template <>
    struct hash<libchess::coord> {
        size_t operator()(const libchess::coord& c) const {
            std::hash<int32_t> hasher;
            return hasher(c.x) ^ (hasher(c.y) << 1);
        }
    };
} // namespace std
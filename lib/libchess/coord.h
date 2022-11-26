#pragma once

namespace libchess {
    // (0, 0) maps to a1, (7, 7) maps to h8
    struct coord {
        int32_t x, y;

        coord() { x = y = 0; }

        coord(int32_t _x, int32_t _y) {
            x = _x;
            y = _y;
        }

        bool operator==(const coord& other) const { return x == other.x && y == other.y; }
        bool operator!=(const coord& other) const { return x != other.x || y != other.y; }

        coord operator+(const coord& other) const { return coord(x + other.x, y + other.y); }
        coord& operator+=(const coord& other) { return *this = *this + other; }

        coord operator-() const { return coord(-x, -y); }
        coord operator-(const coord& other) const { return *this + -other; }
        coord& operator-=(const coord& other) { return *this = *this - other; }
    };
}; // namespace libchess
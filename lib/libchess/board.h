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
#include "coord.h"

namespace libchess {
    enum class piece_type : uint8_t { none = 0, king, queen, rook, knight, bishop, pawn };
    enum class player_color : uint8_t { white = 0, black };

    enum castle_side : uint8_t {
        castle_side_none = 0,
        castle_side_king = (1 << 0),
        castle_side_queen = (1 << 1)
    };

    struct piece_info_t {
        piece_type type;
        player_color color;
    };

    class board : public std::enable_shared_from_this<board> {
    public:
        static constexpr size_t width = 8;
        static constexpr size_t size = width * width;

        struct data_t {
            std::array<piece_info_t, size> pieces;
            player_color current_turn;
            std::unordered_map<player_color, uint8_t> player_castling_availability;
            std::optional<coord> en_passant_target;
            uint64_t halfmove_clock, fullmove_count;
        };

        static std::shared_ptr<board> create();
        static std::shared_ptr<board> create(const data_t& data);
        static std::shared_ptr<board> copy(std::shared_ptr<board> existing);

        static std::shared_ptr<board> create(const std::string& fen);
        static std::shared_ptr<board> create_default();

        static size_t get_index(const coord& pos);
        static bool is_out_of_bounds(const coord& pos);

        ~board() = default;

        board(const board&) = delete;
        board& operator=(const board&) = delete;

        bool get_piece(const coord& pos, piece_info_t* piece);
        bool set_piece(const coord& pos, const piece_info_t& piece);

        data_t& get_data() { return m_data; }
        std::string serialize();

    private:
        board() = default;

        data_t m_data;
    };
} // namespace libchess
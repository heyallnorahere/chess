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
#include "board.h"
#include "coord.h"

namespace libchess {
    struct move_t {
        coord position, destination;
    };

    struct piece_query_t {
        std::optional<piece_type> type;
        std::optional<player_color> color;
        std::optional<int32_t> x, y;

        bool (*filter)(const piece_info_t&, void*) = nullptr;
        void* filter_data = nullptr;
    };

    using piece_capture_callback_t = void (*)(const piece_info_t&, void*);
    class engine {
    public:
        engine() = default;
        ~engine() = default;

        engine(std::shared_ptr<board> _board) { set_board(_board); }

        engine(const engine&) = delete;
        engine& operator=(const engine&) = delete;

        void set_board(std::shared_ptr<board> _board);
        std::shared_ptr<board> get_board() const { return m_board; }

        void set_callback_data(void* data);
        piece_capture_callback_t set_capture_callback(piece_capture_callback_t callback);

        operator bool() const { return m_board_data != nullptr; }

        void find_pieces(const piece_query_t& query, std::vector<coord>& positions);
        bool compute_check(player_color color, std::vector<coord>& pieces);
        bool compute_checkmate(player_color color);

        bool compute_legal_moves(const coord& pos, std::list<coord>& destinations);
        bool is_move_legal(const move_t& move);
        bool commit_move(const move_t& move, bool check_legality = true, bool advance_turn = true);

        void clear_cache();

    private:
        void compute_check_internal(player_color color, const std::vector<coord>& kings,
                                    std::vector<coord>& pieces);

        std::shared_ptr<board> m_board;
        board::data_t* m_board_data = nullptr; // convenience

        std::unordered_map<std::string, std::list<coord>> m_legal_move_cache;
        std::unordered_map<player_color, std::vector<coord>> m_checking_pieces_cache;
        std::optional<bool> m_checkmate_cache;

        void* m_callback_data = nullptr;
        piece_capture_callback_t m_capture_callback = nullptr;
    };
} // namespace libchess
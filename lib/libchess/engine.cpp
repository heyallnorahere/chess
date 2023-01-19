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

#include "libchesspch.h"
#include "engine.h"
#include "util.h"

namespace libchess {
    void engine::set_board(std::shared_ptr<board> _board) {
        if (m_board != _board) {
            clear_cache();
            m_board = _board;

            // should, in theory, return nullptr if m_board is nullptr as well, but just to be safe
            if (m_board) {
                m_board_data = &m_board->get_data();
            } else {
                m_board_data = nullptr;
            }
        }
    }

    void engine::set_callback_data(void* data) { m_callback_data = data; }

    piece_capture_callback_t engine::set_capture_callback(piece_capture_callback_t callback) {
        auto previous = m_capture_callback;
        m_capture_callback = callback;
        return previous;
    }

    void engine::find_pieces(const piece_query_t& query, std::vector<coord>& positions) {
        positions.clear();

        for (int32_t y = 0; y < board::width; y++) {
            for (int32_t x = 0; x < board::width; x++) {
                auto pos = coord(x, y);

                piece_info_t piece;
                if (!m_board->get_piece(pos, &piece)) {
                    continue;
                }

                if (query.type.has_value()) {
                    if (piece.type != query.type.value()) {
                        continue;
                    }
                }

                if (query.color.has_value() && piece.color != query.color.value()) {
                    continue;
                }

                if (query.x.has_value() && query.x != x) {
                    continue;
                }

                if (query.y.has_value() && query.y != y) {
                    continue;
                }

                if (query.filter != nullptr && !(query.filter(piece, query.filter_data))) {
                    continue;
                }

                positions.push_back(pos);
            }
        }
    }

    bool engine::compute_check(player_color color, std::vector<coord>& pieces) {
        pieces.clear();
        if (m_checking_pieces_cache.find(color) != m_checking_pieces_cache.end()) {
            const auto& checking_pieces = m_checking_pieces_cache.at(color);
            pieces.insert(pieces.end(), checking_pieces.begin(), checking_pieces.end());

            return !pieces.empty();
        }

        piece_query_t query;
        query.type = piece_type::king;
        query.color = color;

        std::vector<coord> kings;
        find_pieces(query, kings);

        if (kings.size() > 0) {
            compute_check_internal(color, kings, pieces);
        }

        m_checking_pieces_cache.insert(std::make_pair(color, pieces));
        return !pieces.empty();
    }

    bool engine::compute_checkmate(player_color color) {
        if (color != m_board_data->current_turn) {
            return false;
        }

        if (m_checkmate_cache.has_value()) {
            return m_checkmate_cache.value();
        }

        piece_query_t query;
        query.color = color;

        std::vector<coord> pieces;
        find_pieces(query, pieces);

        bool checkmate = true;
        for (const auto& piece : pieces) {
            std::list<coord> legal_moves;
            compute_legal_moves(piece, legal_moves);

            if (!legal_moves.empty()) {
                checkmate = false;
                break;
            }
        }

        m_checkmate_cache = checkmate;
        return checkmate;
    }

    enum piece_movement_type : uint32_t {
        piece_movement_type_unique = 0,
        piece_movement_type_rook = (1 << 0),
        piece_movement_type_bishop = (1 << 1)
    };

    // spaghetti-esque
    // yknow what no it *is* spaghetti code this shit is awful
    // i am never touching this again
    bool engine::compute_legal_moves(const coord& pos, std::list<coord>& destinations) {
        destinations.clear();

        std::string serialized = util::serialize_coordinate(pos);
        if (m_legal_move_cache.find(serialized) != m_legal_move_cache.end()) {
            const auto& moves = m_legal_move_cache.at(serialized);
            destinations.insert(destinations.end(), moves.begin(), moves.end());

            return true;
        }

        piece_info_t piece;
        if (!m_board->get_piece(pos, &piece)) {
            return false;
        }

        static const std::vector<int32_t> direction_factors = { 1, -1 };
        uint32_t movement_type = piece_movement_type_unique;

        switch (piece.type) {
        case piece_type::king: {
            static const std::vector<int32_t> king_direction_factors = { 1, 0, -1 };
            for (size_t i = 0; i < king_direction_factors.size(); i++) {
                int32_t x_factor = king_direction_factors[i];

                for (size_t j = 0; j < king_direction_factors.size(); j++) {
                    int32_t y_factor = king_direction_factors[j];

                    coord dst = pos + coord(x_factor, y_factor);
                    if (dst.taxicab_length() == 0 || board::is_out_of_bounds(dst)) {
                        continue;
                    }

                    piece_info_t temp;
                    if (m_board->get_piece(dst, &temp) && temp.color == piece.color) {
                        continue;
                    }

                    destinations.push_back(dst);
                }
            }

            uint8_t castling_flags = m_board_data->player_castling_availability.at(piece.color);
            std::vector<std::tuple<int32_t, int32_t>> castling_directions;

            if ((castling_flags & castle_side_queen) != castle_side_none) {
                castling_directions.push_back(std::make_tuple(-1, -1));
            }

            if ((castling_flags & castle_side_king) != castle_side_none) {
                castling_directions.push_back(std::make_tuple(1, (int32_t)board::width));
            }

            for (auto [direction, limit] : castling_directions) {
                bool castling_valid = false;
                for (int32_t x = pos.x + direction; x != limit; x += direction) {
                    piece_info_t current_piece;
                    if (m_board->get_piece(coord(x, pos.y), &current_piece)) {
                        if (current_piece.type == piece_type::rook) {
                            castling_valid = true;
                        } else {
                            castling_valid = false;
                            break;
                        }
                    }
                }

                if (castling_valid) {
                    coord dst = pos + coord(direction * 2, 0);

                    std::vector<coord> king_positions;
                    for (int32_t x = pos.x; x != dst.x + direction; x += direction) {
                        king_positions.push_back(coord(x, pos.y));
                    }

                    std::vector<coord> checking_pieces;
                    if (piece.color == m_board_data->current_turn) {
                        compute_check_internal(piece.color, king_positions, checking_pieces);
                    }

                    if (checking_pieces.empty()) {
                        destinations.push_back(dst);
                    }
                }
            }
        } break;
        case piece_type::queen:
            movement_type = piece_movement_type_rook | piece_movement_type_bishop;
            break;
        case piece_type::rook:
            movement_type = piece_movement_type_rook;
            break;
        case piece_type::knight: {
            static const std::vector<coord> knight_offsets = { coord(2, 1), coord(1, 2) };
            for (size_t i = 0; i < direction_factors.size(); i++) {
                int32_t x_factor = direction_factors[i];

                for (size_t j = 0; j < direction_factors.size(); j++) {
                    int32_t y_factor = direction_factors[j];
                    coord direction = coord(x_factor, y_factor);

                    for (size_t k = 0; k < knight_offsets.size(); k++) {
                        coord offset = knight_offsets[k];
                        coord destination = pos + (direction * offset);

                        if (board::is_out_of_bounds(destination)) {
                            continue;
                        }

                        piece_info_t temp_piece;
                        if (m_board->get_piece(destination, &temp_piece) &&
                            temp_piece.color == piece.color) {
                            continue;
                        }

                        destinations.push_back(destination);
                    }
                }
            }
        } break;
        case piece_type::bishop:
            movement_type = piece_movement_type_bishop;
            break;
        case piece_type::pawn: { // impossible for a pawn to move out of bounds i think
            int32_t step_direction = piece.color == player_color::white ? 1 : -1;
            coord step = coord(0, step_direction);

            coord single_step = pos + step;
            if (!m_board->get_piece(single_step, nullptr)) {
                destinations.push_back(single_step);

                int32_t starting_y = piece.color == player_color::white ? 1 : (board::width - 2);
                coord double_step = single_step + step;

                if (pos.y == starting_y && !m_board->get_piece(double_step, nullptr)) {
                    destinations.push_back(double_step);
                }
            }

            piece_info_t temp_piece;
            for (size_t i = 0; i < direction_factors.size(); i++) {
                int32_t capture_direction = direction_factors[i];
                coord capture_step = single_step + coord(capture_direction, 0);

                if ((m_board->get_piece(capture_step, &temp_piece) &&
                     temp_piece.color != piece.color) ||
                    m_board_data->en_passant_target == capture_step) {
                    destinations.push_back(capture_step);
                }
            }
        } break;
        default:
            return false;
        }

        // this part especially is awful
        {
            std::vector<coord> movement_directions;
            if ((movement_type & piece_movement_type_rook) != piece_movement_type_unique) {
                for (size_t i = 0; i < direction_factors.size(); i++) {
                    int32_t factor = direction_factors[i];
                    movement_directions.insert(movement_directions.end(),
                                               { coord(factor, 0), coord(0, factor) });
                }
            }

            if ((movement_type & piece_movement_type_bishop) != piece_movement_type_unique) {
                for (size_t i = 0; i < direction_factors.size(); i++) {
                    int32_t x_factor = direction_factors[i];

                    for (size_t j = 0; j < direction_factors.size(); j++) {
                        int32_t y_factor = direction_factors[j];
                        movement_directions.push_back(coord(x_factor, y_factor));
                    }
                }
            }

            for (const auto& direction : movement_directions) {
                coord current_pos = pos;
                while (true) {
                    current_pos += direction;
                    if (board::is_out_of_bounds(current_pos)) {
                        break;
                    }

                    piece_info_t current_piece;
                    if (m_board->get_piece(current_pos, &current_piece)) {
                        if (current_piece.color != piece.color) {
                            destinations.push_back(current_pos);
                        }

                        break;
                    } else {
                        destinations.push_back(current_pos);
                    }
                }
            }
        }

        if (piece.color == m_board_data->current_turn) {
            move_t move;
            move.position = pos;

            engine temp_engine;
            auto it = destinations.begin();

            while (it != destinations.end()) {
                move.destination = *it;

                piece_info_t temp_piece;
                if (m_board->get_piece(move.destination, &temp_piece) &&
                    temp_piece.type == piece_type::king) {
                    it++;
                    continue;
                }

                auto new_board = board::copy(m_board);
                temp_engine.set_board(new_board);
                temp_engine.commit_move(move, false, false);

                std::vector<coord> pieces;
                temp_engine.compute_check(piece.color, pieces);

                if (pieces.size() > 0) {
                    it = destinations.erase(it);
                } else {
                    it++;
                }
            }
        }

        m_legal_move_cache.insert(std::make_pair(serialized, destinations));
        return true;
    }

    bool engine::is_move_legal(const move_t& move) {
        std::list<coord> legal_moves;
        if (!compute_legal_moves(move.position, legal_moves)) {
            return false;
        }

        return std::find(legal_moves.begin(), legal_moves.end(), move.destination) !=
               legal_moves.end();
    }

    bool engine::commit_move(const move_t& move, bool check_legality, bool advance_turn) {
        piece_info_t piece;
        if (!m_board->get_piece(move.position, &piece) ||
            board::is_out_of_bounds(move.destination)) {
            return false;
        }

        if (check_legality && !is_move_legal(move)) {
            return false;
        }

        bool reset_halfmove_clock = false;
        if (piece.type == piece_type::pawn) {
            reset_halfmove_clock = true;
        }

        coord capture_position;
        if (piece.type == piece_type::pawn && m_board_data->en_passant_target == move.destination) {
            capture_position = coord(move.destination.x, move.position.y);
        } else {
            capture_position = move.destination;
        }

        piece_info_t captured;
        if (m_board->get_piece(capture_position, &captured)) {
            if (m_capture_callback != nullptr) {
                m_capture_callback(captured, m_callback_data);
            }

            m_board->set_piece(capture_position, { piece_type::none });
            reset_halfmove_clock = true;
        }

        clear_cache();
        m_board->set_piece(move.position, { piece_type::none });
        m_board->set_piece(move.destination, piece);

        coord delta = move.destination - move.position;
        if (piece.type == piece_type::pawn && std::abs(delta.y) == 2) {
            m_board_data->en_passant_target = move.position + coord(0, delta.y / 2);
        } else {
            m_board_data->en_passant_target.reset();
        }

        if (piece.type == piece_type::king && std::abs(delta.x) == 2) {
            m_board_data->player_castling_availability[piece.color] = castle_side_none;

            int32_t direction = delta.x / std::abs(delta.x);
            int32_t rook_x = (direction > 0) ? ((int32_t)board::width - 1) : 0;
            auto rook_pos = coord(rook_x, move.position.y);

            piece_info_t rook;
            m_board->get_piece(rook_pos, &rook);

            m_board->set_piece(rook_pos, { piece_type::none });
            m_board->set_piece(coord(move.destination.x - direction, move.destination.y), rook);
        }

        if (piece.type == piece_type::rook) {
            static const std::unordered_map<castle_side, int32_t> starting_rook_positions = {
                { castle_side::castle_side_queen, 0 },
                { castle_side::castle_side_king, (int32_t)board::width - 1 }
            };

            int32_t y = piece.color == player_color::white ? 0 : (board::width - 1);
            for (auto [side, x] : starting_rook_positions) {
                if (move.position == coord(x, y)) {
                    m_board_data->player_castling_availability[piece.color] &= ~side;
                    break;
                }
            }
        }

        // a little spaghetti-y
        if (advance_turn) {
            if (reset_halfmove_clock) {
                m_board_data->halfmove_clock = 0;
            } else {
                m_board_data->halfmove_clock++;
            }

            if (m_board_data->current_turn == player_color::white) {
                m_board_data->current_turn = player_color::black;
            } else {
                m_board_data->current_turn = player_color::white;
                m_board_data->fullmove_count++;
            }
        }

        return true;
    }

    void engine::clear_cache() {
        m_legal_move_cache.clear();
        m_checking_pieces_cache.clear();
        m_checkmate_cache.reset();

        // todo: clear caches as they're added
    }

    bool engine::get_piece(const coord& pos, piece_info_t* piece) const {
        return m_board->get_piece(pos, piece);
    }

    bool engine::set_piece(const coord& pos, const piece_info_t& piece) const {
        return m_board->set_piece(pos, piece);
    }

    std::string engine::serialize_board() const { return m_board->serialize(); }
    player_color engine::get_current_turn() const { return m_board_data->current_turn; }

    uint8_t engine::get_player_castling_availability(player_color player) const {
        return m_board_data->player_castling_availability.at(player);
    }

    const std::optional<coord>& engine::get_en_passant_target() const {
        return m_board_data->en_passant_target;
    }

    uint64_t engine::get_halfmove_clock() const { return m_board_data->halfmove_clock; }
    uint64_t engine::get_fullmove_count() const { return m_board_data->fullmove_count; }

    void engine::compute_check_internal(player_color color, const std::vector<coord>& kings,
                                        std::vector<coord>& pieces) {
        player_color opposing =
            color != player_color::white ? player_color::white : player_color::black;

        piece_query_t query;
        query.color = opposing;

        std::vector<coord> opposing_pieces;
        find_pieces(query, opposing_pieces);

        for (const auto& piece_pos : opposing_pieces) {
            std::list<coord> legal_moves;
            compute_legal_moves(piece_pos, legal_moves);

            for (const auto& move : legal_moves) {
                if (std::find(kings.begin(), kings.end(), move) != kings.end()) {
                    pieces.push_back(piece_pos);
                }
            }
        }
    }
} // namespace libchess
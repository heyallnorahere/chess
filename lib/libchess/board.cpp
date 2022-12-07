/*
   Copyright 2022 Nora Beda

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
#include "board.h"
#include "util.h"

namespace libchess {
    static bool parse_fen_string_pieces(const std::string& pieces, board::data_t& result) {
        std::vector<std::string> ranks;
        util::split_string(pieces, '/', ranks, util::string_split_options_omit_empty);

        if (ranks.size() != board::width) {
            return false;
        }

        static const std::regex free_space_regex("[1-8]");
        for (size_t i = 0; i < board::width; i++) {
            int32_t x = 0;
            int32_t y = (int32_t)(board::width - (i + 1));

            std::string rank_string = ranks[i];
            for (char c : rank_string) {
                // early check
                if (x >= board::width) {
                    return false;
                }

                // spaghetti-esque string creation & parsing
                std::string character_string(&c, 1);
                if (std::regex_match(character_string, free_space_regex)) {
                    int32_t count = std::atoi(character_string.c_str());

                    for (int32_t j = 0; j < count; j++) {
                        coord pos(x++, y);

                        // later check
                        if (pos.x >= board::width) {
                            return false;
                        }

                        size_t index = board::get_index(pos);
                        result.pieces[index] = { piece_type::none };
                    }
                } else {
                    piece_info_t temp_piece;
                    if (util::parse_piece(c, temp_piece)) {
                        coord pos(x++, y);
                        size_t index = board::get_index(pos);
                        result.pieces[index] = temp_piece;
                    } else {
                        return false; // invalid character
                    }
                }
            }

            if (x < board::width) {
                return false; // rank's too narrow
            }
        }

        return true;
    }

    static bool parse_fen_string(const std::string& fen, board::data_t& result) {
        std::vector<std::string> segments;
        util::split_string(fen, ' ', segments, util::string_split_options_omit_empty);

        // 6 total segments
        if (segments.size() != 6) {
            return false;
        }

        // first up is the pieces
        if (!parse_fen_string_pieces(segments[0], result)) {
            return false;
        }

        // next is current turn
        std::string turn_segments = segments[1];

        // segment should only be 1 character long
        if (turn_segments.length() != 1) {
            return false;
        }

        switch (turn_segments[0]) {
        case 'w':
            result.current_turn = player_color::white;
            break;
        case 'b':
            result.current_turn = player_color::black;
            break;
        default:
            return false;
        }

        // third, castling availability
        std::string castling_segment = segments[2];

        // set defaults
        result.player_castling_availability[player_color::white] =
            result.player_castling_availability[player_color::black] = castling_availability_none;

        if (castling_segment != "-") {
            for (char c : castling_segment) {
                char lower = (char)std::tolower((int)c);
                player_color color = c != lower ? player_color::white : player_color::black;

                castling_availability flag;
                switch (lower) {
                case 'k':
                    flag = castling_availability_king;
                    break;
                case 'q':
                    flag = castling_availability_queen;
                    break;
                default:
                    return false;
                }

                result.player_castling_availability[color] |= flag;
            }
        }

        // fourth, en passant target
        std::string en_passant_segment = segments[3];
        if (en_passant_segment != "-") {
            coord target;
            if (!util::parse_coordinate(en_passant_segment, target)) {
                return false;
            }

            result.en_passant_target = target;
        } else {
            result.en_passant_target.reset();
        }

        // lastly, counters
        std::string halfmove_clock_segment = segments[4];
        std::string fullmove_count_segment = segments[5];

        std::regex counter_regex("[0-9]+");
        if (!std::regex_match(halfmove_clock_segment, counter_regex) ||
            !std::regex_match(fullmove_count_segment, counter_regex)) {
            return false;
        }

        result.halfmove_clock = (uint64_t)std::stoull(halfmove_clock_segment);
        result.fullmove_count = (uint64_t)std::stoull(fullmove_count_segment);

        return true;
    }

    std::shared_ptr<board> board::create() {
        auto _board = new board;

        _board->m_data.halfmove_clock = 0;
        _board->m_data.fullmove_count = 1;

        for (size_t i = 0; i < size; i++) {
            _board->m_data.pieces[i] = { piece_type::none };
        }

        _board->m_data.player_castling_availability[player_color::white] =
            _board->m_data.player_castling_availability[player_color::black] =
                castling_availability_king | castling_availability_queen;

        return std::shared_ptr<board>(_board);
    }

    std::shared_ptr<board> board::create(const data_t& data) {
        auto _board = new board;
        _board->m_data = data; // a lazy copy should be fine

        return std::shared_ptr<board>(_board);
    }

    std::shared_ptr<board> board::copy(std::shared_ptr<board> existing) {
        std::shared_ptr<board> result;
        if (existing) {
            result = create(existing->m_data);
        }

        return result;
    }

    std::shared_ptr<board> board::create(const std::string& fen) {
        auto _board = std::shared_ptr<board>(new board);
        if (!parse_fen_string(fen, _board->m_data)) {
            _board.reset();
        }

        return _board;
    }

    std::shared_ptr<board> board::create_default() {
        // fen string for the default board configuration
        return create("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    }

    size_t board::get_index(const coord& pos) {
        // inverting y because of memory layout
        // ranks are laid out 8-1, one after another

        size_t x = (size_t)pos.x;
        size_t y = width - ((size_t)pos.y + 1);

        return (y * width) + x;
    }

    bool board::is_out_of_bounds(const coord& pos) {
        return pos.x < 0 || pos.x >= width || pos.y < 0 || pos.y >= width;
    }

    bool board::get_piece(const coord& pos, piece_info_t* piece) {
        if (is_out_of_bounds(pos)) {
            if (piece != nullptr) {
                piece->type = piece_type::none;
            }

            return false;
        } else {
            size_t index = get_index(pos);
            const auto& data = m_data.pieces[index];

            if (piece != nullptr) {
                *piece = data;
            }

            return data.type != piece_type::none;
        }
    }

    bool board::set_piece(const coord& pos, const piece_info_t& piece) {
        if (is_out_of_bounds(pos)) {
            return false;
        }

        size_t index = get_index(pos);
        m_data.pieces[index] = piece;

        return true;
    }

    std::string board::serialize() {
        std::stringstream fen;

        piece_info_t piece;
        for (size_t i = 0; i < board::width; i++) {
            if (i > 0) {
                fen << "/";
            }

            uint32_t free_spaces = 0;
            int32_t y = board::width - (i + 1);
            for (int32_t x = 0; x < board::width; x++) {
                auto pos = coord(x, y);

                if (!get_piece(pos, &piece)) {
                    free_spaces++;
                    continue;
                }

                if (free_spaces > 0) {
                    fen << free_spaces;
                    free_spaces = 0;
                }

                auto serialized_piece = util::serialize_piece(piece);
                if (!serialized_piece.has_value()) {
                    throw std::runtime_error("invalid piece type!");
                }

                fen << serialized_piece.value();
            }

            // reused code - though what can you do
            if (free_spaces > 0) {
                fen << free_spaces;
            }
        }

        char current_turn;
        switch (m_data.current_turn) {
        case player_color::white:
            current_turn = 'w';
            break;
        case player_color::black:
            current_turn = 'b';
            break;
        default:
            throw std::runtime_error("invalid current turn!");
        }

        fen << ' ' << current_turn << ' ';
        {
            std::stringstream castling_availability_stream;
            for (auto color : { player_color::white, player_color::black }) {
                std::vector<piece_type> available_sides;
                uint8_t availability = m_data.player_castling_availability.at(color);

                if ((availability & castling_availability_king) != castling_availability_none) {
                    available_sides.push_back(piece_type::king);
                }

                if ((availability & castling_availability_queen) != castling_availability_none) {
                    available_sides.push_back(piece_type::queen);
                }

                for (auto side : available_sides) {
                    piece_info_t piece_desc;
                    piece_desc.type = side;
                    piece_desc.color = color;

                    castling_availability_stream << util::serialize_piece(piece_desc).value();
                }
            }

            std::string castling_availability_desc = castling_availability_stream.str();
            if (castling_availability_desc.empty()) {
                fen << '-';
            } else {
                fen << castling_availability_desc;
            }
        }

        fen << ' ';
        if (m_data.en_passant_target.has_value()) {
            fen << util::serialize_coordinate(m_data.en_passant_target.value());
        } else {
            fen << '-';
        }

        fen << ' ' << m_data.halfmove_clock;
        fen << ' ' << m_data.fullmove_count;

        return fen.str();
    }
} // namespace libchess
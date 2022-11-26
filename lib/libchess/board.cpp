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
        static const std::unordered_map<char, piece_type> piece_characters = {
            { 'r', piece_type::rook },  { 'n', piece_type::knight }, { 'b', piece_type::bishop },
            { 'q', piece_type::queen }, { 'k', piece_type::king },   { 'p', piece_type::pawn }
        };

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
                    // character shenanigans - does this character represent a piece?
                    char lower = (char)std::tolower((int)c);
                    if (piece_characters.find(lower) != piece_characters.end()) {

                        piece_type type = piece_characters.at(lower);
                        player_color color = c != lower ? player_color::white : player_color::black;

                        coord pos(x++, y);
                        size_t index = board::get_index(pos);
                        result.pieces[index] = { type, color };
                    } else {
                        return false; // invalid character
                    }
                }
            }

            if (x < board::width) {
                return false; // rank to narrow
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
        for (size_t i = 0; i < size; i++) {
            _board->m_data.pieces[i] = { piece_type::none };
        }

        return std::shared_ptr<board>(_board);
    }

    std::shared_ptr<board> board::create(const data_t& data) {
        auto _board = new board;
        _board->m_data = data; // a lazy copy should be fine

        return std::shared_ptr<board>(_board);
    }

    std::shared_ptr<board> board::create(const std::string& fen) {
        auto _board = create();
        if (!parse_fen_string(fen, _board->m_data)) {
            _board.reset();
        }

        return _board;
    }

    std::shared_ptr<board> board::create_default() {
        // fen string for the default board configuration
        static const std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

        return create(fen);
    }

    size_t board::get_index(const coord& pos) {
        // inverting y because of memory layout
        // ranks are laid out 8-1, one after another

        size_t x = (size_t)pos.x;
        size_t y = board::width - ((size_t)pos.y + 1);

        return (y * board::width) + x;
    }

    const piece_info_t& board::get_piece(const coord& pos) {
        size_t index = get_index(pos);
        return m_data.pieces[index];
    }

    void board::set_piece(const coord& pos, const piece_info_t& piece) {
        size_t index = get_index(pos);
        m_data.pieces[index] = piece;
    }

    board::board() {
        m_data.player_castling_availability[player_color::white] =
            m_data.player_castling_availability[player_color::black] =
                castling_availability_king | castling_availability_queen;
    }
} // namespace libchess
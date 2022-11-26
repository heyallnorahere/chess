#pragma once
#include "coord.h"

namespace libchess {
    enum class piece_type : uint8_t { none = 0, king, queen, rook, knight, bishop, pawn };
    enum class player_color : uint8_t { white, black };

    enum castling_availability : uint8_t {
        castling_availability_none = 0,
        castling_availability_king = (1 << 0),
        castling_availability_queen = (1 << 1)
    };

    struct piece_info_t {
        piece_type type;
        player_color color;
    };

    class board : std::enable_shared_from_this<board> {
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

        static std::shared_ptr<board> create(const std::string& fen);
        static std::shared_ptr<board> create_default();

        static size_t get_index(const coord& pos);

        ~board() = default;

        board(const board&) = delete;
        board& operator=(const board&) = delete;

        const piece_info_t& get_piece(const coord& pos);
        void set_piece(const coord& pos, const piece_info_t& piece);

        data_t& get_data() { return m_data; }

    private:
        board();

        data_t m_data;
    };
} // namespace libchess
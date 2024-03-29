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

#include <testbed.h>
#include <libchess.h>

static bool parse_move(const std::string& desc, libchess::move_t& move) {
    std::vector<std::string> segments;
    libchess::util::split_string(desc, ' ', segments,
                                 libchess::util::string_split_options_omit_empty);

    if (segments.size() != 2) {
        return false;
    }

    return libchess::util::parse_coordinate(segments[0], move.position) &&
           libchess::util::parse_coordinate(segments[1], move.destination);
}

class board_position_set {
public:
    board_position_set() = default;
    ~board_position_set() = default;

    board_position_set(const board_position_set&) = delete;
    board_position_set& operator=(const board_position_set&) = delete;

    void set_fen(const std::string& name, const std::string& fen) {
        if (m_fens.find(name) != m_fens.end()) {
            m_fens[name] = fen;
        } else {
            m_fens.insert(std::make_pair(name, fen));
        }
    }

    std::optional<std::string> get_fen(const std::string& name) const {
        std::optional<std::string> result;
        if (m_fens.find(name) != m_fens.end()) {
            result = m_fens.at(name);
        }

        return result;
    }

private:
    std::unordered_map<std::string, std::string> m_fens;
};

static void run_check(const std::vector<std::string>& data, const board_position_set* positions,
                      bool should_pass) {
    std::shared_ptr<libchess::board> board;

    if (data.size() > 1) {
        auto fen = positions->get_fen(data[1]);
        assert::is_true(fen.has_value());

        board = libchess::board::create(fen.value());
    } else {
        board = libchess::board::create_default();
    }

    libchess::move_t move;
    assert::is_true(parse_move(data[0], move));
    assert::is_not_nullptr(board);

    libchess::engine engine(board);
    bool result = engine.is_move_legal(move);

    if (should_pass) {
        assert::is_true(result);
    } else {
        assert::is_false(result);
    }
}

class legal_moves : public test_theory {
public:
    legal_moves(const board_position_set& positions) { m_positions = &positions; }

protected:
    virtual void add_inline_data() override {
        inline_data({ "a2 a3" });
        inline_data({ "a2 a4" });
        inline_data({ "b1 a3" });
        inline_data({ "d5 e6", "en_passant" });
        inline_data({ "e1 g1", "castling" });
        inline_data({ "f1 g1", "check" });
        inline_data({ "b2 a1", "king_move" });
    }

    virtual void invoke(const std::vector<std::string>& data) override {
        run_check(data, m_positions, true);
    }

    virtual std::string get_check_name() override { return "legal_moves"; }

private:
    const board_position_set* m_positions;
};

class illegal_moves : public test_theory {
public:
    illegal_moves(const board_position_set& positions) { m_positions = &positions; }

protected:
    virtual void add_inline_data() override {
        inline_data({ "a2 a5" });
        inline_data({ "c1 b2" });
        inline_data({ "c3 c4", "pawn_pressure" });
        inline_data({ "c3 c5", "pawn_pressure" });
        inline_data({ "d5 e6", "en_passant_illegal" });
        inline_data({ "e1 g1" });
        inline_data({ "e1 g1", "castling_intercepted" });
        inline_data({ "f1 g2", "check" });
        inline_data({ "f2 f4", "check" });
    }

    virtual void invoke(const std::vector<std::string>& data) override {
        run_check(data, m_positions, false);
    }

    virtual std::string get_check_name() override { return "illegal_moves"; }

private:
    const board_position_set* m_positions;
};

class voided_castling_availability : public test_theory {
protected:
    virtual void add_inline_data() override {
        inline_data({ "a1 b1", "w", "k" });
        inline_data({ "h1 g1", "w", "q" });
        inline_data({ "e1 e2", "w", "" });
        inline_data({ "e1 d1", "w", "" });
        inline_data({ "e1 g1", "w", "" });
        inline_data({ "e1 c1", "w", "" });
        inline_data({ "h8 h1", "b", "q" });
    };

    virtual void invoke(const std::vector<std::string>& data) override {
        const std::string& turn = data[1];
        std::string fen = "1k5r/8/8/8/8/8/8/R3K2R " + turn + " KQ - 0 1";

        auto board = libchess::board::create(fen);
        assert::is_not_nullptr(board);

        libchess::move_t move;
        assert::is_true(parse_move(data[0], move));

        uint8_t expected = 0;
        const std::string& expectedCastlingAvailability = data[2];

        for (int i = 0; i < expectedCastlingAvailability.length(); i++) {
            switch (expectedCastlingAvailability[i]) {
            case 'k':
                expected |= libchess::castle_side_king;
                break;
            case 'q':
                expected |= libchess::castle_side_queen;
                break;
            default:
                assert::throw_error();
                break;
            }
        }

        libchess::engine engine(board);
        assert::is_true(engine.commit_move(move));

        auto actual = board->get_data().player_castling_availability[libchess::player_color::white];
        assert::is_equal(actual, expected);
    }

    virtual std::string get_check_name() override { return "voided_castling_availability"; }
};

class checkmate : public test_theory {
protected:
    virtual void add_inline_data() override {
        inline_data({ "y", "k4r2/8/8/8/8/8/3PPq2/3QK3 w - - 0 1" });
        inline_data({ "n", "k4r2/8/8/8/8/8/4Pq2/3QK3 w - - 0 1" });
        inline_data({ "y", "k7/8/8/8/4n3/8/4Pq2/3QK3 w - - 0 1" });
    }

    virtual void invoke(const std::vector<std::string>& data) override {
        auto board = libchess::board::create(data[1]);
        assert::is_not_nullptr(board);

        libchess::engine engine(board);
        bool is_mate = engine.compute_checkmate(board->get_data().current_turn);

        if (data[0] == "y") {
            assert::is_true(is_mate);
        } else {
            assert::is_false(is_mate);
        }
    }

    virtual std::string get_check_name() override { return "checkmate"; }
};

class en_passant : public test_theory {
protected:
    virtual void add_inline_data() override {
        inline_data({ "d5 e6", "k7/8/8/3Pp3/8/8/8/K7 w - e6 0 1" });
        inline_data({ "e5 d6", "k7/8/8/3pP3/8/8/8/K7 w - d6 0 1" });
        inline_data({ "d4 e3", "k7/8/8/8/3pP3/8/8/K7 b - e3 0 1" });
        inline_data({ "e4 d3", "k7/8/8/8/3Pp3/8/8/K7 b - d3 0 1" });
    }

    virtual void invoke(const std::vector<std::string>& data) override {
        auto board = libchess::board::create(data[1]);
        assert::is_not_nullptr(board);

        libchess::move_t move;
        assert::is_true(parse_move(data[0], move));

        libchess::piece_info_t piece;
        assert::is_true(board->get_piece(move.position, &piece));
        assert::is_equal(piece.color, board->get_data().current_turn);

        libchess::engine engine(board);
        assert::is_true(engine.commit_move(move));

        auto taken = libchess::coord(move.destination.x, move.position.y);
        assert::is_false(board->get_piece(taken, nullptr));

        libchess::piece_info_t destination_piece;
        assert::is_true(board->get_piece(move.destination, &destination_piece));

        assert::is_equal(piece.type, destination_piece.type);
        assert::is_equal(piece.color, destination_piece.color);
    }

    virtual std::string get_check_name() override { return "en_passant"; }
};

DEFINE_ENTRYPOINT() {
    board_position_set positions;

    positions.set_fen("check", "k7/8/8/8/8/7q/5P2/5K2 w - - 0 1");
    positions.set_fen("pawn_pressure",
                      "rnb1kbnr/pp1ppppp/2p5/q7/3P4/2P5/PP2PPPP/RNBQKBNR w KQkq - 0 1");

    positions.set_fen("en_passant",
                      "rnbqkbnr/pp1p1ppp/8/2pPp3/8/8/PPP1PPPP/RNBQKBNR w KQkq e6 0 1");

    positions.set_fen("en_passant_illegal",
                      "rnbqkbnr/pppp1ppp/8/3Pp3/8/8/PPP1PPPP/RNBQKBNR w KQkq - 0 1");

    positions.set_fen("castling", "rnbqkbnr/pppppppp/8/8/8/5NP1/PPPPPPBP/RNBQK2R w KQkq - 0 1");
    positions.set_fen("castling_intercepted",
                      "1nbqkbnr/pppppppp/6r1/8/8/8/PPPP4/RNBQK2R w KQkq - 0 1");

    positions.set_fen("castling_unavailable",
                      "rnbqkbnr/pppppppp/8/8/8/5NP1/PPPPPPBP/RNBQK2R w kq - 0 1");

    positions.set_fen("king_move", "6k1/7p/7P/5p2/8/8/pK5r/8 w - - 4 46");

    invoke_check<legal_moves>(positions);
    invoke_check<illegal_moves>(positions);

    invoke_check<voided_castling_availability>();
    invoke_check<checkmate>();
    invoke_check<en_passant>();
}
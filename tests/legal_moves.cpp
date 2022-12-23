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
        inline_data({ "a1 b1", "k" });
        inline_data({ "h1 g1", "q" });
    };

    virtual void invoke(const std::vector<std::string>& data) override {
        auto board = libchess::board::create("1k6/8/8/8/8/8/8/R3K2R w KQ - 0 1");
        assert::is_not_nullptr(board);

        libchess::move_t move;
        assert::is_true(parse_move(data[0], move));

        libchess::castle_side expected;
        switch (data[1][0]) {
        case 'k':
            expected = libchess::castle_side_king;
            break;
        case 'q':
            expected = libchess::castle_side_queen;
            break;
        default:
            assert::throw_error();
            break;
        }

        libchess::engine engine(board);
        assert::is_true(engine.commit_move(move));

        auto actual = board->get_data().player_castling_availability[libchess::player_color::white];
        assert::is_equal(actual, expected);
    }

    virtual std::string get_check_name() override { return "voided_castling_availability"; }
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

    invoke_check<legal_moves>(positions);
    invoke_check<illegal_moves>(positions);
    invoke_check<voided_castling_availability>();
}
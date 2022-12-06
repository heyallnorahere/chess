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

class valid_fen_strings : public test_theory {
public:
    virtual void add_inline_data() override {
        inline_data({ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" });
        inline_data({ "8/8/8/8/8/8/8/8 w - - 0 1" });
        inline_data({ "3qk3/8/8/8/8/8/8/3QK3 w - - 0 1" });
    }

    virtual void invoke(const std::vector<std::string>& data) override {
        std::string original_fen = data[0];

        auto board = libchess::board::create(original_fen);
        assert::is_not_nullptr(board);

        std::string new_fen = board->serialize();
        assert::is_equal(new_fen, original_fen);
    }
};

class invalid_fen_strings : public test_theory {
public:
    virtual void add_inline_data() override {
        inline_data({ "" });
        inline_data({ "8/8/8/8/8/8/8 w - - 0 1" });
        inline_data({ "8/8/8/8/8/8/8/8 f - - 0 1" });
        inline_data({ "8/8/8/8/8/8/8/8 w abAB - 0 1" });
        inline_data({ "8/8/8/8/8/8/8/8 w - i1 0 1" });
        inline_data({ "8/8/8/8/8/8/8/8 w - a9 0 1" });
        inline_data({ "8/8/8/8/8/8/8/8 w - abc 0 1" });
    }

    virtual void invoke(const std::vector<std::string>& data) override {
        auto board = libchess::board::create(data[0]);
        assert::is_nullptr(board);
    }
};

DEFINE_ENTRYPOINT() {
    invoke_check<valid_fen_strings>();
    invoke_check<invalid_fen_strings>();
}
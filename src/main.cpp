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

#include "pch.h"
#include <iostream>

static bool dump_board(std::shared_ptr<libchess::board> board) {
    libchess::piece_info_t piece;
    for (int32_t y = (int32_t)(libchess::board::width - 1); y >= 0; y--) {
        for (int32_t x = 0; x < (int32_t)libchess::board::width; x++) {
            board->get_piece(libchess::coord(x, y), &piece);

            char piece_character;
            switch (piece.type) {
            case libchess::piece_type::none:
                piece_character = ' ';
                break;
            case libchess::piece_type::king:
                piece_character = 'k';
                break;
            case libchess::piece_type::queen:
                piece_character = 'q';
                break;
            case libchess::piece_type::rook:
                piece_character = 'r';
                break;
            case libchess::piece_type::knight:
                piece_character = 'n';
                break;
            case libchess::piece_type::bishop:
                piece_character = 'b';
                break;
            case libchess::piece_type::pawn:
                piece_character = 'p';
                break;
            default:
                return false;
            }

            if (piece.type != libchess::piece_type::none &&
                piece.color == libchess::player_color::white) {
                piece_character +=
                    ('A' - 'a'); // i know it's negative i just think this looks neater
            }

            std::cout << piece_character;
        }

        std::cout << std::endl;
    }

    return true;
}

int main(int argc, const char** argv) {
    std::shared_ptr<libchess::board> board;
    if (argc > 1) {
        board = libchess::board::create(argv[1]);
    } else {
        board = libchess::board::create_default();
    }

    if (!board) {
        return 1;
    }

    return dump_board(board) ? 0 : 1;
}
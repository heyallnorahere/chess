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

static void on_capture(const libchess::piece_info_t& piece, void* data) {
    std::string piece_name;
    switch (piece.type) {
    case libchess::piece_type::king:
        piece_name = "king";
        break;
    case libchess::piece_type::queen:
        piece_name = "queen";
        break;
    case libchess::piece_type::rook:
        piece_name = "rook";
        break;
    case libchess::piece_type::knight:
        piece_name = "knight";
        break;
    case libchess::piece_type::bishop:
        piece_name = "bishop";
        break;
    case libchess::piece_type::pawn:
        piece_name = "pawn";
        break;
    default:
        return;
    }

    std::string color_name = piece.color == libchess::player_color::white ? "white" : "black";
    std::cout << "piece captured\ntype: " << piece_name << "\ncolor: " << color_name << std::endl;
}

int main(int argc, const char** argv) {
    auto board =
        libchess::board::create("rnbqkbnr/pppppppp/8/8/8/8/PPPPpPPP/RNBQKBNR w KQkq - 0 1");
    if (!board) {
        return 1;
    }

    libchess::engine engine(board);
    engine.set_capture_callback(on_capture);

    libchess::move_t move;
    move.position = libchess::coord(6, 0);
    move.destination = libchess::coord(4, 1);

    if (!engine.commit_move(move)) {
        return 1;
    }

    return 0;
}
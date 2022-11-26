#include "pch.h"

#include <iostream>

int main(int argc, const char** argv) {
    auto board = libchess::board::create_default();

    for (int32_t y = (int32_t)(libchess::board::width - 1); y >= 0; y--) {
        for (int32_t x = 0; x < (int32_t)libchess::board::width; x++) {
            const auto& piece = board->get_piece(libchess::coord(x, y));

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
                return 1;
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

    return 0;
}
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

#include "libchess-native.h"

// for p/invoke
extern "C" {

LIBCHESS_API native_board_t* CreateBoardDefault() {
    auto board = new native_board_t;
    board->instance = libchess::board::create_default();
    return board;
}

LIBCHESS_API native_board_t* CreateBoard(const char* fen) {
    auto instance = libchess::board::create(fen);

    native_board_t* board = nullptr;
    if (instance) {
        board = new native_board_t;
        board->instance = instance;
    }

    return board;
}

LIBCHESS_API void DestroyBoard(native_board_t* board) {
    delete board; // destroys the reference
}

LIBCHESS_API bool IsOutOfBounds(const libchess::coord* position) {
    return libchess::board::is_out_of_bounds(*position);
}

LIBCHESS_API bool GetBoardPiece(native_board_t* board, const libchess::coord* position,
                                libchess::piece_info_t* piece) {
    return board->instance->get_piece(*position, piece);
}

LIBCHESS_API bool SetBoardPiece(native_board_t* board, const libchess::coord* position,
                                const libchess::piece_info_t* piece) {
    return board->instance->set_piece(*position, *piece);
}

LIBCHESS_API const char* SerializeBoardFEN(native_board_t* board) {
    std::string fen = board->instance->serialize();

    size_t length = fen.length();
    size_t size = (length + 1) * sizeof(char);
    char* buffer = (char*)malloc(size);

    buffer[length] = '\0';
    memcpy(buffer, fen.c_str(), length * sizeof(char));

    return buffer;
}

LIBCHESS_API void AdvanceTurn(native_board_t* board) {
    auto& data = board->instance->get_data();
    if (data.current_turn == libchess::player_color::white) {
        data.current_turn = libchess::player_color::black;
    } else {
        data.current_turn = libchess::player_color::white;
    }
}

LIBCHESS_API libchess::player_color GetCurrentBoardTurn(native_board_t* board) {
    return board->instance->get_data().current_turn;
}

LIBCHESS_API uint8_t GetBoardCastlingAvailability(native_board_t* board,
                                                  libchess::player_color player) {
    return board->instance->get_data().player_castling_availability.at(player);
}

LIBCHESS_API bool GetBoardEnPassantTarget(native_board_t* board, libchess::coord* target) {
    auto& data = board->instance->get_data();

    if (data.en_passant_target.has_value()) {
        *target = data.en_passant_target.value();
        return true;
    } else {
        return false;
    }
}

LIBCHESS_API uint64_t GetBoardHalfmoveClock(native_board_t* board) {
    return board->instance->get_data().halfmove_clock;
}

LIBCHESS_API uint64_t GetBoardFullmoveCount(native_board_t* board) {
    return board->instance->get_data().fullmove_count;
}

LIBCHESS_API libchess::board* GetInternalBoardPointer(native_board_t* board) {
    return board->instance.get();
}

} // end p/invoke wrapper
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

using piece_callback_t = void (*)(const libchess::piece_info_t*);
static void engine_piece_callback(const libchess::piece_info_t& piece, void* data) {
    auto callback = (piece_callback_t)data;
    if (callback != nullptr) {
        callback(&piece);
    }
}

using query_filter_t = bool (*)(const libchess::coord*, const libchess::piece_info_t*);
static bool engine_query_filter(const libchess::coord& position,
                                const libchess::piece_info_t& piece, void* data) {
    auto callback = (query_filter_t)data;
    return callback(&position, &piece);
}

extern "C" {

LIBCHESS_API native_engine_t* CreateEngine() {
    auto engine = new native_engine_t;

    engine->instance.set_capture_callback(engine_piece_callback);
    engine->instance.set_callback_data(nullptr);

    return engine;
}

LIBCHESS_API void DestroyEngine(native_engine_t* engine) { delete engine; }

LIBCHESS_API native_board_t* GetEngineBoard(native_engine_t* engine) {
    auto instance = engine->instance.get_board();

    if (instance) {
        auto board = new native_board_t;
        board->instance = instance;

        return board;
    } else {
        return nullptr;
    }
}

LIBCHESS_API void SetEngineBoard(native_engine_t* engine, native_board_t* board) {
    std::shared_ptr<libchess::board> instance;
    if (board != nullptr) {
        instance = board->instance;
    };

    engine->instance.set_board(instance);
}

LIBCHESS_API void SetEngineCaptureCallback(native_engine_t* engine, piece_callback_t callback) {
    engine->instance.set_callback_data((void*)callback);
}

LIBCHESS_API libchess::piece_query_t* CreatePieceQuery() { return new libchess::piece_query_t; }

LIBCHESS_API void SetQueryPieceType(libchess::piece_query_t* query, libchess::piece_type type) {
    query->type = type;
}

LIBCHESS_API void SetQueryPieceColor(libchess::piece_query_t* query, libchess::player_color color) {
    query->color = color;
}

LIBCHESS_API void SetQueryPieceX(libchess::piece_query_t* query, int32_t x) { query->x = x; }
LIBCHESS_API void SetQueryPieceY(libchess::piece_query_t* query, int32_t y) { query->y = y; }

LIBCHESS_API void SetQueryFilter(libchess::piece_query_t* query, query_filter_t filter) {
    query->filter = engine_query_filter;
    query->filter_data = (void*)filter;
}

LIBCHESS_API void EngineFindPieces(native_engine_t* engine, libchess::piece_query_t* query,
                                   void (*callback)(const libchess::coord*)) {
    std::vector<libchess::coord> pieces;
    engine->instance.find_pieces(*query, pieces);

    delete query;
    for (const auto& position : pieces) {
        callback(&position);
    }
}

LIBCHESS_API bool EngineComputeCheck(native_engine_t* engine, libchess::player_color color,
                                     void (*callback)(const libchess::coord*)) {
    std::vector<libchess::coord> pieces;
    bool check = engine->instance.compute_check(color, pieces);

    for (const auto& position : pieces) {
        callback(&position);
    }

    return check;
}

LIBCHESS_API bool EngineComputeCheckmate(native_engine_t* engine, libchess::player_color color) {
    return engine->instance.compute_checkmate(color);
}

LIBCHESS_API void EngineComputeLegalMoves(native_engine_t* engine, const libchess::coord* position,
                                          void (*callback)(const libchess::coord*)) {
    std::list<libchess::coord> destinations;
    engine->instance.compute_legal_moves(*position, destinations);

    for (const auto& position : destinations) {
        callback(&position);
    }
}

LIBCHESS_API bool EngineIsMoveLegal(native_engine_t* engine, const libchess::move_t* move) {
    return engine->instance.is_move_legal(*move);
}

LIBCHESS_API bool EngineCommitMove(native_engine_t* engine, const libchess::move_t* move,
                                   bool advance_turn) {
    return engine->instance.commit_move(*move, true, advance_turn);
}

LIBCHESS_API void ClearEngineCache(native_engine_t* engine) { engine->instance.clear_cache(); }

} // end of p/invoke block
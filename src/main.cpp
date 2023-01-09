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

#include "pch.h"
#include "renderer.h"

/*
static bool dump_board(std::shared_ptr<libchess::board> board) {
    libchess::piece_info_t piece;
    for (int32_t y = (int32_t)(libchess::board::width - 1); y >= 0; y--) {
        for (int32_t x = 0; x < (int32_t)libchess::board::width; x++) {
            board->get_piece(libchess::coord(x, y), &piece);
            std::cout << libchess::util::serialize_piece(piece).value_or(' ');
        }

        std::cout << std::endl;
    }

    return true;
}
*/

static bool s_quit;
static std::mutex s_mutex;

using namespace libchess::console;
void key_callback(char c, void* user_data) {
    if (c >= 'A') {
        renderer::render(libchess::coord(0, 0), c, color_default, color_default);
        renderer::flush();
    }

    if (c == '\r') {
        s_mutex.lock();
        s_quit = true;
        s_mutex.unlock();
    }
}

int main(int argc, const char** argv) {
    /*
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
    */

    renderer::init(800, 600);
    size_t callback_index = renderer::add_key_callback(key_callback);

    renderer::render(libchess::coord(10, 10), 'F', color_cyan, color_red);
    renderer::flush();

    s_mutex.lock();
    s_quit = false;
    s_mutex.unlock();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        s_mutex.lock();
        if (s_quit) {
            break;
        }

        s_mutex.unlock();
    }

    s_mutex.unlock();
    renderer::remove_key_callback(callback_index);
    renderer::shutdown();

    return 0;
}
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
#include "client.h"
#include "renderer.h"

namespace libchess::console {
    struct client_callback_delegate {
        client_callback_delegate(client* _this) {
            this->_this = _this;
        }

        void key_callback(char c) {
            util::mutex_lock lock(_this->m_mutex);

            if (c == '\r') {
                _this->m_should_quit = true;
            }

            // todo: things...
        }

        client* _this = nullptr;
    };

    static void client_key_callback(char c, void* user_data) {
        client_callback_delegate delegate((client*)user_data);
        delegate.key_callback(c);
    }

    std::shared_ptr<client> client::create(const std::optional<std::string>& fen) {
        auto _client = std::shared_ptr<client>(new client);

        if (fen.has_value()) {
            if (!_client->load_fen(fen.value())) {
                _client.reset();
            }
        }

        if (_client) {
            _client->redraw();
        }

        return _client;
    }

    client::~client() { renderer::remove_key_callback(m_key_callback); }

    bool client::load_fen(const std::string& fen) {
        util::mutex_lock lock(m_mutex);

        auto _board = board::create(fen);
        if (!_board) {
            return false;
        }

        m_engine.set_board(_board);
        return true;
    }

    bool client::should_quit() {
        util::mutex_lock lock(m_mutex);
        return m_should_quit;
    }

    client::client() {
        auto _board = board::create_default();
        m_engine.set_board(_board);

        m_key_callback = renderer::add_key_callback(client_key_callback, this);
        m_should_quit = false;
    }

    void client::redraw() {
        redraw_board(coord(0, 0));

        // todo: redraw console
    }

    void client::redraw_board(const coord& offset) {
        redraw_board_frame(offset);

        // todo: draw pieces
    }

    void client::redraw_board_frame(const coord& offset) {
        for (size_t i = 0; i <= board::width; i++) {
            for (size_t j = 0; j < board::width; j++) {
                int32_t coord_0 = (int32_t)i * 2;
                int32_t coord_1 = 1 + ((int32_t)j * 2);

                renderer::render(offset + coord(coord_0, coord_1), L'\x2551');
                renderer::render(offset + coord(coord_1, coord_0), L'\x2550');
            }
        }
    }
} // namespace libchess::console
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
    class client_mutex_lock {
    public:
        client_mutex_lock(client* _client) {
            m_lock = std::make_unique<util::mutex_lock>(_client->m_mutex);
        }

        ~client_mutex_lock() { m_lock.reset(); }

        client_mutex_lock(const client_mutex_lock&) = delete;
        client_mutex_lock& operator=(const client_mutex_lock&) = delete;

    private:
        std::unique_ptr<util::mutex_lock> m_lock;
    };

    static void client_key_callback(char c, void* user_data) {
        auto _this = (client*)user_data;
        client_mutex_lock lock(_this);

        // todo: things...
    }

    std::shared_ptr<client> client::create(const std::optional<std::string>& fen) {
        auto _client = std::shared_ptr<client>(new client);

        if (fen.has_value()) {
            if (!_client->load_fen(fen.value())) {
                _client.reset();
            }
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

    std::shared_ptr<board> client::get_board() {
        util::mutex_lock lock(m_mutex);
        return m_engine.get_board();
    }

    client::client() {
        auto _board = board::create_default();
        m_engine.set_board(_board);

        m_key_callback = renderer::add_key_callback(client_key_callback, this);
    }
} // namespace libchess::console
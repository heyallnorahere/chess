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

#pragma once

namespace libchess::console {
    class client {
    public:
        static std::shared_ptr<client> create(const std::optional<std::string>& fen = {});

        ~client();

        bool load_fen(const std::string& fen);
        std::shared_ptr<board> get_board();

    private:
        client();

        engine m_engine;
        std::mutex m_mutex;

        size_t m_key_callback;

        friend class client_mutex_lock;
    };
} // namespace libchess::console
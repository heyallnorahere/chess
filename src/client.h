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
#include "game_console.h"

namespace libchess::console {
    class client : public std::enable_shared_from_this<client> {
    public:
        static std::shared_ptr<client> create(const std::optional<std::string>& fen = {});

        ~client();

        bool load_fen(const std::string& fen);

        void update();
        bool should_quit();

        void get_console(const std::function<void(std::shared_ptr<game_console>)>& callback);

    private:
        client();

        void register_commands();
        void on_keystroke(char keystroke);

        void redraw();
        void redraw_console(const coord& offset);

        void redraw_board(const coord& offset);
        void redraw_board_frame(const coord& offset);

        // commands
        void command_quit(command_context& context);
        void command_redraw(command_context& context);

        std::shared_ptr<game_console> m_console;
        size_t m_console_update_callback, m_console_scroll_callback,
            m_console_line_submitted_callback;

        engine m_engine;
        std::mutex m_mutex;

        size_t m_key_callback;
        bool m_should_quit;
        bool m_should_redraw;

        bool m_reset_scroll_position;
        int32_t m_scroll_position, m_scroll_increment;
    };
} // namespace libchess::console
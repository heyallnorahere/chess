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
    std::shared_ptr<client> client::create(const std::optional<std::string>& fen) {
        auto _client = std::shared_ptr<client>(new client);

        if (fen.has_value()) {
            if (!_client->load_fen(fen.value())) {
                _client.reset();
            }
        }

        if (_client) {
            _client->register_commands();
            _client->m_console->set_accept_input(true);
        }

        return _client;
    }

    client::~client() {
        renderer::remove_key_callback(m_key_callback);

        m_console->remove_update_callback(m_console_update_callback);
        m_console->remove_scroll_callback(m_console_scroll_callback);
        m_console->remove_line_submitted_callback(m_console_line_submitted_callback);
    }

    bool client::load_fen(const std::string& fen) {
        util::mutex_lock lock(m_mutex);

        auto _board = board::create(fen);
        if (!_board) {
            return false;
        }

        m_engine.set_board(_board);
        return true;
    }

    void client::update() {
        util::mutex_lock lock(m_mutex);

        if (m_should_redraw) {
            redraw();
            m_should_redraw = false;
        }

        // todo: update?????
    }

    bool client::should_quit() {
        util::mutex_lock lock(m_mutex);
        return m_should_quit;
    }

    void client::get_console(const std::function<void(std::shared_ptr<game_console>)>& callback) {
        util::mutex_lock lock(m_mutex);
        callback(m_console);
    }

    client::client() {
        auto _board = board::create_default();
        m_engine.set_board(_board);

        m_key_callback = renderer::add_key_callback(LIBCHESS_BIND_METHOD(client::on_keystroke));
        m_should_quit = false;
        m_should_redraw = true;

        m_scroll_position = -1;
        m_scroll_increment = 0;
        m_reset_scroll_position = false;

        m_console = game_console::create();
        m_console_update_callback = m_console->add_update_callback([this]() mutable {
            m_reset_scroll_position = true;
            m_should_redraw = true;
        });

        m_console_scroll_callback =
            m_console->add_scroll_callback([this](int32_t increment) mutable {
                m_scroll_increment += increment;
                m_should_redraw = true;
            });

        m_console_line_submitted_callback =
            m_console->add_line_submitted_callback([this](const std::string& line) mutable {
                if (m_scroll_position >= 0) {
                    m_scroll_increment++;
                }

                m_should_redraw = true;
            });
    }

#define BIND_CLIENT_COMMAND(func)                                                                  \
    [this](command_context& context) {                                                             \
        context.set_accept_input(false);                                                           \
        func(context);                                                                             \
        context.set_accept_input(true);                                                            \
    }

    void client::register_commands() {
        command_factory factory(m_console);

        // quit command
        factory.add_alias("quit");
        factory.set_callback(BIND_CLIENT_COMMAND(client::command_quit));
        factory.set_description("Exit the chess engine.");

        // redraw command
        factory.new_command();
        factory.add_alias("redraw");
        factory.set_callback(BIND_CLIENT_COMMAND(client::command_redraw));
        factory.set_description("Redraw the screen.");
    }

    void client::on_keystroke(char keystroke) {
        util::mutex_lock lock(m_mutex);
        m_console->process_keystroke(keystroke);
    }

    void client::redraw() {
        redraw_board(coord(0, 0));
        redraw_console(coord(0, board::width * 2 + 1));
    }

    void client::redraw_console(const coord& offset) {
        static constexpr int32_t console_width = 50; // temporary
        static constexpr int32_t console_height = 5; // also temporary

        // corners
        renderer::render(offset + coord(0, 0), L'\x2554');
        renderer::render(offset + coord(console_width + 1, 0), L'\x2557');
        renderer::render(offset + coord(0, console_height + 1), L'\x255a');
        renderer::render(offset + coord(console_width + 1, console_height + 1), L'\x255d');

        // horizontal lines
        for (int32_t i = 0; i < console_width; i++) {
            static constexpr wchar_t line_character = L'\x2550';
            int32_t x = i + 1;

            renderer::render(offset + coord(x, 0), line_character);
            renderer::render(offset + coord(x, console_height + 1), line_character);
        }

        // vertical lines
        for (int32_t i = 0; i < console_height; i++) {
            static constexpr wchar_t line_character = L'\x2551';
            int32_t y = i + 1;

            renderer::render(offset + coord(0, y), line_character);
            renderer::render(offset + coord(console_width + 1, y), line_character);
        }

        m_console->get_log(
            [&](const std::list<std::string>& log) {
                int32_t scroll_pos = m_scroll_position;
                int32_t log_size = (int32_t)log.size();

                if (scroll_pos < 0) {
                    scroll_pos = 0;
                }

                scroll_pos += m_scroll_increment;
                if (m_reset_scroll_position) {
                    scroll_pos = 0;
                }

                scroll_pos = std::min(scroll_pos, log_size - 1);
                scroll_pos = std::max(scroll_pos, 0);

                for (int32_t i = 0; i < console_height - 1; i++) {
                    std::string message;
                    if (i < log_size - scroll_pos) {
                        auto it = log.rbegin();
                        std::advance(it, i + scroll_pos);
                        message = *it;
                    }

                    int32_t y = console_height - (i + 1);
                    for (int32_t j = 0; j < console_width; j++) {
                        int32_t x = j + 1;

                        wchar_t character;
                        if (j < message.length()) {
                            character = (wchar_t)message[j];
                        } else {
                            character = L' ';
                        }

                        renderer::render(offset + coord(x, y), character);
                    }
                }

                if (m_scroll_increment != 0) {
                    m_scroll_increment = 0;

                    if (scroll_pos > 0) {
                        m_scroll_position = scroll_pos;
                    }
                }

                if (m_reset_scroll_position) {
                    m_scroll_position = -1;
                    m_reset_scroll_position = false;
                }
            },
            (size_t)console_width);

        {
            std::string current_command = m_console->get_current_command();
            size_t cursor_pos = m_console->get_cursor_pos();

            renderer::render(offset + coord(1, console_height), L'>');
            for (int32_t i = 0; i < console_width - 1; i++) {
                auto pos = offset + coord((int32_t)i + 2, console_height);

                if (i == cursor_pos) {
                    renderer::render(pos, L'\x2588');
                } else {
                    if (i < current_command.length()) {
                        renderer::render(pos, (wchar_t)current_command[i]);
                    } else {
                        renderer::render(pos, L' ');
                    }
                }
            }
        }
    }

    void client::redraw_board(const coord& offset) {
        redraw_board_frame(offset);

        // drawing pieces
        for (int32_t x = 0; x < board::width; x++) {
            for (int32_t y = 0; y < board::width; y++) {
                auto local = coord(x, y);

                bool is_tile_white = local.taxicab_length() % 2 != 0;
                uint32_t fg = is_tile_white ? color_black : color_white;
                uint32_t bg = is_tile_white ? color_white : color_black;

                piece_info_t piece;
                m_engine.get_piece(local, &piece);

                char character = util::serialize_piece(piece).value_or(' ');
                auto global = offset + coord(1 + (x * 2), 1 + ((board::width - (y + 1)) * 2));

                renderer::render(global, (wchar_t)character, fg, bg);
            }
        }
    }

    void client::redraw_board_frame(const coord& offset) {
        // lines
        for (size_t i = 0; i <= board::width; i++) {
            for (size_t j = 0; j < board::width; j++) {
                int32_t coord_0 = (int32_t)i * 2;
                int32_t coord_1 = 1 + ((int32_t)j * 2);

                renderer::render(offset + coord(coord_0, coord_1), L'\x2551');
                renderer::render(offset + coord(coord_1, coord_0), L'\x2550');
            }
        }

        // intersections
        for (size_t i = 0; i < board::width - 1; i++) {
            for (size_t j = 0; j < board::width - 1; j++) {
                int32_t x = 2 + ((int32_t)i * 2);
                int32_t y = 2 + ((int32_t)j * 2);

                renderer::render(offset + coord(x, y), L'\x256c');
            }
        }

        // corners
        renderer::render(offset + coord(0, 0), L'\x2554');
        renderer::render(offset + coord(board::width * 2, 0), L'\x2557');
        renderer::render(offset + coord(0, board::width * 2), L'\x255a');
        renderer::render(offset + coord(board::width * 2, board::width * 2), L'\x255d');

        // edge intersections
        for (size_t i = 0; i < board::width - 1; i++) {
            int32_t c = 2 + ((int32_t)i * 2);

            renderer::render(offset + coord(c, 0), L'\x2566');
            renderer::render(offset + coord(c, board::width * 2), L'\x2569');
            renderer::render(offset + coord(0, c), L'\x2560');
            renderer::render(offset + coord(board::width * 2, c), L'\x2563');
        }
    }

    void client::command_quit(command_context& context) { m_should_quit = true; }

    void client::command_redraw(command_context& context) {
        context.submit_line("Console redrawn!"); // should trigger a console redraw
    }
} // namespace libchess::console
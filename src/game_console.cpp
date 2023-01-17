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
#include "game_console.h"
#include "renderer.h"

namespace libchess::console {
    static const std::string s_fallback_alias = "__fallback__";

    std::shared_ptr<game_console> game_console::create() {
        auto console = std::shared_ptr<game_console>(new game_console);

        // todo: checks?

        return console;
    }

    game_console::~game_console() { renderer::destroy_keystroke_state(m_keystroke_state); }

    void game_console::set_accept_input(bool accept) {
        util::mutex_lock lock(m_mutex);
        set_accept_input_internal(accept); // for consistency
    }

    void game_console::process_keystroke(char c) {
        util::mutex_lock lock(m_mutex);

        bool should_update = false;
        int32_t scroll_offset = 0;

        auto type = renderer::parse_keystroke(c, &m_keystroke_state);
        switch (type) {
        case keystroke_type::character:
            switch (c) {
            case '\r':
                execute_command_internal(m_current_command);

                m_current_command.clear();
                m_cursor_pos = 0;

                should_update = true;
                break;
            case (char)8: // backspace characters
            case (char)127:
                if (m_cursor_pos > 0) {
                    m_cursor_pos--;
                    m_current_command.replace(m_cursor_pos, 1, "");

                    should_update = true;
                }

                break;
            default:
                m_current_command.insert(m_cursor_pos, 1, c);
                m_cursor_pos++;

                should_update = true;
                break;
            }

            break;
        case keystroke_type::left_arrow:
            if (m_cursor_pos > 0) {
                m_cursor_pos--;
                should_update = true;
            }

            break;
        case keystroke_type::right_arrow:
            if (m_cursor_pos < m_current_command.length()) {
                m_cursor_pos++;
                should_update = true;
            }

            break;
        case keystroke_type::up_arrow:
            scroll_offset++;
            break;
        case keystroke_type::down_arrow:
            scroll_offset--;
            break;
        default:
            // nothing
            break;
        }

        if (scroll_offset != 0) {
            call_event(m_scroll_callbacks, scroll_offset);
        }

        if (should_update) {
            call_event(m_update_callbacks);
        }
    }

    void game_console::execute_command(const std::string& command) {
        util::mutex_lock lock(m_mutex);
        execute_command_internal(command);
    }

    void game_console::submit_line(const std::string& line) {
        util::mutex_lock lock(m_mutex);
        submit_line_internal(line);
    }

    void game_console::get_log(const std::function<void(const std::list<std::string>&)>& callback,
                               size_t max_line_width) {
        util::mutex_lock lock(m_mutex);

        if (max_line_width > 0) {
            std::list<std::string> temp_log;
            for (const auto& line : m_log) {
                if (line.length() > max_line_width) {
                    size_t current_pos = 0;
                    do {
                        temp_log.push_back(line.substr(current_pos, max_line_width));
                        current_pos += max_line_width;
                    } while (current_pos < line.length());
                } else {
                    temp_log.push_back(line);
                }
            }

            callback(temp_log);
        } else {
            callback(m_log);
        }
    }

    std::string game_console::get_current_command() {
        util::mutex_lock lock(m_mutex);
        return m_current_command;
    }

    size_t game_console::get_cursor_pos() {
        util::mutex_lock lock(m_mutex);
        return m_cursor_pos;
    }

    size_t game_console::add_update_callback(const std::function<void()>& callback) {
        util::mutex_lock lock(m_mutex);
        return add_callback(m_update_callbacks, callback);
    }

    bool game_console::remove_update_callback(size_t index) {
        util::mutex_lock lock(m_mutex);
        return remove_callback(m_update_callbacks, index);
    }

    size_t game_console::add_scroll_callback(const std::function<void(int32_t)>& callback) {
        util::mutex_lock lock(m_mutex);
        return add_callback(m_scroll_callbacks, callback);
    }

    bool game_console::remove_scroll_callback(size_t index) {
        util::mutex_lock lock(m_mutex);
        return remove_callback(m_scroll_callbacks, index);
    }

    size_t game_console::add_line_submitted_callback(const std::function<void(const std::string)>& callback) {
        util::mutex_lock lock(m_mutex);
        return add_callback(m_line_submitted_callbacks, callback);
    }

    bool game_console::remove_line_submitted_callback(size_t index) {
        util::mutex_lock lock(m_mutex);
        return remove_callback(m_line_submitted_callbacks, index);
    }

    game_console::game_console() {
        m_accept_input = false;
        m_keystroke_state = nullptr;
        m_cursor_pos = 0;
    }

    void game_console::execute_command_internal(const std::string& command) {
        submit_line_internal(">" + command);

        std::vector<std::string> segments;
        util::split_string(command, ' ', segments, util::string_split_options_omit_empty);

        if (segments.empty()) {
            return;
        }

        std::vector<std::string> command_arguments;
        if (segments.size() > 1) {
            std::string current_argument;

            bool in_quotes = false;
            for (const auto& segment : segments) {
                bool escape = false;

                if (!current_argument.empty()) {
                    current_argument += ' ';
                }

                for (char c : segment) {
                    switch (c) {
                    case '\\':
                        if (escape) {
                            current_argument += c;
                            escape = false;
                        } else {
                            escape = true;
                        }

                        break;
                    case '"':
                        if (escape) {
                            current_argument += c;
                            escape = false;
                        } else {
                            in_quotes = !in_quotes;
                        }

                        break;
                    default:
                        current_argument += c;
                        escape = false;

                        break;
                    }
                }

                if (!escape && !in_quotes) {
                    command_arguments.push_back(current_argument);
                    current_argument.clear();
                }
            }

            if (!current_argument.empty()) {
                command_arguments.push_back(current_argument);
            }
        } else {
            command_arguments.push_back(segments[0]);
        }

        std::string command_name = command_arguments[0];
        if (m_commands.find(command_name) != m_commands.end()) {
            auto it = command_arguments.begin();
            it++;

            std::vector<std::string> args(it, command_arguments.end());
            command_context context(shared_from_this(), args);

            auto info = m_commands.at(command_name);
            info->callback(context);
        } else if (m_commands.find(s_fallback_alias) != m_commands.end()) {
            command_context context(shared_from_this(), command_arguments);

            auto info = m_commands.at(command_name);
            info->callback(context);
        } else {
            submit_line_internal("Invalid command");
        }
    }

    void game_console::submit_line_internal(const std::string& line) {
        m_log.push_back(line);

        // placeholder limit
        while (m_log.size() > 30) {
            m_log.pop_front();
        }

        call_event(m_line_submitted_callbacks, line);
    }

    void game_console::set_accept_input_internal(bool accept) {
        m_accept_input = accept;
        // maybe call events? idk
    }

    void command_context::submit_line(const std::string& line) {
        util::mutex_lock lock(m_mutex);
        m_console->submit_line_internal(line);
    }

    void command_context::set_accept_input(bool accept) {
        util::mutex_lock lock(m_mutex);
        m_console->set_accept_input_internal(accept);
    }

    command_context::command_context(std::shared_ptr<game_console> console,
                                     const std::vector<std::string>& args) {
        m_console = console;
        m_args = args;
    }

    void command_factory::new_command() {
        util::mutex_lock lock(m_mutex);

        submit();
        m_current_command.reset();
    }

    void command_factory::set_as_fallback() { add_alias(s_fallback_alias); }
    bool command_factory::is_fallback() { return has_alias(s_fallback_alias); }

    void command_factory::add_alias(const std::string& alias) {
        util::mutex_lock lock(m_mutex);

        verify_command();
        m_current_command->aliases.insert(alias);
    }

    bool command_factory::has_alias(const std::string& alias) {
        util::mutex_lock lock(m_mutex);

        if (!m_current_command) {
            return false;
        }

        return m_current_command->aliases.find(alias) != m_current_command->aliases.end();
    }

    void command_factory::set_description(const std::string& desc) {
        util::mutex_lock lock(m_mutex);

        verify_command();
        m_current_command->description = desc;
    }

    void command_factory::set_callback(const console_command_callback& callback) {
        util::mutex_lock lock(m_mutex);

        verify_command();
        m_current_command->callback = callback;
    }

    void command_factory::submit() {
        if (!m_current_command || !m_current_command->callback) {
            return;
        }

        for (const auto& alias : m_current_command->aliases) {
            m_console->m_commands[alias] = m_current_command;
        }
    }

    void command_factory::verify_command() {
        if (m_current_command) {
            return;
        }

        m_current_command = std::make_shared<console_command_t>();
    }
} // namespace libchess::console
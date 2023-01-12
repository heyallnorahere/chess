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

    game_console::~game_console() {
        renderer::destroy_keystroke_state(m_keystroke_state);
    }

    void game_console::set_accept_input(bool accept) {
        util::mutex_lock lock(m_mutex);
        m_accept_input = accept;
    }


    void game_console::process_keystroke(char c) {
        util::mutex_lock lock(m_mutex);

        auto type = renderer::parse_keystroke(c, &m_keystroke_state);
        switch (type) {
        case keystroke_type::character:
            switch (c) {
            case '\r':
                execute_command_internal(m_current_command);
                m_current_command.clear();

                m_cursor_pos = 0;
                break;
            case (char)127: // backspace
                if (m_cursor_pos > 0) {
                    m_cursor_pos--;
                    m_current_command.replace(m_cursor_pos, 1, "");
                }

                break;
            default:
                m_current_command.insert(m_cursor_pos, 1, c);
                m_cursor_pos++;
                break;
            }

            break;
        case keystroke_type::left_arrow:
            if (m_cursor_pos > 0) {
                m_cursor_pos--;
            }

            break;
        case keystroke_type::right_arrow:
            if (m_cursor_pos < m_current_command.length()) {
                m_cursor_pos++;
            }

            break;
        default:
            // nothing
            break;
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

    void game_console::get_log(const std::function<void(const std::list<std::string>&)>& callback) {
        util::mutex_lock lock(m_mutex);
        callback(m_log);
    }

    std::string game_console::get_current_command() {
        util::mutex_lock lock(m_mutex);
        return m_current_command;
    }

    game_console::game_console() {
        m_accept_input = false;
        m_keystroke_state = nullptr;
        m_cursor_pos = 0;
    }

    void game_console::execute_command_internal(const std::string& command) {
        submit_line_internal(">" + command);

        // todo: parse command
    }

    void game_console::submit_line_internal(const std::string& line) {
        m_log.push_back(line);

        // placeholder limit
        while (m_log.size() > 30) {
            m_log.pop_front();
        }
    }

    void command_factory::new_command() {
        submit();
        m_current_command.reset();
    }

    void command_factory::set_as_fallback() { add_alias(s_fallback_alias); }
    bool command_factory::is_fallback() { return has_alias(s_fallback_alias); }

    void command_factory::add_alias(const std::string& alias) {
        verify_command();
        m_current_command->aliases.insert(alias);
    }

    bool command_factory::has_alias(const std::string& alias) {
        if (!m_current_command) {
            return false;
        }

        return m_current_command->aliases.find(alias) != m_current_command->aliases.end();
    }

    void command_factory::set_description(const std::string& desc) {
        verify_command();
        m_current_command->description = desc;
    }

    void command_factory::set_callback(const console_command_callback& callback) {
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
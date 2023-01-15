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
    class command_context;
    using console_command_callback = std::function<void(command_context&)>;

    struct console_command_t {
        console_command_callback callback;
        std::unordered_set<std::string> aliases;
        std::string description;
    };

    class game_console : public std::enable_shared_from_this<game_console> {
    public:
        static std::shared_ptr<game_console> create();

        ~game_console();

        game_console(const game_console&) = delete;
        game_console& operator=(const game_console&) = delete;

        void set_accept_input(bool accept);
        void process_keystroke(char c);
        void execute_command(const std::string& command);

        void submit_line(const std::string& line);
        void get_log(const std::function<void(const std::list<std::string>&)>& callback);

        std::string get_current_command();
        size_t get_cursor_pos();

        size_t add_update_callback(const std::function<void()>& callback);
        bool remove_update_callback(size_t index);

    private:
        game_console();

        void execute_command_internal(const std::string& command);
        void submit_line_internal(const std::string& line);
        void set_accept_input_internal(bool accept);

        std::unordered_map<std::string, std::shared_ptr<console_command_t>> m_commands;
        std::vector<std::optional<std::function<void()>>> m_update_callbacks;
        std::list<std::string> m_log;
        std::mutex m_mutex;

        std::string m_current_command;
        size_t m_cursor_pos;

        bool m_accept_input;
        void* m_keystroke_state;

        friend class command_factory;
        friend class command_context;
    };

    class command_context {
    public:
        ~command_context() = default;

        command_context(const command_context&) = delete;
        command_context& operator=(const command_context&) = delete;

        void submit_line(const std::string& line);
        void set_accept_input(bool accept);

        const std::vector<std::string>& get_args() const { return m_args; }

    private:
        command_context(std::shared_ptr<game_console> console,
                        const std::vector<std::string>& args);

        std::shared_ptr<game_console> m_console;
        std::vector<std::string> m_args;
        std::mutex m_mutex;

        friend class game_console;
    };

    class command_factory {
    public:
        command_factory(std::shared_ptr<game_console> console) { m_console = console; }
        ~command_factory() { submit(); }

        command_factory(const command_factory&) = delete;
        command_factory& operator=(const command_factory&) = delete;

        void new_command();

        void set_as_fallback();
        bool is_fallback();

        void add_alias(const std::string& alias);
        bool has_alias(const std::string& alias);

        void set_description(const std::string& desc);
        void set_callback(const console_command_callback& callback);

    private:
        void submit();
        void verify_command();

        std::shared_ptr<game_console> m_console;
        std::shared_ptr<console_command_t> m_current_command;
        std::mutex m_mutex;
    };
} // namespace libchess::console
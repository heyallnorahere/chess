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
#include "backends.h"

#include <termios.h>
#include <unistd.h>
#include <pthread.h>

namespace libchess::console {
    static const std::string s_terminal_escape_sequence = "\x1b[";
    static std::unique_ptr<termios> s_normal_terminal_state;

    static void unix_save_screen() { std::cout << s_terminal_escape_sequence << "?47h"; }
    static void unix_restore_screen() { std::cout << s_terminal_escape_sequence << "?47l"; }
    static void unix_clear_screen() { std::cout << s_terminal_escape_sequence << "J"; }

    static void unix_save_cursor_pos() { std::cout << s_terminal_escape_sequence << "s"; }
    static void unix_restore_cursor_pos() { std::cout << s_terminal_escape_sequence << "u"; }

    static void unix_set_color(uint32_t fg, uint32_t bg) {
        std::cout << s_terminal_escape_sequence << "3" << fg << ";4" << bg << "m";
    }

    static void unix_reset_color() { std::cout << s_terminal_escape_sequence << "0m"; }

    static void unix_set_cursor_pos(const coord& pos) {
        std::cout << s_terminal_escape_sequence << pos.y << ";" << pos.x << "H";
    }

    static void unix_advance_cursor_line() { std::cout << s_terminal_escape_sequence << "1E"; }

    static void unix_disable_cursor() { std::cout << s_terminal_escape_sequence << "?25l"; }
    static void unix_enable_cursor() { std::cout << s_terminal_escape_sequence << "?25h"; }

    static void unix_flush_console() { std::cout << std::flush; }

    static void unix_setup_input_capture() {
        if (s_normal_terminal_state) {
            return;
        }

        termios desc;
        if (tcgetattr(STDOUT_FILENO, &desc) != 0) {
            throw std::runtime_error("failed to query the state of the terminal!");
        }

        s_normal_terminal_state = std::make_unique<termios>(desc);
        cfmakeraw(&desc);

        if (tcsetattr(STDOUT_FILENO, TCSANOW, &desc) != 0) {
            throw std::runtime_error("failed to set the state of the terminal!");
        }
    }

    static void unix_cleanup_input_capture() {
        if (!s_normal_terminal_state) {
            return;
        }

        if (tcsetattr(STDOUT_FILENO, TCSANOW, s_normal_terminal_state.get()) != 0) {
            throw std::runtime_error("failed to revert terminal!");
        }

        s_normal_terminal_state.reset();
    }

    static void unix_set_thread_name(std::thread& thread, const std::string& name) {
#ifdef LIBCHESS_PLATFORM_LINUX
        pthread_setname_np(thread.native_handle(), name.c_str());
#else
        // nothing. osx is lame
#endif
    }

    void populate_backend_functions(renderer_backend_t& backend) {
        backend.save_screen = unix_save_screen;
        backend.restore_screen = unix_restore_screen;
        backend.clear_screen = unix_clear_screen;

        backend.save_cursor_pos = unix_save_cursor_pos;
        backend.restore_cursor_pos = unix_restore_cursor_pos;

        backend.set_color = unix_set_color;
        backend.reset_color = unix_reset_color;

        backend.set_cursor_pos = unix_set_cursor_pos;
        backend.advance_cursor_line = unix_advance_cursor_line;

        backend.disable_cursor = unix_disable_cursor;
        backend.enable_cursor = unix_enable_cursor;

        backend.flush_console = unix_flush_console;

        backend.setup_input_capture = unix_setup_input_capture;
        backend.cleanup_input_capture = unix_cleanup_input_capture;

        backend.set_thread_name = unix_set_thread_name;
    }
} // namespace libchess::console
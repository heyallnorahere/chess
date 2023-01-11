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
    static const std::wstring s_terminal_escape_sequence = L"\x1b[";
    static std::unique_ptr<termios> s_normal_terminal_state;

    static void unix_save_screen() { std::wcout << s_terminal_escape_sequence << L"?47h"; }
    static void unix_restore_screen() { std::wcout << s_terminal_escape_sequence << L"?47l"; }
    static void unix_clear_screen() { std::wcout << s_terminal_escape_sequence << L"J"; }

    static void unix_save_cursor_pos() { std::wcout << s_terminal_escape_sequence << L"s"; }
    static void unix_restore_cursor_pos() { std::wcout << s_terminal_escape_sequence << L"u"; }

    static void unix_set_cursor_pos(const coord& pos) {
        std::wcout << s_terminal_escape_sequence << pos.y << L";" << pos.x << L"H";
    }

    static void unix_disable_cursor() { std::wcout << s_terminal_escape_sequence << L"?25l"; }
    static void unix_enable_cursor() { std::wcout << s_terminal_escape_sequence << L"?25h"; }

    static void unix_set_color(uint32_t fg, uint32_t bg) {
        std::wcout << s_terminal_escape_sequence << L"3" << fg << L";4" << bg << L"m";
    }

    static void unix_reset_color() { std::wcout << s_terminal_escape_sequence << L"0m"; }

    static void unix_verify_locale() { setlocale(LC_ALL, "en_US.UTF-8"); }
    static void unix_flush_console() { std::wcout << std::flush; }

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

    static char unix_capture_character_blocking() { return (char)std::cin.get(); }

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
        backend.set_cursor_pos = unix_set_cursor_pos;

        backend.disable_cursor = unix_disable_cursor;
        backend.enable_cursor = unix_enable_cursor;

        backend.set_color = unix_set_color;
        backend.reset_color = unix_reset_color;

        backend.verify_locale = unix_verify_locale;
        backend.flush_console = unix_flush_console;

        backend.setup_input_capture = unix_setup_input_capture;
        backend.cleanup_input_capture = unix_cleanup_input_capture;
        backend.capture_character_blocking = unix_capture_character_blocking;

        backend.set_thread_name = unix_set_thread_name;
    }
} // namespace libchess::console
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

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

namespace libchess::console {
    static HANDLE s_stdout_handle = ::GetStdHandle(STD_OUTPUT_HANDLE);
    static HANDLE s_stdin_handle = ::GetStdHandle(STD_INPUT_HANDLE);

    // https://learn.microsoft.com/en-us/windows/console/clearing-the-screen lmao
    static void windows_clear_screen() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        if (!::GetConsoleScreenBufferInfo(s_stdout_handle, &csbi)) {
            throw std::runtime_error("failed to query the console screen buffer!");
        }

        SMALL_RECT scroll_rect;
        scroll_rect.Left = 0;
        scroll_rect.Top = 0;
        scroll_rect.Right = csbi.dwSize.X;
        scroll_rect.Bottom = csbi.dwSize.Y;

        COORD scroll_target;
        scroll_target.X = 0;
        scroll_target.Y = -csbi.dwSize.Y;

        CHAR_INFO fill;
        fill.Char.AsciiChar = ' ';
        fill.Attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        ::ScrollConsoleScreenBufferA(s_stdout_handle, &scroll_rect, nullptr, scroll_target, &fill);
    }

    static void windows_set_cursor_pos(const coord& pos) {
        COORD position;
        position.X = (SHORT)pos.x;
        position.Y = (SHORT)pos.y;

        ::SetConsoleCursorPosition(s_stdout_handle, position);
    }

    static void get_windows_cursor_info(CONSOLE_CURSOR_INFO& info) {
        if (!::GetConsoleCursorInfo(s_stdout_handle, &info)) {
            throw std::runtime_error("failed to query cursor info!");
        }
    }

    static void windows_disable_cursor() {
        CONSOLE_CURSOR_INFO info;
        get_windows_cursor_info(info);

        info.bVisible = FALSE;
        ::SetConsoleCursorInfo(s_stdout_handle, &info);
    }

    static void windows_enable_cursor() {
        CONSOLE_CURSOR_INFO info;
        get_windows_cursor_info(info);

        info.bVisible = TRUE;
        ::SetConsoleCursorInfo(s_stdout_handle, &info);
    }

    static uint32_t to_win32_color(uint32_t color) {
        uint32_t result = 0;

        if ((color & color_red) != 0) {
            result |= color_blue;
        }

        if ((color & color_green) != 0) {
            result |= color_green;
        }

        if ((color & color_blue) != 0) {
            result |= color_red;
        }

        return result;
    }
    static void windows_set_color(uint32_t fg, uint32_t bg) {
        uint32_t desired_bg = bg != color_default ? to_win32_color(bg) : 0;
        uint32_t desired_fg = fg != color_default
                                  ? to_win32_color(fg)
                                  : FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

        WORD attributes = 0;
        attributes |= (WORD)desired_fg;
        attributes |= (WORD)(desired_bg << 4);

        ::SetConsoleTextAttribute(s_stdout_handle, attributes);
    }

    static void windows_reset_color() { windows_set_color(color_default, color_default); }

    static void windows_verify_locale() { setlocale(LC_ALL, "en_US.UTF-8"); }

    static std::optional<DWORD> s_previous_stdin_mode;
    static void windows_setup_input_capture() {
        if (s_previous_stdin_mode.has_value()) {
            return;
        }

        DWORD mode;
        ::GetConsoleMode(s_stdin_handle, &mode);
        s_previous_stdin_mode = mode;

        mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);

        ::SetConsoleMode(s_stdin_handle, mode);
    }

    static void windows_cleanup_input_capture() {
        if (!s_previous_stdin_mode.has_value()) {
            return;
        }

        ::SetConsoleMode(s_stdin_handle, s_previous_stdin_mode.value());
        s_previous_stdin_mode.reset();
    }

    static char windows_capture_character_blocking() {
        INPUT_RECORD record;
        DWORD inputs_read;

        while (true) {
            if (::ReadConsoleInputA(s_stdin_handle, &record, 1, &inputs_read) && inputs_read != 0) {
                if (record.EventType == KEY_EVENT && record.Event.KeyEvent.bKeyDown) {
                    char character = record.Event.KeyEvent.uChar.AsciiChar;
                    if (character != '\0') {
                        return character;
                    } else {
                        // using ascii box control characters as code
                        // im lazy
                        switch (record.Event.KeyEvent.wVirtualKeyCode) {
                        case VK_UP:
                            return (char)17;
                        case VK_DOWN:
                            return (char)18;
                        case VK_LEFT:
                            return (char)19;
                        case VK_RIGHT:
                            return (char)20;
                        default:
                            // nothing
                            break;
                        }
                    }
                }
            } else {
                return (char)-1;
            }
        }
    }

    static keystroke_type windows_parse_keystroke(char c, void** state) {
        switch (c) { // see windows_capture_character_blocking
        case (char)17:
            return keystroke_type::up_arrow;
        case (char)18:
            return keystroke_type::down_arrow;
        case (char)19:
            return keystroke_type::left_arrow;
        case (char)20:
            return keystroke_type::right_arrow;
        default:
            return keystroke_type::character;
        }
    }

    static void windows_set_thread_name(std::thread& thread, const std::string& name) {
        std::vector<WCHAR> name_buffer;
        for (char c : name) {
            // good enough
            name_buffer.push_back((WCHAR)c);
        }

        name_buffer.push_back(L'\0');
        ::SetThreadDescription(thread.native_handle(), name_buffer.data());
    }

    void populate_backend_functions(renderer_backend_t& backend) {
        backend.save_screen = nullptr;
        backend.restore_screen = nullptr;
        backend.clear_screen = windows_clear_screen;

        backend.save_cursor_pos = nullptr;
        backend.restore_cursor_pos = nullptr;
        backend.set_cursor_pos = windows_set_cursor_pos;

        backend.disable_cursor = windows_disable_cursor;
        backend.enable_cursor = windows_enable_cursor;

        backend.set_color = windows_set_color;
        backend.reset_color = windows_reset_color;

        backend.verify_locale = windows_verify_locale;
        backend.flush_console = nullptr;

        backend.setup_input_capture = windows_setup_input_capture;
        backend.cleanup_input_capture = windows_cleanup_input_capture;
        backend.capture_character_blocking = windows_capture_character_blocking;

        backend.parse_keystroke = windows_parse_keystroke;
        backend.destroy_keystroke_state = nullptr;

        backend.set_thread_name = windows_set_thread_name;
    }
} // namespace libchess::console
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
    enum renderer_color : uint32_t {
        color_black = 0,
        color_red = (1 << 0),
        color_green = (1 << 1),
        color_yellow = color_red | color_green,
        color_blue = (1 << 2),
        color_magenta = color_red | color_blue,
        color_cyan = color_green | color_blue,
        color_white = color_red | color_green | color_blue,
        color_default = 9
    };

    enum class keystroke_type {
        up_arrow,
        down_arrow,
        left_arrow,
        right_arrow,
        character,
        escape
    };

    using key_callback = void (*)(char, void*);
    class renderer {
    public:
        renderer() = delete;

        static void init(int32_t width, int32_t height);
        static void shutdown();

        static void clear_screen();
        static void flush();

        static void render(const coord& pos, wchar_t character, uint32_t fg = color_default,
                           uint32_t bg = color_default);

        static size_t add_key_callback(key_callback callback, void* user_data = nullptr);
        static bool remove_key_callback(size_t index);

        static keystroke_type parse_keystroke(char c, void** state);
        static void destroy_keystroke_state(void* state);
    };
}; // namespace libchess::console
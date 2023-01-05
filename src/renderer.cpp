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
#include "renderer.h"

namespace libchess::console {
    struct cell_info_t {
        char character;
        uint32_t fg, bg;
    };

    struct renderer_info_t {
        cell_info_t* buffer;
        size_t buffer_size;
        int32_t width, height;

        std::string escape_sequence;
        std::unordered_set<coord> rendered_indices;
    };

    static std::unique_ptr<renderer_info_t> s_renderer_info;
    static void save_screen() {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // no implementation i dont think
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "?47h";
#endif
    }

    static void restore_screen() {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // still nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "?47l";
#endif
    }

    static void save_cursor_pos() {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "s";
#endif
    }

    static void restore_cursor_pos() {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "u";
#endif
    }

    static void set_color(uint32_t fg, uint32_t bg) {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "3" << fg << ";4" << bg << "m";
#endif
    }

    static void reset_color() {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "0m";
#endif
    }

    static void set_cursor_pos(const coord& pos) {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << pos.y << ";" << pos.x << "H";
#endif
    }

    static void advance_cursor_line() {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "1E";
#endif
    }

    static void disable_cursor() {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "?25l";
#endif
    }

    static void enable_cursor() {
#ifdef LIBCHESS_PLATFORM_WINDOWS
        // nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "?25h";
#endif
    }

    static void flush_console() {
#ifdef LIBCHESS_PLATFORM_UNIX
        std::cout << std::flush;
#else
        // nothing
#endif
    }

    void renderer::init(int32_t width, int32_t height) {
        if (s_renderer_info) {
            throw std::runtime_error("renderer already initialized!");
        }

        s_renderer_info = std::make_unique<renderer_info_t>();
#ifdef LIBCHESS_PLATFORM_UNIX
        s_renderer_info->escape_sequence = "\x1b[";
#endif

        s_renderer_info->width = width;
        s_renderer_info->height = height;

        s_renderer_info->buffer_size = sizeof(cell_info_t) * width * height;
        s_renderer_info->buffer = (cell_info_t*)malloc(s_renderer_info->buffer_size);

        if (s_renderer_info->buffer == nullptr) {
            throw std::runtime_error("failed to allocate renderer buffer!");
        }

        save_cursor_pos();
        save_screen();
        clear_screen();
        disable_cursor();
        flush_console();
    }

    void renderer::shutdown() {
        free(s_renderer_info->buffer);

        reset_color();
        enable_cursor();
        restore_screen();
        restore_cursor_pos();
        flush_console();

        s_renderer_info.reset();
    }

    void renderer::clear_screen() {
        s_renderer_info->rendered_indices.clear();
        for (size_t i = 0; i < (size_t)s_renderer_info->width * s_renderer_info->height; i++) {
            s_renderer_info->buffer[i].character = ' ';
            s_renderer_info->buffer[i].fg = color_default;
            s_renderer_info->buffer[i].bg = color_default;
        }

#ifdef LIBCHESS_PLATFORM_WINDOWS
        // nothing
#elif defined(LIBCHESS_PLATFORM_UNIX)
        std::cout << s_renderer_info->escape_sequence << "J";
#endif

        set_cursor_pos(coord(0, 0));
        set_color(color_default, color_default);
        flush_console();
    }

    void renderer::flush() {
        for (const auto& pos : s_renderer_info->rendered_indices) {
            size_t index = ((size_t)pos.y * s_renderer_info->width) + pos.x;
            const auto& cell = s_renderer_info->buffer[index];

            set_cursor_pos(pos);
            set_color(cell.fg, cell.bg);

            std::cout << cell.character;
#ifdef LIBCHESS_PLATFORM_WINDOWS
            std::cout << std::flush;
#endif
        }

        flush_console();
        s_renderer_info->rendered_indices.clear();
    }

    void renderer::render(const coord& pos, char character, uint32_t fg, uint32_t bg) {
        size_t index = ((size_t)pos.y * s_renderer_info->width) + pos.x;
        auto& cell = s_renderer_info->buffer[index];

        if (cell.character == character && cell.fg == fg && cell.bg == bg) {
            return;
        }

        cell.character = character;
        cell.fg = fg;
        cell.bg = bg;

        s_renderer_info->rendered_indices.insert(pos);
    }
} // namespace libchess::console
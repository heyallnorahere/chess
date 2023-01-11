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
#include "renderer/backends.h"

namespace libchess::console {
    struct cell_info_t {
        wchar_t character;
        uint32_t fg, bg;
    };

    struct key_callback_desc_t {
        key_callback_t callback;
        void* user_data;
    };

    struct renderer_info_t {
        cell_info_t* buffer;
        size_t buffer_size;
        int32_t width, height;

        renderer_backend_t backend;
        std::unordered_set<coord> rendered_indices;

        std::vector<std::optional<key_callback_desc_t>> key_callbacks;
        std::mutex key_callback_mutex;
    };

    static std::unique_ptr<renderer_info_t> s_renderer_info;
    static void renderer_key_thread() {
        char c;
        while ((c = s_renderer_info->backend.capture_character_blocking()) != -1) {
            util::mutex_lock lock(s_renderer_info->key_callback_mutex);

            for (const auto& callback : s_renderer_info->key_callbacks) {
                if (!callback.has_value()) {
                    continue;
                }

                const auto& data = callback.value();
                data.callback(c, data.user_data);
            }
        }
    }

    void renderer::init(int32_t width, int32_t height) {
        if (s_renderer_info) {
            throw std::runtime_error("renderer already initialized!");
        }

        s_renderer_info = std::make_unique<renderer_info_t>();
        get_renderer_backend(s_renderer_info->backend);

        s_renderer_info->width = width;
        s_renderer_info->height = height;

        s_renderer_info->buffer_size = sizeof(cell_info_t) * width * height;
        s_renderer_info->buffer = (cell_info_t*)malloc(s_renderer_info->buffer_size);

        if (s_renderer_info->buffer == nullptr) {
            throw std::runtime_error("failed to allocate renderer buffer!");
        }

        if (s_renderer_info->backend.save_cursor_pos != nullptr) {
            s_renderer_info->backend.save_cursor_pos();
        }

        if (s_renderer_info->backend.save_screen != nullptr) {
            s_renderer_info->backend.save_screen();
        }

        s_renderer_info->backend.clear_screen();
        s_renderer_info->backend.disable_cursor();

        if (s_renderer_info->backend.flush_console != nullptr) {
            s_renderer_info->backend.flush_console();
        }

        s_renderer_info->backend.setup_input_capture();

        std::thread thread(renderer_key_thread);
        s_renderer_info->backend.set_thread_name(thread, "Key capture thread");
        thread.detach();
    }

    void renderer::shutdown() {
        free(s_renderer_info->buffer);

        s_renderer_info->backend.cleanup_input_capture();
        s_renderer_info->backend.reset_color();
        s_renderer_info->backend.enable_cursor();

        if (s_renderer_info->backend.restore_screen != nullptr) {
            s_renderer_info->backend.restore_screen();
        } else {
            s_renderer_info->backend.clear_screen();
        }

        if (s_renderer_info->backend.restore_cursor_pos != nullptr) {
            s_renderer_info->backend.restore_cursor_pos();
        } else {
            s_renderer_info->backend.set_cursor_pos(coord(0, 0));
        }

        if (s_renderer_info->backend.flush_console != nullptr) {
            s_renderer_info->backend.flush_console();
        }

        s_renderer_info.reset();
    }

    void renderer::clear_screen() {
        s_renderer_info->rendered_indices.clear();
        for (size_t i = 0; i < (size_t)s_renderer_info->width * s_renderer_info->height; i++) {
            s_renderer_info->buffer[i].character = ' ';
            s_renderer_info->buffer[i].fg = color_default;
            s_renderer_info->buffer[i].bg = color_default;
        }

        s_renderer_info->backend.clear_screen();
        s_renderer_info->backend.set_cursor_pos(coord(0, 0));
        s_renderer_info->backend.set_color(color_default, color_default);

        if (s_renderer_info->backend.flush_console != nullptr) {
            s_renderer_info->backend.flush_console();
        }
    }

    void renderer::flush() {
        for (const auto& pos : s_renderer_info->rendered_indices) {
            size_t index = ((size_t)pos.y * s_renderer_info->width) + pos.x;
            const auto& cell = s_renderer_info->buffer[index];

            s_renderer_info->backend.set_cursor_pos(pos);
            s_renderer_info->backend.set_color(cell.fg, cell.bg);

            std::wcout << cell.character;
            if (s_renderer_info->backend.flush_console == nullptr) {
                std::cout << std::flush;
            }
        }

        if (s_renderer_info->backend.flush_console != nullptr) {
            s_renderer_info->backend.flush_console();
        }

        s_renderer_info->rendered_indices.clear();
    }

    void renderer::render(const coord& pos, wchar_t character, uint32_t fg, uint32_t bg) {
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

    size_t renderer::add_key_callback(key_callback_t callback, void* user_data) {
        util::mutex_lock lock(s_renderer_info->key_callback_mutex);

        std::optional<size_t> found_index;
        for (size_t i = 0; i < s_renderer_info->key_callbacks.size(); i++) {
            if (s_renderer_info->key_callbacks[i].has_value()) {
                continue;
            }

            found_index = i;
            break;
        }

        key_callback_desc_t data;
        data.callback = callback;
        data.user_data = user_data;

        if (found_index.has_value()) {
            size_t index = found_index.value();
            s_renderer_info->key_callbacks[index] = data;

            return index;
        } else {
            size_t index = s_renderer_info->key_callbacks.size();
            s_renderer_info->key_callbacks.emplace_back(data);

            return index;
        }
    }

    bool renderer::remove_key_callback(size_t index) {
        util::mutex_lock lock(s_renderer_info->key_callback_mutex);

        if (index >= s_renderer_info->key_callbacks.size()) {
            return false;
        }

        auto& callback = s_renderer_info->key_callbacks[index];
        if (!callback.has_value()) {
            return false;
        }

        callback.reset();
        return true;
    }
} // namespace libchess::console
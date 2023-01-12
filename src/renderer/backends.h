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
#include "../renderer.h"

namespace libchess::console {
    struct renderer_backend_t {
        void (*save_screen)();
        void (*restore_screen)();
        void (*clear_screen)();

        void (*save_cursor_pos)();
        void (*restore_cursor_pos)();
        void (*set_cursor_pos)(const coord&);

        void (*disable_cursor)();
        void (*enable_cursor)();

        void (*set_color)(uint32_t, uint32_t);
        void (*reset_color)();

        void (*verify_locale)();
        void (*flush_console)();

        void (*setup_input_capture)();
        void (*cleanup_input_capture)();
        char (*capture_character_blocking)();

        keystroke_type (*parse_keystroke)(char, void**);
        void (*destroy_keystroke_state)(void*);

        void (*set_thread_name)(std::thread&, const std::string&);
    };

    void populate_backend_functions(renderer_backend_t& backend);

    inline void get_renderer_backend(renderer_backend_t& backend) {
        memset(&backend, 0, sizeof(renderer_backend_t));
        populate_backend_functions(backend);
    }
}; // namespace libchess::console
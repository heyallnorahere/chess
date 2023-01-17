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
#include "coord.h"
#include "board.h"

#define LIBCHESS_BIND_METHOD(func)                                                                 \
    [this](auto&&... args) -> decltype(auto) { return func(std::forward<decltype(args)>(args)...); }

namespace libchess::util {
    enum string_split_options : uint32_t {
        string_split_options_none = 0,
        string_split_options_omit_empty = (1 << 0)
    };

    void split_string(const std::string& src, char delimiter, std::vector<std::string>& result,
                      uint32_t options = string_split_options_none);
    void split_string(const std::string& src, const std::string& delimiters,
                      std::vector<std::string>& result,
                      uint32_t options = string_split_options_none);

    bool parse_coordinate(const std::string& coordinate, coord& result);
    std::string serialize_coordinate(const coord& position);

    bool parse_piece(char character, piece_info_t& piece, bool parse_color = true);
    std::optional<char> serialize_piece(const piece_info_t& piece, bool serialize_color = true);

    class mutex_lock {
    public:
        mutex_lock(std::mutex& mutex) {
            m_mutex = &mutex;
            m_mutex->lock();
        }

        ~mutex_lock() { m_mutex->unlock(); }

        mutex_lock(const mutex_lock&) = delete;
        mutex_lock& operator=(const mutex_lock&) = delete;

    private:
        std::mutex* m_mutex;
    };
} // namespace libchess::util
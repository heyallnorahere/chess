/*
   Copyright 2022 Nora Beda

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
#include <stdexcept>
#include <utility>
#include <string>
#include <optional>

#ifdef assert
#undef assert
#endif

namespace assert {
    using message_t = std::optional<std::string>;

    inline void is_true(bool value, const message_t& message = {}) {
        if (!value) {
            throw std::runtime_error("assertion failed; " +
                                     message.value_or("value was not true!"));
        }
    }

    inline void is_false(bool value, const message_t& message = {}) {
        is_true(!value, message.value_or("value was not false!"));
    }

    template <typename T1, typename T2>
    inline void is_equal(const T1& lhs, const T2& rhs, const message_t& message = {}) {
        is_true(lhs == rhs, message.value_or("lhs and rhs were not equal!"));
    }

    template <typename T1, typename T2>
    inline void is_not_equal(const T1& lhs, const T2& rhs, const message_t& message = {}) {
        is_true(lhs != rhs, message.value_or("lhs and rhs were equal!"));
    }

    template <typename T>
    inline void is_nullptr(const T& value, const message_t& message = {}) {
        is_true(value == nullptr, message.value_or("value was not nullptr!"));
    }

    template <typename T>
    inline void is_not_nullptr(const T& value, const message_t& message = {}) {
        is_true(value != nullptr, message.value_or("value was nullptr!"));
    }
}; // namespace assert
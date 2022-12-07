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
    class assert_message {
    public:
        assert_message() = default;
        ~assert_message() = default;

        assert_message(std::nullopt_t) {}
        assert_message(const std::string& message) : m_message(message) {}

        assert_message& operator=(std::nullopt_t) {
            m_message.reset();
            return *this;
        }

        assert_message& operator=(const std::string& message) {
            m_message = message;
            return *this;
        }

        assert_message(const assert_message&) = default;
        assert_message& operator=(const assert_message&) = default;

        std::string message() const { return m_message.value(); }
        bool has_message() const { return m_message.has_value(); }

        std::string with_default(const std::string& default_message) const {
            return m_message.value_or(default_message);
        }

    private:
        std::optional<std::string> m_message;
    };

    class failed_assertion : public std::runtime_error {
    public:
        using _base = std::runtime_error;

        explicit failed_assertion(const std::string& message) : _base(message.c_str()) {}
        explicit failed_assertion(const char* message) : _base(message) {}
    };

    inline void is_true(bool value, const assert_message& message = {}) {
        if (!value) {
            throw failed_assertion(message.with_default("value was not true!"));
        }
    }

    inline void is_false(bool value, const assert_message& message = {}) {
        is_true(!value, message.with_default("value was not false!"));
    }

    template <typename T1, typename T2>
    inline void is_equal(const T1& lhs, const T2& rhs, const assert_message& message = {}) {
        is_true(lhs == rhs, message.with_default("lhs and rhs were not equal!"));
    }

    template <typename T1, typename T2>
    inline void is_not_equal(const T1& lhs, const T2& rhs, const assert_message& message = {}) {
        is_true(lhs != rhs, message.with_default("lhs and rhs were equal!"));
    }

    template <typename T>
    inline void is_nullptr(const T& value, const assert_message& message = {}) {
        is_true(value == nullptr, message.with_default("value was not nullptr!"));
    }

    template <typename T>
    inline void is_not_nullptr(const T& value, const assert_message& message = {}) {
        is_true(value != nullptr, message.with_default("value was nullptr!"));
    }
}; // namespace assert
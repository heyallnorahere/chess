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
#include <stddef.h>

#include <string>
#include <vector>
#include <utility>
#include <memory>

class test_fact {
public:
    test_fact() = default;
    virtual ~test_fact() = default;

    virtual void invoke() = 0;
};

class test_theory : public test_fact {
public:
    virtual void invoke() override;

    virtual void add_inline_data() = 0;
    virtual void invoke(const std::vector<std::string>& data) = 0;

protected:
    void inline_data(const std::vector<std::string>& data);

private:
    void clear_inline_data();

    std::vector<std::string> m_data;
    std::vector<size_t> m_data_indices;
};

template <typename T, typename... Args>
inline void invoke_check(Args&&... args) {
    static_assert(std::is_base_of_v<test_fact, T>,
                  "the passed type is not derived from test_fact or test_theory!");

    auto test_instance = std::unique_ptr<test_fact>(new T(std::forward<Args>(args)...));
    test_instance->invoke();
}
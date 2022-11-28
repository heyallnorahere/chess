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

#include "tests.h"
#include <stdexcept>

void test_theory::invoke() {
    clear_inline_data();
    add_inline_data();

    if (m_data_indices.empty()) {
        throw std::runtime_error("no inline data was added!");
    }

    for (size_t i = 0; i < m_data_indices.size(); i++) {
        size_t start = m_data_indices[i];

        auto start_it = m_data.begin();
        std::advance(start_it, start);

        size_t next = i + 1;
        std::vector<std::string>::iterator end_it;

        if (next < m_data_indices.size()) {
            size_t end = m_data_indices[next];

            end_it = m_data.begin();
            std::advance(end_it, end);
        } else {
            end_it = m_data.end();
        }

        std::vector<std::string> inline_data(start_it, end_it);
        invoke(inline_data);
    }
}

void test_theory::inline_data(const std::vector<std::string>& data) {
    m_data_indices.push_back(m_data.size());
    m_data.insert(m_data.end(), data.begin(), data.end());
}

void test_theory::clear_inline_data() {
    m_data_indices.clear();
    m_data.clear();
}
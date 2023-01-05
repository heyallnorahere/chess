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

#include "tests.h"
#include "assertions.h"

#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <map>

enum class test_result {
    passed,
    failed
};

class check_stats {
public:
    check_stats() = default;
    ~check_stats() = default;

    uint32_t& operator[](test_result key) { return m_result_counts[key]; }

    uint32_t operator[](test_result key) const {
        if (m_result_counts.find(key) != m_result_counts.end()) {
            return m_result_counts.at(key);
        } else {
            return 0;
        }
    }

    uint32_t cumulative() const {
        uint32_t result = 0;
        for (auto [key, value] : m_result_counts) {
            result += value;
        }

        return result;
    }

    void reset() { m_result_counts.clear(); }

private:
    std::map<test_result, uint32_t> m_result_counts;
};

static check_stats s_check_stats;
void test_check::test_init() {
    s_check_stats.reset();
}

int test_check::test_cleanup() {
    std::cout << s_check_stats[test_result::passed] << " checks passed ; ";
    std::cout << s_check_stats[test_result::failed] << " checks failed ; ";
    std::cout << s_check_stats.cumulative() << " checks ran" << std::endl;

    return s_check_stats[test_result::failed] > 0 ? 1 : 0;
}

void test_check::catch_check(const std::function<void()>& check,
                             const std::optional<std::string>& check_suffix) {
    std::string check_name = get_check_name();
    if (check_suffix.has_value()) {
        check_name += ": " + check_suffix.value();
    }

    std::cout << check_name << " - ";
    std::string failure_message;
    try {
        check();
        std::cout << "PASSED" << std::endl;

        s_check_stats[test_result::passed]++;
        return;
    } catch (const assert::failed_assertion& assertion) {
        failure_message = std::string("assertion failed: ") + assertion.what();
    } catch (const std::exception& exc) {
        failure_message = std::string("exception thrown: ") + exc.what();
    }

    std::cout << "FAILED" << std::endl;
    std::cout << "\t" << failure_message << std::endl;

    s_check_stats[test_result::failed]++;
}

void test_fact::invoke_check() {
    catch_check([this]() mutable { invoke(); });
}

void test_theory::invoke_check() {
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
        std::stringstream suffix;

        suffix << "{ ";
        for (size_t i = 0; i < inline_data.size(); i++) {
            if (i > 0) {
                suffix << ", ";
            }

            suffix << std::quoted(inline_data[i]);
        }

        suffix << " }";
        catch_check([&]() { invoke(inline_data); }, suffix.str());
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
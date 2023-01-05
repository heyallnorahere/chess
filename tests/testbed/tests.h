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
#include <stddef.h>

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <functional>
#include <optional>

class test_check {
public:
    static void test_init();
    static int test_cleanup();

    test_check() = default;
    virtual ~test_check() = default;

    virtual void invoke_check() = 0;

protected:
    void catch_check(const std::function<void()>& check,
                     const std::optional<std::string>& check_suffix = {});

    virtual std::string get_check_name() = 0;
};

class test_fact : public test_check {
public:
    test_fact() = default;
    virtual ~test_fact() override = default;

    virtual void invoke_check() override;

protected:
    virtual void invoke() = 0;
};

class test_theory : public test_check {
public:
    test_theory() = default;
    virtual ~test_theory() override = default;

    virtual void invoke_check() override;

protected:
    virtual void add_inline_data() = 0;
    virtual void invoke(const std::vector<std::string>& data) = 0;

    void inline_data(const std::vector<std::string>& data);

private:
    void clear_inline_data();

    std::vector<std::string> m_data;
    std::vector<size_t> m_data_indices;
};
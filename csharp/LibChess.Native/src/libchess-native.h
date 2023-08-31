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
#include <libchess.h>

#ifdef LIBCHESS_COMPILER_MSVC
#define LIBCHESS_API __declspec(dllexport)
#else
#define LIBCHESS_API
#endif

struct native_board_t {
    std::shared_ptr<libchess::board> instance;
};

struct native_engine_t {
    libchess::engine instance;
};
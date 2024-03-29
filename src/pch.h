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

#include <cstdint> // int32_t, uint32_t, etc.
#include <stddef.h> // ::size_t
#include <cstring> // memset

#include <vector>
#include <memory>
#include <map>
#include <iostream>
#include <stdexcept>
#include <unordered_set>
#include <thread>
#include <mutex>
#include <string>
#include <optional>
#include <functional>
#include <utility>
#include <algorithm>
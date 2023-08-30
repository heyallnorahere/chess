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

#include "libchess-native.h"

extern "C" {

const char* SerializeCoordinate(const libchess::coord* coord) {
    auto result = libchess::util::serialize_coordinate(*coord);

    size_t length = result.length();
    size_t size = (length + 1) * sizeof(char);
    char* buffer = (char*)malloc(size);

    buffer[length] = '\0';
    memcpy(buffer, result.c_str(), size - sizeof(char));

    return buffer;
}

bool ParseCoordinate(const char* src, libchess::coord* result) {
    return libchess::util::parse_coordinate(src, *result);
}

} // end of p/invoke block
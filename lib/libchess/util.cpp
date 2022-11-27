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

#include "libchesspch.h"
#include "util.h"
#include "board.h"

namespace libchess::util {
    void split_string(const std::string& src, char delimiter, std::vector<std::string>& result,
                      uint32_t options) {
        split_string(src, std::string(&delimiter, 1), result, options);
    }

    void split_string(const std::string& src, const std::string& delimiters,
                      std::vector<std::string>& result, uint32_t options) {
        bool omit_empty = (options & string_split_options_omit_empty) != string_split_options_none;
        result.clear();

        size_t last_position, offset = 0;
        while ((last_position = src.find_first_of(delimiters, offset)) != std::string::npos) {
            std::string segment = src.substr(offset, last_position - offset);
            offset = last_position + 1;

            if (!omit_empty || segment.length() > 0) {
                result.push_back(segment);
            }
        }

        std::string last_segment = src.substr(offset);
        if (!omit_empty || last_segment.length() > 0) {
            result.push_back(last_segment);
        }
    }

    bool parse_coordinate(const std::string& coordinate, coord& result) {
        static const std::regex filter("[a-hA-H][1-8]");
        if (!std::regex_match(coordinate, filter)) {
            return false;
        }

        char x_char = (char)std::tolower((int)coordinate[0]);
        result.x = (int32_t)x_char - (int32_t)'a';

        std::string y_string = coordinate.substr(1);
        result.y = (int32_t)std::atoi(y_string.c_str()) - 1;

        return true;
    }

    std::string serialize_coordinate(const coord& position) {
        std::string result;

        if (!board::is_out_of_bounds(position)) {
            std::stringstream stream;

            stream << 'a' + (char)position.x;
            stream << position.y + 1;

            result = stream.str();
        }

        return result;
    }
} // namespace libchess::util
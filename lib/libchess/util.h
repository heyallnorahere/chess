#pragma once
#include "coord.h"

namespace libchess::util {
    enum string_split_options : uint32_t {
        string_split_options_none = 0,
        string_split_options_omit_empty = (1 << 0)
    };

    void split_string(const std::string& src, char delimiter, std::vector<std::string>& result,
                      uint32_t options = string_split_options_none);
    void split_string(const std::string& src, const std::string& delimiters,
                      std::vector<std::string>& result,
                      uint32_t options = string_split_options_none);

    bool parse_coordinate(const std::string& coordinate, coord& result);
} // namespace libchess::util
#pragma once

#include <string>

namespace utils {

    std::string indent(const uint16_t depth, const char chr=' ') {
        std::string str(depth, chr);
    }
}

#include "utils.hpp"

namespace utils {
    std::string indent(const size_t depth, const char chr) {
        std::string str;
        for(size_t i=0; i<depth; i++) str += chr;
        return str;
    }
}

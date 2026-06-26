#include "core/string_table.h"
#include <string>
#include <cstring>

StringTable& StringTable::instance() {
    static StringTable st;
    return st;
}

const char* StringTable::insert(const char* str) {
    if (!str) return "";
    auto h = hash(str);
    auto it = table.find(h);
    if (it != table.end()) return it->second.c_str();
    auto& s = table[h] = str;
    return s.c_str();
}

const char* StringTable::insert(std::string_view sv) {
    return insert(std::string(sv).c_str());
}

uint32_t StringTable::hash(const char* str) const {
    uint32_t h = 0;
    while (*str) {
        h = (h << 5) - h + (unsigned char)*str++;
    }
    return h;
}

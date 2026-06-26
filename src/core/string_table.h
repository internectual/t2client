#pragma once
#include <cstdint>
#include <string_view>
#include <unordered_map>

class StringTable {
public:
    static StringTable& instance();
    const char* insert(const char* str);
    const char* insert(std::string_view sv);
    uint32_t hash(const char* str) const;

private:
    StringTable() = default;
    std::unordered_map<uint32_t, std::string> table;
};

#pragma once
#include "fs/file_system.h"

class Vl2Archive : public Archive {
public:
    Vl2Archive();
    ~Vl2Archive() override;

    bool open(const char* path) override;
    bool readFile(const char* path, std::vector<uint8_t>& data) override;
    bool fileExists(const char* path) const override;
    void listFiles(const char* pattern, std::vector<std::string>& out) const override;

private:
    struct Impl;
    Impl* impl;
};

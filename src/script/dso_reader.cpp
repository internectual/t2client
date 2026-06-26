#include "script/dso_reader.h"
#include "core/console.h"
#include <cstring>
#include <fstream>
#include <vector>

DSOReader::DSOReader() {}

static const char* opcodeNames[] = {
    "PushInt", "PushFloat", "PushString", "PushVar", "Dup", "Drop", "Swap",
    "Add", "Sub", "Mul", "Div", "Mod", "Neg", "Inc", "Dec",
    "CmpEq", "CmpNe", "CmpGt", "CmpGe", "CmpLt", "CmpLe",
    "And", "Or", "Not",
    "Jmp", "JmpZ", "JmpNZ", "Call", "CallMethod", "Return",
    "SetVar", "GetVar", "SetField", "GetField", "SetInternal", "GetInternal",
    "NewObject", "DeleteObject", "SetName", "GetName",
    "StrCat", "StrCmp", "StrLen", "StrSub", "StrPos",
    "Assert", "Breakpoint", "Nop", "Halt"
};

const char* DSOReader::opcodeName(DSOOpcode op) const {
    uint8_t o = (uint8_t)op;
    if (o >= 0x01 && o <= 0x06) return opcodeNames[o - 0x01];
    if (o >= 0x10 && o <= 0x17) return opcodeNames[o - 0x10 + 7];
    if (o >= 0x20 && o <= 0x25) return opcodeNames[o - 0x20 + 15];
    if (o >= 0x30 && o <= 0x32) return opcodeNames[o - 0x30 + 21];
    if (o >= 0x40 && o <= 0x45) return opcodeNames[o - 0x40 + 24];
    if (o >= 0x50 && o <= 0x55) return opcodeNames[o - 0x50 + 30];
    if (o >= 0x60 && o <= 0x63) return opcodeNames[o - 0x60 + 36];
    if (o >= 0x70 && o <= 0x74) return opcodeNames[o - 0x70 + 40];
    if (o >= 0x80 && o <= 0x82) return opcodeNames[o - 0x80 + 45];
    if (o == 0xFF) return "Halt";
    return "Unknown";
}

uint32_t DSOReader::readU32(const uint8_t*& ptr, size_t& remaining) {
    if (remaining < 4) return 0;
    uint32_t v = *(uint32_t*)ptr;
    ptr += 4; remaining -= 4;
    return v;
}

int32_t DSOReader::readS32(const uint8_t*& ptr, size_t& remaining) {
    return (int32_t)readU32(ptr, remaining);
}

float DSOReader::readF32(const uint8_t*& ptr, size_t& remaining) {
    if (remaining < 4) return 0;
    float v = *(float*)ptr;
    ptr += 4; remaining -= 4;
    return v;
}

std::string DSOReader::readString(const uint8_t*& ptr, size_t& remaining) {
    if (remaining < 2) return "";
    uint16_t len = *(uint16_t*)ptr;
    ptr += 2; remaining -= 2;
    if (remaining < len) len = remaining;
    std::string s((char*)ptr, len);
    ptr += len; remaining -= len;
    return s;
}

bool DSOReader::read(const uint8_t* data, size_t size, DSOFile& out) {
    const uint8_t* ptr = data;
    size_t remaining = size;

    // DSO signature "TES "
    if (remaining < 4 || memcmp(ptr, "TES ", 4) != 0) {
        Console::instance().printf(LogLevel::Warn, "Invalid DSO signature");
        return false;
    }
    ptr += 4; remaining -= 4;

    out.version = readU32(ptr, remaining);
    out.name = readString(ptr, remaining);

    uint32_t numFuncs = readU32(ptr, remaining);
    for (uint32_t i = 0; i < numFuncs; i++) {
        DSOFunction func;
        func.name = readString(ptr, remaining);
        func.package = readString(ptr, remaining);
        func.address = readU32(ptr, remaining);
        func.argc = readS32(ptr, remaining);
        func.hasVarArgs = readU32(ptr, remaining) != 0;
        uint32_t numArgNames = readU32(ptr, remaining);
        for (uint32_t j = 0; j < numArgNames; j++)
            func.argNames.push_back(readString(ptr, remaining));
        out.functions.push_back(func);
    }

    // Read constants
    uint32_t numConsts = readU32(ptr, remaining);
    for (uint32_t i = 0; i < numConsts; i++) {
        DSOConstant c;
        uint32_t type = readU32(ptr, remaining);
        if (type == 0) {
            c.type = DSOConstant::Int;
            c.i = readS32(ptr, remaining);
        } else if (type == 1) {
            c.type = DSOConstant::Float;
            c.f = readF32(ptr, remaining);
        } else if (type == 2) {
            c.type = DSOConstant::String;
            c.str = readString(ptr, remaining);
        }
        out.constants.push_back(c);
    }

    // Read bytecode
    uint32_t codeSize = readU32(ptr, remaining);
    const uint8_t* codeStart = ptr;
    uint32_t codeEnd = 0;
    while (codeEnd < codeSize) {
        // Skip for now, store raw
        uint8_t op = ptr[codeEnd];
        codeEnd++;
        // Skip args based on opcode
        // For now just read all bytes
    }
    ptr += codeSize; remaining -= codeSize;

    Console::instance().printf(LogLevel::Debug, "DSO: %s v%u, %zu funcs, %zu consts, %u bytes code",
        out.name.c_str(), out.version, out.functions.size(), out.constants.size(), codeSize);
    return true;
}

bool DSOReader::readFromFile(const char* path, DSOFile& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), {});
    return read(data.data(), data.size(), out);
}

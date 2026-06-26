#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>

enum class DSOOpcode : uint8_t {
    // Stack operations
    PushInt     = 0x01,
    PushFloat   = 0x02,
    PushString  = 0x03,
    PushVar     = 0x04,
    Dup         = 0x05,
    Drop        = 0x06,
    Swap        = 0x07,

    // Arithmetic
    Add         = 0x10,
    Sub         = 0x11,
    Mul         = 0x12,
    Div         = 0x13,
    Mod         = 0x14,
    Neg         = 0x15,
    Inc         = 0x16,
    Dec         = 0x17,

    // Comparison
    CmpEq       = 0x20,
    CmpNe       = 0x21,
    CmpGt       = 0x22,
    CmpGe       = 0x23,
    CmpLt       = 0x24,
    CmpLe       = 0x25,

    // Logical
    And         = 0x30,
    Or          = 0x31,
    Not         = 0x32,

    // Control flow
    Jmp         = 0x40,
    JmpZ        = 0x41,
    JmpNZ       = 0x42,
    Call        = 0x43,
    CallMethod  = 0x44,
    Return      = 0x45,

    // Variables
    SetVar      = 0x50,
    GetVar      = 0x51,
    SetField    = 0x52,
    GetField    = 0x53,
    SetInternal = 0x54,
    GetInternal = 0x55,

    // Objects
    NewObject   = 0x60,
    DeleteObject= 0x61,
    SetName     = 0x62,
    GetName     = 0x63,

    // Strings
    StrCat      = 0x70,
    StrCmp      = 0x71,
    StrLen      = 0x72,
    StrSub      = 0x73,
    StrPos      = 0x74,

    // Misc
    Assert      = 0x80,
    Breakpoint  = 0x81,
    Nop         = 0x82,
    Halt        = 0xFF,
};

struct DSOConstant {
    enum Type { Int, Float, String };
    Type type;
    union { int32_t i; float f; };
    std::string str;
};

struct DSOInstruction {
    DSOOpcode opcode;
    std::vector<uint32_t> args;
};

struct DSOFunction {
    std::string name;
    std::string package;
    uint32_t address;
    int32_t argc;
    bool hasVarArgs;
    std::vector<std::string> argNames;
};

struct DSOFile {
    uint32_t version;
    std::string name;
    std::vector<DSOConstant> constants;
    std::vector<DSOInstruction> instructions;
    std::vector<DSOFunction> functions;
    std::unordered_map<uint32_t, uint32_t> stringTable; // addr -> index
};

class DSOReader {
public:
    DSOReader();

    bool read(const uint8_t* data, size_t size, DSOFile& out);
    bool readFromFile(const char* path, DSOFile& out);

    const char* opcodeName(DSOOpcode op) const;

private:
    bool readHeader(const uint8_t*& ptr, size_t& remaining, DSOFile& out);
    bool readConstants(const uint8_t*& ptr, size_t& remaining, DSOFile& out);
    bool readCode(const uint8_t*& ptr, size_t& remaining, DSOFile& out);
    uint32_t readU32(const uint8_t*& ptr, size_t& remaining);
    int32_t readS32(const uint8_t*& ptr, size_t& remaining);
    float readF32(const uint8_t*& ptr, size_t& remaining);
    std::string readString(const uint8_t*& ptr, size_t& remaining);
};

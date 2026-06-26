#pragma once
#include "script/dso_reader.h"
#include "core/console.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <stack>
#include <functional>

class VMValue {
public:
    enum Type { None, Int, Float, String };

    Type type = None;
    union { int32_t i = 0; float f; };
    std::string str;

    VMValue() = default;
    VMValue(int32_t v) : type(Int), i(v) {}
    VMValue(float v) : type(Float), f(v) {}
    VMValue(const char* v) : type(String), str(v ? v : "") {}
    VMValue(const std::string& v) : type(String), str(v) {}

    int32_t toInt() const;
    float toFloat() const;
    std::string toString() const;
    bool toBool() const;
};

class ScriptObject {
public:
    std::string className;
    std::string name;
    std::unordered_map<std::string, VMValue> fields;
    std::unordered_map<std::string, VMValue> internals;
};

class ScriptEngine;
class VirtualMachine {
public:
    VirtualMachine(ScriptEngine* engine);
    ~VirtualMachine();

    bool loadScript(const uint8_t* data, size_t size, const char* name = nullptr);
    bool loadScriptFile(const char* path);

    VMValue callFunction(const char* name, const std::vector<VMValue>& args = {});
    VMValue callMethod(const char* objName, const char* method, const std::vector<VMValue>& args = {});

    void setVariable(const char* name, const VMValue& val);
    VMValue getVariable(const char* name);

    void registerNativeFunction(const char* name, std::function<VMValue(const std::vector<VMValue>&)> fn);

    void run(uint32_t address);
    VMValue execute(const std::vector<VMValue>& args);

    void dumpState();

private:
    struct Impl;
    Impl* impl;
};

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    bool init();
    void shutdown();

    void executeString(const char* script);
    void executeFile(const char* path);

    // Compile .cs to .dso and execute
    bool compileAndRun(const char* csPath);

    VirtualMachine* vm() { return vmInstance; }
    Console& getConsole() { return *con; }

    using NativeFunction = std::function<VMValue(const std::vector<VMValue>&)>;
    void registerFunction(const char* name, NativeFunction fn);

    void addObject(ScriptObject* obj);
    ScriptObject* findObject(const char* name);

private:
    VirtualMachine* vmInstance{};
    Console* con{};
    std::unordered_map<std::string, ScriptObject*> objects;
};

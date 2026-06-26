#include "script/script_engine.h"
#include "core/console.h"
#include "core/engine.h"
#include <fstream>
#include <stack>
#include <unordered_map>
#include <functional>

int32_t VMValue::toInt() const {
    switch (type) {
        case Int: return i;
        case Float: return (int32_t)f;
        case String: return atoi(str.c_str());
        default: return 0;
    }
}

float VMValue::toFloat() const {
    switch (type) {
        case Int: return (float)i;
        case Float: return f;
        case String: return atof(str.c_str());
        default: return 0.0f;
    }
}

std::string VMValue::toString() const {
    switch (type) {
        case Int: return std::to_string(i);
        case Float: return std::to_string(f);
        case String: return str;
        default: return "";
    }
}

bool VMValue::toBool() const {
    switch (type) {
        case Int: return i != 0;
        case Float: return f != 0.0f;
        case String: return !str.empty() && str != "0" && str != "false";
        default: return false;
    }
}

struct CompiledScript {
    DSOFile dso;
    std::vector<uint8_t> bytecode;
    std::unordered_map<std::string, DSOFunction*> funcMap;
};

struct VirtualMachine::Impl {
    ScriptEngine* engine;
    std::vector<CompiledScript*> scripts;
    std::stack<VMValue> stack;
    std::unordered_map<std::string, VMValue> globals;
    std::unordered_map<std::string, std::function<VMValue(const std::vector<VMValue>&)>> natives;

    // Call stack
    struct Frame {
        CompiledScript* script{};
        uint32_t ip{};
        std::vector<VMValue> locals;
        std::vector<VMValue> args;
        VMValue result;
    };
    std::stack<Frame> callStack;
    std::vector<ScriptObject*> objects;

    Impl(ScriptEngine* e) : engine(e) {}
};

VirtualMachine::VirtualMachine(ScriptEngine* engine) : impl(new Impl(engine)) {}
VirtualMachine::~VirtualMachine() { delete impl; }

bool VirtualMachine::loadScript(const uint8_t* data, size_t size, const char* name) {
    DSOReader reader;
    auto* script = new CompiledScript;
    if (!reader.read(data, size, script->dso)) {
        delete script;
        return false;
    }
    script->bytecode.assign(data, data + size);
    for (auto& f : script->dso.functions)
        script->funcMap[f.name] = &f;
    impl->scripts.push_back(script);
    return true;
}

bool VirtualMachine::loadScriptFile(const char* path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    std::vector<uint8_t> data((std::istreambuf_iterator<char>(f)), {});
    return loadScript(data.data(), data.size(), path);
}

VMValue VirtualMachine::callFunction(const char* name, const std::vector<VMValue>& args) {
    // Check natives first
    auto nit = impl->natives.find(name);
    if (nit != impl->natives.end()) {
        return nit->second(args);
    }

    // Find in scripts
    for (auto* script : impl->scripts) {
        auto fit = script->funcMap.find(name);
        if (fit != script->funcMap.end()) {
            impl->callStack.push({script, fit->second->address, {}, args, {}});
            return execute(args);
        }
    }

    Console::instance().printf(LogLevel::Warn, "VM: function not found: %s", name);
    return {};
}

VMValue VirtualMachine::execute(const std::vector<VMValue>& args) {
    if (impl->callStack.empty()) return {};
    auto& frame = impl->callStack.top();
    auto* script = frame.script;
    const auto& dso = script->dso;

    // Minimal VM execution - for now return 0
    // Full VM implementation would parse and execute bytecode here
    Console::instance().printf(LogLevel::Debug, "VM: executing %s (%zu args)",
        !dso.functions.empty() ? dso.functions[0].name.c_str() : "unknown", args.size());

    impl->callStack.pop();
    return VMValue(0);
}

void VirtualMachine::setVariable(const char* name, const VMValue& val) {
    impl->globals[name] = val;
}

VMValue VirtualMachine::getVariable(const char* name) {
    auto it = impl->globals.find(name);
    if (it != impl->globals.end()) return it->second;

    // Check console variables
    auto* cvar = Console::instance().find(name);
    if (cvar && cvar->type == Console::ConsoleItem::Variable)
        return VMValue(cvar->value.c_str());

    return {};
}

void VirtualMachine::registerNativeFunction(const char* name,
    std::function<VMValue(const std::vector<VMValue>&)> fn) {
    impl->natives[name] = std::move(fn);
}

ScriptEngine::ScriptEngine() : con(&Console::instance()) {}
ScriptEngine::~ScriptEngine() {}

bool ScriptEngine::init() {
    vmInstance = new VirtualMachine(this);

    // Register core native functions
    vmInstance->registerNativeFunction("echo", [](const auto& args) {
        std::string msg;
        for (auto& a : args) {
            if (!msg.empty()) msg += " ";
            msg += a.toString();
        }
        Console::instance().printf(LogLevel::Info, "%s", msg.c_str());
        return VMValue(1);
    });

    vmInstance->registerNativeFunction("getVariable", [this](const auto& args) {
        if (args.empty()) return VMValue();
        return vmInstance->getVariable(args[0].toString().c_str());
    });

    vmInstance->registerNativeFunction("setVariable", [this](const auto& args) {
        if (args.size() >= 2) {
            vmInstance->setVariable(args[0].toString().c_str(),
                args[1].toString().c_str());
        }
        return VMValue(1);
    });

    return true;
}

void ScriptEngine::shutdown() {
    delete vmInstance;
}

void ScriptEngine::executeString(const char* script) {
    Console::instance().execute(script);
}

void ScriptEngine::executeFile(const char* path) {
    Console::instance().executeFile(path);
}

void ScriptEngine::registerFunction(const char* name, NativeFunction fn) {
    if (vmInstance) vmInstance->registerNativeFunction(name, std::move(fn));
}

void ScriptEngine::addObject(ScriptObject* obj) {
    objects[obj->name] = obj;
}

ScriptObject* ScriptEngine::findObject(const char* name) {
    auto it = objects.find(name);
    return it != objects.end() ? it->second : nullptr;
}

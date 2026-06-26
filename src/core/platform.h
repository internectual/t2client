#pragma once
#include <cstdint>
#include <functional>
#include <string>

struct PlatformConfig {
    std::string title = "Tribes 2";
    int32_t width = 1024;
    int32_t height = 768;
    bool fullscreen = false;
    bool vsync = true;
    int32_t msaaSamples = 0;
};

struct InputState {
    bool keysDown[512]{};
    bool mouseButtons[8]{};
    int32_t mouseX{}, mouseY{}, mouseDeltaX{}, mouseDeltaY{};
    int32_t mouseWheel{};
};

class Platform {
public:
    Platform();
    ~Platform();

    bool init(const PlatformConfig& config);
    void shutdown();
    bool processEvents();
    void swapBuffers();
    bool isRunning() const;

    InputState& input() { return inputState; }

    int32_t width() const;
    int32_t height() const;
    float aspect() const;
    double time() const;
    uint64_t frameCount() const;

    void setTitle(const char* title);
    void showMouse(bool show);
    void setMousePos(int32_t x, int32_t y);
    void setRelativeMouse(bool relative);

    void* nativeWindow();
    void* nativeGLContext();

    using ResizeCallback = std::function<void(int32_t w, int32_t h)>;
    void setResizeCallback(ResizeCallback cb);

private:
    struct Impl;
    Impl* impl;
    InputState inputState;
    bool running = false;
};

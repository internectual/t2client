#include "game/hud.h"
#include "game/game.h"
#include "render/renderer.h"
#include "core/engine.h"
#include <algorithm>
#include <vector>

struct HUD::Impl {
    std::vector<std::pair<std::string, double>> messages;
    std::vector<std::string> chatLines;
    double messageStart = 0;
};

HUD::HUD() : impl(new Impl) {}
HUD::~HUD() { delete impl; }

void HUD::init() {}

void HUD::render(Game* game) {
    if (!visible || !game || game->state() != Game::Playing) return;

    auto& r = Engine::instance().renderer();
    auto* font = r.getFont();
    if (!font) return;

    int32_t w = Engine::instance().platform().width();
    int32_t h = Engine::instance().platform().height();

    // Crosshair
    renderCrosshair();

    // Health
    renderHealthBar(game->player().health(), 100.0f);

    // Energy
    renderEnergyBar(game->player().energy(), 100.0f);

    // HUD text
    char buf[128];
    snprintf(buf, sizeof(buf), "HP: %.0f", game->player().health());
    if (font) font->render(buf, 20.0f, h - 60.0f, {1, 1, 1, 1}, 1.0f);

    snprintf(buf, sizeof(buf), "EN: %.0f", game->player().energy());
    if (font) font->render(buf, 20.0f, h - 40.0f, {0, 1, 1, 1}, 1.0f);

    // Messages
    double now = Engine::instance().timer().now();
    for (size_t i = 0; i < impl->messages.size(); i++) {
        auto& msg = impl->messages[i];
        double age = now - msg.second;
        if (age < 3.0) {
            float alpha = (float)(1.0 - age / 3.0);
            if (font) font->render(msg.first.c_str(), w * 0.5f - 100, h * 0.3f + i * 25,
                                   {1, 1, 1, alpha}, 1.0f);
        }
    }

    // Clean old messages
    impl->messages.erase(
        std::remove_if(impl->messages.begin(), impl->messages.end(),
            [now](auto& m) { return (now - m.second) > 3.0; }),
        impl->messages.end()
    );
}

void HUD::renderCrosshair() {
    auto& r = Engine::instance().renderer();
    int32_t w = Engine::instance().platform().width() / 2;
    int32_t h = Engine::instance().platform().height() / 2;
    float s = 8.0f;

    r.drawLine({(float)w - s, (float)h, 0}, {(float)w + s, (float)h, 0}, {0, 1, 0, 1});
    r.drawLine({(float)w, (float)h - s, 0}, {(float)w, (float)h + s, 0}, {0, 1, 0, 1});
}

void HUD::renderHealthBar(float health, float maxHealth) {
    // Simplified - draws rect
}

void HUD::renderEnergyBar(float energy, float maxEnergy) {}

void HUD::renderAmmo(int32_t current, int32_t max) {}

void HUD::renderScoreboard(Game* game) {}

void HUD::renderMessage(const char* text, float duration) {
    impl->messages.push_back({text, Engine::instance().timer().now()});
}

void HUD::showMessage(const char* text, const ColorF& color) {
    impl->messages.push_back({text, Engine::instance().timer().now()});
}

void HUD::addChatLine(const char* text) {
    impl->chatLines.push_back(text);
    if (impl->chatLines.size() > 50)
        impl->chatLines.erase(impl->chatLines.begin());
}

void Menu::init() {}

void Menu::update(float dt) {
    // Menu logic
}

void Menu::render() {
    auto& r = Engine::instance().renderer();
    auto* font = r.getFont();
    int32_t w = Engine::instance().platform().width();
    int32_t h = Engine::instance().platform().height();

    // Simple menu rendering
    if (!active) return;

    switch (currentScreen) {
        case Main: {
            const char* title = "TRIBES 2";
            if (font) font->render(title, w * 0.5f - 80, 100, {1, 1, 0, 1}, 2.0f);

            const char* items[] = {"Start Local Game", "Server Browser", "Settings", "Controls", "Quit"};
            for (int i = 0; i < 5; i++) {
                if (font) font->render(items[i], w * 0.5f - 80, 200 + i * 40, {1, 1, 1, 1}, 1.2f);
            }
            break;
        }
        case ServerBrowser: {
            if (font) font->render("Server Browser", 20, 20, {1, 1, 0, 1}, 1.5f);
            for (size_t i = 0; i < servers.size(); i++) {
                char buf[256];
                snprintf(buf, sizeof(buf), "%s | %s | %d/%d | %dms",
                    servers[i].name.c_str(), servers[i].map.c_str(),
                    servers[i].players, servers[i].maxPlayers, servers[i].ping);
                if (font) font->render(buf, 20, 80 + i * 30, {1, 1, 1, 1}, 1.0f);
            }
            break;
        }
        default: break;
    }
}

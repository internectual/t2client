#include "game/game.h"
#include "game/physics.h"
#include "render/renderer.h"
#include "render/shader.h"
#include "core/console.h"
#include "core/engine.h"
#include "fs/file_system.h"
#include <cmath>

Player::Player() {}
Player::~Player() {}

void Player::update(float dt) {
    // Physics handled by Game::update
}

void Player::render() {
    // Placeholder - render a simple box
    auto& r = Engine::instance().renderer();
    Box3F box = {{pos.x - 0.4f, pos.y - 1.0f, pos.z - 0.4f},
                 {pos.x + 0.4f, pos.y + 1.0f, pos.z + 0.4f}};
    r.drawBox(box, {0, 1, 0, 1});
}

void Player::applyMove(const Point3F& move, bool jump, bool jet) {
    // Movement handled by Physics system
}

void Player::applyDamage(float amount) {
    if (arm > 0) {
        float absorbed = Math::min(arm, amount * 0.6f);
        arm -= absorbed;
        hp -= amount - absorbed;
    } else {
        hp -= amount;
    }
    if (hp <= 0) { hp = 0; /* handle death */ }
}

void Player::respawn() {
    hp = 100.0f;
    eng = 100.0f;
    arm = 0.0f;
    vel = {0,0,0};
    pos = {0, 10, 0};
    onGround = false;
}

Point3F Player::cameraPos() const {
    return {pos.x, pos.y + eyeHeight, pos.z};
}

Point3F Player::cameraTarget() const {
    float cx = pos.x + std::sin(rot.z) * std::cos(rot.x);
    float cy = pos.y + eyeHeight + std::sin(rot.x);
    float cz = pos.z + std::cos(rot.z) * std::cos(rot.x);
    return {cx, cy, cz};
}

World::World() {}
World::~World() {}

bool World::load(const char* mapName) {
    Console::instance().printf(LogLevel::Info, "Loading map: %s", mapName);

    // Load terrain
    std::string terrainPath = std::string("missions/") + mapName + ".dif";
    auto data = Engine::instance().fs().read(terrainPath.c_str());
    if (!data.empty()) {
        terrainBlock.load(data.data(), data.size());
    } else {
        // Generate default terrain
        terrainBlock.load(nullptr, 0);
    }
    terrainBlock.loaded = true;

    // Load sky
    auto& fs = Engine::instance().fs();
    std::vector<std::string> skyFaces = {
        "skies/default/right.jpg", "skies/default/left.jpg",
        "skies/default/top.jpg", "skies/default/bottom.jpg",
        "skies/default/front.jpg", "skies/default/back.jpg"
    };
    skyBox.load(skyFaces);

    loaded = true;
    return true;
}

void World::update(float dt) {
    // Update world objects, animations, etc.
}

void World::render(const Point3F& cameraPos) {
    if (!loaded) return;

    auto& r = Engine::instance().renderer();

    // Render terrain
    if (terrainBlock.loaded) {
        ShaderManager::getTerrainShader()->bind();
        terrainBlock.render(cameraPos);
    }

    // Render world objects
    auto* dshader = ShaderManager::getDefaultShader();
    dshader->bind();

    for (auto& obj : worldObjects) {
        if (obj.shape && obj.shape->loaded) {
            MatrixF model;
            model.setTranslation(obj.pos);
            obj.shape->render(0);
        }
    }

    // Render sky
    skyBox.render(r.view, r.projection);
}

void World::addObject(const WorldObject& obj) {
    worldObjects.push_back(obj);
}

float World::getHeight(float x, float z) const {
    if (!terrainBlock.loaded || terrainBlock.heights.empty()) return 0;

    int tx = (int)((x / (float)terrainBlock.size + 0.5f) * terrainBlock.size);
    int tz = (int)((z / (float)terrainBlock.size + 0.5f) * terrainBlock.size);
    tx = Math::clamp(tx, 0, terrainBlock.size - 1);
    tz = Math::clamp(tz, 0, terrainBlock.size - 1);
    return terrainBlock.heights[tz * terrainBlock.size + tx] * terrainBlock.heightScale;
}

Game::Game() : pl(new Player), w(new World) {}
Game::~Game() { delete pl; delete w; }

bool Game::init() {
    Console::instance().printf(LogLevel::Info, "Game initialized");

    // Register game console commands
    auto& con = Console::instance();
    con.addCommand("startLocal", [this](int32_t argc, const char* const* argv) {
        startLocalGame();
    });

    con.addCommand("connect", [this](int32_t argc, const char* const* argv) {
        if (argc > 1) connectToServer(argv[1], argc > 2 ? (uint16_t)atoi(argv[2]) : 28000);
    });

    return true;
}

void Game::shutdown() {}

void Game::update(float dt) {
    time += dt;

    if (gameState == Playing) {
        // Update physics
        Physics physics;
        physics.update(pl, dt, currentInput);

        // Update world
        w->update(dt);
    }
}

void Game::render(float dt) {
    if (gameState != Playing && gameState != Dead) return;

    auto& r = Engine::instance().renderer();
    r.beginFrame({0.3f, 0.5f, 0.8f, 1.0f});

    r.setCamera(pl->cameraPos(), pl->cameraTarget(), {0, 1, 0});
    w->render(pl->position());
    pl->render();

    r.endFrame();
}

void Game::startLocalGame() {
    Console::instance().printf(LogLevel::Info, "Starting local game");
    setState(Loading);

    // Load a default map
    if (w->load("test")) {
        pl->respawn();
        setState(Playing);
        Console::instance().printf(LogLevel::Info, "Game started");
    } else {
        Console::instance().printf(LogLevel::Error, "Failed to load map");
        setState(Menu);
    }
}

void Game::connectToServer(const char* host, uint16_t port) {
    cfg.serverHost = host;
    cfg.serverPort = port;
    cfg.online = true;

    Console::instance().printf(LogLevel::Info, "Connecting to %s:%d...", host, port);

    auto& net = Engine::instance().network();
    Connection* conn = net.createConnection();
    if (conn->connect(host, port)) {
        conn->setConnectCallback([this](bool success) {
            if (success) {
                setState(Playing);
                Console::instance().printf(LogLevel::Info, "Connected!");
            } else {
                setState(Menu);
                Console::instance().printf(LogLevel::Info, "Connection failed");
            }
        });
    }
}

void Game::applyInput(const InputMove& input) {
    currentInput = input;
}

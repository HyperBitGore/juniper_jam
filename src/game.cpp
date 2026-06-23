#include "game.hpp"
#include "entity.hpp"
#include "g_engine/file_loading/font_loader.hpp"
#include "g_engine/rendering/font_renderer.hpp"
#include "path.hpp"
#include <cstdint>

Game::Game(std::unique_ptr<gore::imagerenderer>& image_r, std::unique_ptr<gore::trianglerenderer>& triangle_r, std::unique_ptr<gore::fontrenderer>& font_r) {
    this->image_r = image_r.get();
    this->triangle_r = triangle_r.get();
    this->font_r = font_r.get();
    font_map.setHashFunction(font_hash);
    spatial_hashmap = SpatialHashmap(50, 5000);
    load();
    setGameMode(GAME_MODE::MAIN_MENU);
}
void Game::loop() {
    bool above_click = false;
    switch (mode) {
        case GAME_MODE::MAIN_MENU:
            above_click = main_menu_loop();    
        break;
        case GAME_MODE::PAUSE_MENU:
            above_click = pause_menu_loop();
            break;
        case GAME_MODE::GAME_LOOP:
            above_click = game_loop();
        break;
    }
    updateButtons(above_click);
}

inline uint32_t floatToBytes (float n) {
    uint32_t m;
    std::memcpy(&m, &n, sizeof(uint32_t));
    return m;
}

void Game::save() {
    std::ofstream file("save.save", std::ios::out | std::ios::binary);
    // write money and food
    size_t siz = entities.size();
    file.write(reinterpret_cast<const char*>(&this->money), sizeof(this->money));
    file.write(reinterpret_cast<const char*>(&this->food), sizeof(this->food));
    file.write(reinterpret_cast<const char*>(&siz), sizeof(unsigned long));
    // serialize entities
    for (auto& i : entities) {
        uint8_t v = i.typeToUint8t();
        file.write(reinterpret_cast<const char*>(&v), sizeof(uint8_t));
        uint32_t data = floatToBytes(i.pos.x);
        file.write(reinterpret_cast<const char*>(&data), sizeof(float));
        data = floatToBytes(i.pos.y);
        file.write(reinterpret_cast<const char*>(&data), sizeof(float));
        data = floatToBytes(i.dimen.x);
        file.write(reinterpret_cast<const char*>(&data), sizeof(float));
        data = floatToBytes(i.dimen.y);
        file.write(reinterpret_cast<const char*>(&data), sizeof(float));
    }
    file.close();
}
void Game::load() {
    // pathfinder::calculatePathBenchmark(&spatial_hashmap);
    entities.clear();
    std::ifstream file("save.save", std::ios::binary);
    if (file) {
        file.read(reinterpret_cast<char*>(&this->money), sizeof(this->money));
        file.read(reinterpret_cast<char*>(&this->food), sizeof(this->food));
        size_t siz = 0;
        file.read(reinterpret_cast<char*>(&siz), sizeof(size_t));
        for (size_t i = 0; i < siz; i++) {
            entity_type type;
            uint8_t v;
            file.read(reinterpret_cast<char*>(&v), sizeof(uint8_t));
            type = entity::uin8tToType(v);
            // read the position and dimensions
            float px, py, dx, dy;
            file.read(reinterpret_cast<char*>(&px), sizeof(float));
            file.read(reinterpret_cast<char*>(&py), sizeof(float));
            file.read(reinterpret_cast<char*>(&dx), sizeof(float));
            file.read(reinterpret_cast<char*>(&dy), sizeof(float));
            entities.push_back({{px, py}, {dx, dy}, -1, type});
        }
    }
    file.close();
    
    /*const float cs = 50.0f; // one cell = one wall
    // Horizontal wall across the middle with a gap on the right (y=400, x=50..700)
    for (int i = 0; i < 14; i++) {
        entities.push_back({{ cs + i * cs, 8 * cs }, { cs, cs }, -1, entity_type::STRUCTURE});
    }
    // Vertical wall on the left forming an L with the horizontal wall (x=50, y=150..350)
    for (int i = 0; i < 5; i++) {
        entities.push_back({{ cs, 3 * cs + i * cs }, { cs, cs }, -1, entity_type::STRUCTURE});
    }
    // Small 2x2 cluster in the top-right to force routing decisions
    entities.push_back({{ 14 * cs, 3 * cs }, { cs, cs }, -1, entity_type::STRUCTURE});
    entities.push_back({{ 15 * cs, 3 * cs }, { cs, cs }, -1, entity_type::STRUCTURE});
    entities.push_back({{ 14 * cs, 4 * cs }, { cs, cs }, -1, entity_type::STRUCTURE});
    entities.push_back({{ 15 * cs, 4 * cs }, { cs, cs }, -1, entity_type::STRUCTURE});
    this->money = 100;
    this->food = 24;*/
}
void Game::setGameMode(GAME_MODE mode) {
     switch (mode) {
        case GAME_MODE::MAIN_MENU:
            constructMainMenuButtons();
        break;
        case GAME_MODE::PAUSE_MENU:
            constructPauseMenuButtons();
            break;
        case GAME_MODE::GAME_LOOP:
            constructGameButtons();
        break;
    }
    this->mode = mode;
}

void Game::addFont (std::filesystem::path location, uint32_t dpi) {
    gore::font font = gore::fontloader::loadFont(location, 0, 1024);
    gore::fontraster::rasterizeFont(&font, 96, dpi,  0xFFFFFFFF, 32, 127);
    font_map.insert(location.filename(), font);
}

void Game::renderButton (button b, gore::font* font) {
    triangle_r->setColor({0.0f, 0.5f, 1.0f, 1.0f});
    triangle_r->drawQuad(b.pos, b.dimen.x, b.dimen.y);
    font_r->drawText(b.text, font, b.pos.x, b.pos.y + BUTTON_TEXT_PT, BUTTON_TEXT_PT, eng->getDPI());
}

void Game::updateButtons (bool above_click) {
    gore::font* font = font_map.get("OpenSans-Regular.ttf");
    font_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    for (auto& i : buttons) {
        if (i.display) renderButton(i, font);
    }
    mouse_click_cooldown += delta;
    if (mouse_click_cooldown > 0.3f) {
        if (eng->getMouseLeftDown() || above_click) {
            mouse_click_cooldown = 0.0f;
            gore::vec2 pos = eng->getMousePos();
            entity mouse = { pos, {10.0f, 10.0f }};
            for (auto& i : buttons) {
                if (i.display && i.isColliding(mouse)) {
                    i.function(&i);
                    break;
                }
            }
        }
    }
}
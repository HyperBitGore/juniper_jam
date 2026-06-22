#include "game.hpp"
#include "entity.hpp"
#include "g_engine/file_loading/font_loader.hpp"
#include "g_engine/rendering/font_renderer.hpp"

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
void Game::save() {
    // TODO
}
void Game::load() {
    // Obstacle layout for pathfinding tests (world: 1024x768)
    // Horizontal wall across the middle with a gap on the right
    for (int i = 0; i < 14; i++) {
        entities.push_back({{ 50.0f + i * 40.0f, 384.0f }, { 40.0f, 40.0f }, -1, entity_type::STRUCTURE});
    }
    // Vertical wall on the left forming an L with the horizontal wall
    for (int i = 0; i < 6; i++) {
        entities.push_back({{ 50.0f, 144.0f + i * 40.0f }, { 40.0f, 40.0f }, -1, entity_type::STRUCTURE});
    }
    // Small cluster in the top-right to force routing decisions
    entities.push_back({{ 700.0f, 150.0f }, { 40.0f, 40.0f }, -1, entity_type::STRUCTURE});
    entities.push_back({{ 740.0f, 150.0f }, { 40.0f, 40.0f }, -1, entity_type::STRUCTURE});
    entities.push_back({{ 700.0f, 190.0f }, { 40.0f, 40.0f }, -1, entity_type::STRUCTURE});
    entities.push_back({{ 740.0f, 190.0f }, { 40.0f, 40.0f }, -1, entity_type::STRUCTURE});
    this->money = 100;
    this->food = 24;
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
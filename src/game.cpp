#include "game.hpp"
#include "entity.hpp"
#include "g_engine/file_loading/font_loader.hpp"
#include "g_engine/rendering/font_renderer.hpp"

Game::Game(std::unique_ptr<gore::imagerenderer>& image_r, std::unique_ptr<gore::trianglerenderer>& triangle_r, std::unique_ptr<gore::fontrenderer>& font_r) {
    mode = GAME_MODE::MAIN_MENU;
    this->image_r = image_r.get();
    this->triangle_r = triangle_r.get();
    this->font_r = font_r.get();
    font_map.setHashFunction(font_hash);
    spatial_hashmap = SpatialHashmap(50, 5000);
    load();
}
void Game::loop() {
    switch (mode) {
        case GAME_MODE::MAIN_MENU:
            main_menu_loop();    
        break;
        case GAME_MODE::PAUSE_MENU:
            pause_menu_loop();
            break;
        case GAME_MODE::GAME_LOOP:
            game_loop();
        break;
    }
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
}
void Game::setGameMode(GAME_MODE mode) {
    this->mode = mode;
}

void Game::addFont (std::filesystem::path location, uint32_t dpi) {
    gore::font font = gore::fontloader::loadFont(location, 0, 1024);
    gore::fontraster::rasterizeFont(&font, 96, dpi,  0xFFFFFFFF, 32, 127);
    font_map.insert(location.filename(), font);
}
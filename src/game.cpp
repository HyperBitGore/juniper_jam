#include "game.hpp"
#include "g_engine/file_loading/font_loader.hpp"
#include "g_engine/rendering/font_renderer.hpp"

Game::Game(std::unique_ptr<gore::imagerenderer>& image_r, std::unique_ptr<gore::trianglerenderer>& triangle_r, std::unique_ptr<gore::fontrenderer>& font_r) {
    mode = GAME_MODE::MAIN_MENU;
    this->image_r = image_r.get();
    this->triangle_r = triangle_r.get();
    this->font_r = font_r.get();
    font_map.setHashFunction(font_hash);
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
    // TODO
}
void Game::setGameMode(GAME_MODE mode) {
    this->mode = mode;
}

void Game::addFont (std::filesystem::path location, uint32_t dpi) {
    gore::font font = gore::fontloader::loadFont(location, 0, 1024);
    gore::fontraster::rasterizeFont(&font, 96, dpi,  0xFFFFFFFF, 32, 127);
    font_map.insert(location.filename(), font);
}
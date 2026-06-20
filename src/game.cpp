#include "game.hpp"

Game::Game () {
    mode = GAME_MODE::MAIN_MENU;
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
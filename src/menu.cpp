#include "game.hpp"

entity play_button = { {300.0f, 200.0f}, {500.0f, 200.0f}};

bool Game::main_menu_loop() {
    /*gore::font* font = font_map.get("OpenSans-Regular.ttf");
    if (font) {
        font_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
        triangle_r->setColor({0.0f, 0.5f, 0.5f, 1.0f});
        triangle_r->drawQuad({250.0f, 200.0f}, 500.0f, 100.0f);
        font_r->drawText("Play", font, 300.0f, 300.0f, 96, 96);
        if (eng->getMouseLeftDown()) {
            gore::vec2 pos = eng->getMousePos();
            std::cout << pos.x << ", " << pos.y << "\n";
            entity e = { pos, { 10, 10 }};
            if (play_button.isColliding(e)) {
                mode = GAME_MODE::GAME_LOOP;
            }
        }
    }*/
    return false;
}

void Game::constructMainMenuButtons () {
    buttons.clear();
    auto play = [&](button* b) {
        setGameMode(GAME_MODE::GAME_LOOP);
    };
    button play_button({250.0f, 200.0f}, play, "Play");
    play_button.display = true;
    buttons.push_back(play_button);
}
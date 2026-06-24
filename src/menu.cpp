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
        load(current_save);
    };
    auto play_level = [&](button* b) {
        setGameMode(GAME_MODE::LEVEL_EDITOR);
    };
    auto new_game_func = [&](button* b) {
        setGameMode(GAME_MODE::GAME_LOOP);
        this->new_game();
    };
    const float gap = BUTTON_TEXT_PT + 10;
    auto centered_x = [](std::string text) {
        return (float)WINDOW_WIDTH / 2 - (text.size() * BUTTON_TEXT_PT) / 2.0f;
    };
    button play_button({centered_x("New Game"), (float)WINDOW_HEIGHT / 2}, new_game_func, "New Game");
    play_button.display = true;
    buttons.push_back(play_button);
    button load_button({centered_x("New Game"), (float)WINDOW_HEIGHT / 2 + gap}, play, "Load Game");
    buttons.push_back(load_button);
    button level_button({centered_x("New Game"), (float)WINDOW_HEIGHT / 2 + gap * 2}, play_level, "Level Editor");
    buttons.push_back(level_button);
}
#pragma once
#include <iostream>
#include <g_engine/g_engine_2d.hpp>

class entity {
    public:
        gore::vec2 pos;
        gore::vec2 dimen;
        int32_t img_id;
        entity (gore::vec2 pos, gore::vec2 dimen, int32_t imd_id = -1) {
            this->pos = pos;
            this->dimen = dimen;
            this->img_id = imd_id;
        }
};
// Game
//   -> update functions
//          Main menu
//          Pause menu
//          Game loop

// TODO
//  - basic entity movement
//  - pathing
//  - construction
//  - saving
//  - combat

enum class GAME_MODE { MAIN_MENU, PAUSE_MENU, GAME_LOOP };

class Game {
    private:
        // vars
        GAME_MODE mode;
        std::vector<entity> entities;
        void main_menu_loop();
        void pause_menu_loop();
        void game_loop();
    public:
        Game ();
        void loop();
        void save();
        void load();
        void setGameMode(GAME_MODE mode);
};
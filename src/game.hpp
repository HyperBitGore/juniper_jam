#pragma once
#include "g_engine/rendering/font_renderer.hpp"
#include "g_engine/rendering/image_renderer.hpp"
#include "g_engine/rendering/primitive_renderer.hpp"
#include "g_engine/util/shader.hpp"
#include <filesystem>
#include <iostream>
#include <g_engine/g_engine_2d.hpp>
#include <memory>

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
        bool isColliding(const entity& e) const {
            return (pos.x + dimen.x > e.pos.x &&         // this.right > other.left
                    pos.x < e.pos.x + e.dimen.x &&       // this.left < other.right
                    pos.y + dimen.y > e.pos.y &&         // this.bottom > other.top
                    pos.y < e.pos.y + e.dimen.y);        // this.top < other.bottom
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
        gore::imagerenderer* image_r;
        gore::trianglerenderer* triangle_r;
        gore::fontrenderer* font_r;
        std::vector<entity> entities;
        // fonts
        static uint32_t font_hash (std::string str) {
            if (str.size() == 0) {
                return 0;
            } 
            return str[0] + str.size() % 1024;
        }
        gore::hashmap<gore::font, std::string> font_map;
        void main_menu_loop();
        void pause_menu_loop();
        void game_loop();
    public:
        double delta = 0.0;
        gore::g_engine_2d* eng;
        Game (std::unique_ptr<gore::imagerenderer>& image_r, std::unique_ptr<gore::trianglerenderer>& triangle_r, std::unique_ptr<gore::fontrenderer>& font_r);
        void loop();
        void save();
        void load();
        void setGameMode(GAME_MODE mode);
        void addFont (std::filesystem::path path, uint32_t dpi);
};
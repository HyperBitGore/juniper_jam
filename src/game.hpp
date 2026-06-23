#pragma once
#include "entity.hpp"
#include "g_engine/rendering/font_renderer.hpp"
#include "g_engine/rendering/image_renderer.hpp"
#include "g_engine/rendering/primitive_renderer.hpp"
#include "g_engine/util/shader.hpp"
#include <filesystem>
#include <g_engine/g_engine_2d.hpp>
#include <memory>
#include "spatial_hashmap.hpp"
#define BUTTON_TEXT_PT 16
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
// Game
//   -> update functions
//          Main menu
//          Pause menu
//          Game loop

// TODO
//  - camera movement
//  - level editor mode
//  - combat
//  - game loop
//      - engine construction
//      - farming
//      - mining
//      - enemy waves

enum class GAME_MODE { MAIN_MENU, PAUSE_MENU, GAME_LOOP };

struct button : entity {
    std::function<void(button*)> function; 
    std::string text;
    bool display;
    button (gore::vec2 pos, std::function<void(button*)> function, std::string text) : entity(pos, { BUTTON_TEXT_PT, BUTTON_TEXT_PT }, -1, entity_type::BUTTON) {
        this->function = function;
        this->text = text;
        // modify pos and dimen to match text pts
        this->dimen.x = text.size() * BUTTON_TEXT_PT;
        this->display = true;
    }
};

class Game {
    private:
        // vars
        GAME_MODE mode;
        gore::imagerenderer* image_r;
        gore::trianglerenderer* triangle_r;
        gore::fontrenderer* font_r;
        // game loop vars
        std::vector<entity> entities;
        uint32_t money;
        uint32_t food;
        SpatialHashmap spatial_hashmap;
        // fonts
        static uint32_t font_hash (std::string str) {
            if (str.size() == 0) {
                return 0;
            } 
            return str[0] + str.size() % 1024;
        }
        gore::hashmap<gore::font, std::string> font_map;
        std::vector<button> buttons; // construct these in a function called when loop changed
        bool main_menu_loop();
        bool pause_menu_loop();
        bool game_loop();
        void constructGameButtons ();
        void constructPauseMenuButtons ();
        void constructMainMenuButtons ();
        // update buttons
        double mouse_click_cooldown = 0;
        void updateButtons (bool above_click);
        void renderButton (button b, gore::font* font);
    public:
        bool play = true;
        double delta = 0.0;
        gore::g_engine_2d* eng;
        Game (std::unique_ptr<gore::imagerenderer>& image_r, std::unique_ptr<gore::trianglerenderer>& triangle_r, std::unique_ptr<gore::fontrenderer>& font_r);
        void loop();
        void save();
        void load();
        void setGameMode(GAME_MODE mode);
        void addFont (std::filesystem::path path, uint32_t dpi);
};
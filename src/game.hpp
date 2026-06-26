#pragma once
#include "entity.hpp"
#include "g_engine/rendering/font_renderer.hpp"
#include "g_engine/rendering/image_renderer.hpp"
#include "g_engine/rendering/primitive_renderer.hpp"
#include "g_engine/util/shader.hpp"
#include <deque>
#include <filesystem>
#include <g_engine/g_engine_2d.hpp>
#include <memory>
#include <unordered_map>
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
//  - game loop
//      - enemy waves

enum class GAME_MODE { MAIN_MENU, PAUSE_MENU, GAME_LOOP, LEVEL_EDITOR };

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
        gore::trianglerenderer* static_triangle_r;
        gore::fontrenderer* font_r;
        gore::fontrenderer* static_font_r;
        gore::linerenderer* line_r;
        // game loop vars
        double cam_move = 0.0;
        gore::vec2 cam_pos = {0, 0};
        float cam_zoom = 1.0;
        std::deque<entity> entities;
        std::deque<entity> enemies;
        int rpm = 200;
        bool motor_on = true;
        float blade_scroll = 0.0f;
        std::deque<entity> blades;
        std::vector<button> popups;
        entity* selected = nullptr;
        int64_t money;
        int64_t food;
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
        bool level_edit = false;
        int motor_index = 4;
        double enemy_spawn_timer = 0.0;
        double enemy_spawn_max = 10.0;
        int enemy_spawning_level = 2;
        int spawn_count = 0;
        bool main_menu_loop();
        bool pause_menu_loop();
        bool game_loop();
        void renderSelectFrame ();
        bool level_editor_loop();
        void constructGameButtons ();
        void constructPauseMenuButtons ();
        void constructMainMenuButtons ();
        std::string current_save = "save.save";
        double camera_update = 0.0;
        void cameraUpdate ();
        gore::vec2 dropdown_world_pos = {0, 0};
        // update buttons
        double mouse_click_cooldown = 0;
        void updateButtons (bool above_click);
        void renderButton (button b, gore::font* font);
        gore::vec2 randomLocation ();
        void addPopup (gore::vec2 pos, std::string text);
        void renderPopups ();
        // images
        std::unordered_map<std::string, gore::IMG> images;
        void addImage (std::string image);
        int edge_count = 0;
        void renderBackground ();
        void enemySpawning ();
    public:
        bool play = true;
        double delta = 0.0;
        gore::g_engine_2d* eng;
        Game (std::unique_ptr<gore::imagerenderer>& image_r, std::unique_ptr<gore::trianglerenderer>& triangle_r, std::unique_ptr<gore::trianglerenderer>& static_triangle_r, std::unique_ptr<gore::linerenderer>& line_r, std::unique_ptr<gore::fontrenderer>& font_r, std::unique_ptr<gore::fontrenderer>& static_font_r);
        void loop();
        void new_game ();
        void save(std::string path);
        void load(std::string path);
        void levelEditorLoad ();
        void setGameMode(GAME_MODE mode);
        void addFont (std::filesystem::path path, uint32_t dpi);
        entity constructEntity (gore::vec2 pos, gore::vec2 dimen, int imd_id = -1, entity_type type = entity_type::UNIT);
};
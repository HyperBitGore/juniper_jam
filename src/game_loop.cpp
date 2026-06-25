#include "entity.hpp"
#include "g_engine/rendering/font_renderer.hpp"
#include "g_engine/util/shader.hpp"
#include "game.hpp"
#include "path.hpp"
#include <string>

bool render_right_click_dropdown = false;
//  - freezing was caused by std::vector reallocation invalidating raw entity* pointers stored in the spatial hashmap
//    fixed by: using std::deque (stable pointers on push_back), inserting initial entities, and tracking selected through erases
// TODO motor spin

// TODO upgrading
// TODO text popups
// TODO art
// TODO animation
// TODO sound/music
// TODO balance
bool Game::game_loop() {
    if (eng->getKeyReleased(g_Escape)) {
        this->setGameMode(GAME_MODE::PAUSE_MENU);
        return false;
    }
    enemy_spawn_timer += delta;
    if (enemy_spawn_timer >= enemy_spawn_max) {
        enemy_spawn_timer = 0.0;
        entity enem = constructEntity(randomLocation(), { 20, 20 }, -1, entity_type::MASS);
        enemies.push_back(enem);
        spatial_hashmap.insert(&enemies[enemies.size() - 1]);
        enemy_spawn_max -= 0.002;
        if (enemy_spawn_max < 1.0) {
            enemy_spawn_max = 1.0;
        }
    }
    bool above_click = false;
    gore::font* font = font_map.get("OpenSans-Regular.ttf");
    mouse_click_cooldown += delta;
    gore::vec2 m_pos = eng->getMousePos();
    if (mouse_click_cooldown > 0.3f) {
        if (eng->getMouseLeftDown()) {
            above_click = true;
            gore::vec2 pos = eng->getMousePos();
            entity mouse = { pos, {10.0f, 10.0f }};
            if (!render_right_click_dropdown) {
                mouse_click_cooldown = 0.0f;
                std::vector<entity*> collisions = spatial_hashmap.getCollisions(&mouse);
                if (collisions.size() > 0) {
                    if (selected != nullptr && selected->type == entity_type::UNIT && (collisions[0]->type == entity_type::FARM || collisions[0]->type == entity_type::MASS)) {
                        // find the index
                        int index = -1;
                        if (collisions[0]->type == entity_type::FARM) {
                            for (size_t i = 0; i < entities.size(); i++) {
                                if (collisions[0] == &entities[i]) {
                                    index = i;
                                    break;
                                }
                            }
                        } else {
                            for (size_t i = 0; i < enemies.size(); i++) {
                                if (collisions[0] == &enemies[i]) {
                                    index = i;
                                    break;
                                }
                            }
                        }
                        if (index != -1) {
                            if (collisions[0]->type == entity_type::MASS) {
                                selected->action = { index, action_type::ATTACK};
                            } else {
                                selected->action = { index, action_type::COLLECT};
                            }
                        }
                    } else {
                        selected = collisions[0];
                    }
                } else if (selected != nullptr && selected->type == entity_type::UNIT) {
                    selected->action = {-1, action_type::NONE};
                    spatial_hashmap.remove(selected);
                    selected->path = pathfinder::calculatePath(&spatial_hashmap, *selected, pos);
                    spatial_hashmap.insert(selected);
                }
            }
        }
        if (eng->getMouseRightDown()) {
            mouse_click_cooldown = 0.0f;
            above_click = true;
            if (!render_right_click_dropdown) {
                render_right_click_dropdown = true;
                gore::vec2 pos = eng->getMousePos(false, true);
                dropdown_world_pos = m_pos;
                for (size_t i = 0; i < buttons.size(); i++) {
                    buttons[i].pos = { pos.x, pos.y + (float)i * (BUTTON_TEXT_PT + 4) };
                    buttons[i].display = true;
                }
            } else {
                render_right_click_dropdown = false;
                for (auto& b : buttons) b.display = false;
            }
        }
    }
    triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    auto drawSelectedOutline = [&](entity& e) {
        line_r->setColor({0.0f, 1.0f, 0.0f, 1.0f});
        const gore::vec2 pad = {5.0f, 5.0f};
        gore::vec2 tl = e.pos - pad;
        gore::vec2 tr = {e.pos.x + e.dimen.x + pad.x, e.pos.y - pad.y};
        gore::vec2 br = e.pos + e.dimen + pad;
        gore::vec2 bl = {e.pos.x - pad.x, e.pos.y + e.dimen.y + pad.y};
        line_r->addLine(tl, tr); // top
        line_r->addLine(tr, br); // right
        line_r->addLine(br, bl); // bottom
        line_r->addLine(bl, tl); // left
        line_r->drawBuffer();
    };
    for (size_t i = 0; i < entities.size(); i++) {
        if (entities[i].update) {
            entities[i].update(&entities[i]);
        }
        if (spatial_hashmap.checkCollision(&entities[i])) {
            std::cout << "colliding!" << entities[i].pos.x << ", " << entities[i].pos.y << "\n";
        }
        if (&entities[i] == selected) {
            drawSelectedOutline(entities[i]);
            if (entities[i].render) entities[i].render(&entities[i]);
        } else if (entities[i].render) {
            entities[i].render(&entities[i]);
        } else {
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
            triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
        }
    }
    for (size_t i = 0; i < enemies.size(); i++) {
         if (enemies[i].update) {
            enemies[i].update(&enemies[i]);
        }
        if (&enemies[i] == selected) {
            drawSelectedOutline(enemies[i]);
            if (enemies[i].render) enemies[i].render(&enemies[i]);
        } else if (enemies[i].render) {
            enemies[i].render(&enemies[i]);
        } else {
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
            triangle_r->drawQuad(enemies[i].pos, enemies[i].dimen.x, enemies[i].dimen.y);
        }
    }
    // cull dead enemies — track selected index so pointer stays valid after erase
    {
        int sel_idx = -1;
        for (int k = 0; k < (int)enemies.size(); k++) {
            if (&enemies[k] == selected) { sel_idx = k; break; }
        }
        bool selected_was_in_enemies = (sel_idx >= 0);
        for (int i = (int)enemies.size() - 1; i >= 0; i--) {
            if (enemies[i].hp <= 0) {
                if (sel_idx == i) {
                    sel_idx = -2; // mark as erased
                } else if (sel_idx > i) {
                    sel_idx--;
                }
                for (int j = 0; j < (int)enemies.size(); j++) {
                    spatial_hashmap.remove(&enemies[j]);
                }
                enemies.erase(enemies.begin() + i);
                for (int j = 0; j < (int)enemies.size(); j++) {
                    spatial_hashmap.insert(&enemies[j]);
                }
            }
        }
        if (selected_was_in_enemies) {
            selected = (sel_idx >= 0 && sel_idx < (int)enemies.size()) ? &enemies[sel_idx] : nullptr;
        }
    }
    // cull dead entities (units, farms, structures — not map edges or motor)
    {
        int sel_idx = -1;
        for (int k = 0; k < (int)entities.size(); k++) {
            if (&entities[k] == selected) { sel_idx = k; break; }
        }
        for (int i = (int)entities.size() - 1; i >= 0; i--) {
            if (entities[i].hp <= 0 &&
                entities[i].type != entity_type::MAP_EDGE &&
                entities[i].type != entity_type::MOTOR) {
                if (sel_idx == i) {
                    sel_idx = -2; // mark as erased
                } else if (sel_idx > i) {
                    sel_idx--;
                }
                for (int j = 0; j < (int)entities.size(); j++) {
                    spatial_hashmap.remove(&entities[j]);
                }
                entities.erase(entities.begin() + i);
                for (int j = 0; j < (int)entities.size(); j++) {
                    spatial_hashmap.insert(&entities[j]);
                }
                // motor_index stays fixed since motor is always at index 4
            }
        }
        if (sel_idx >= 0 && sel_idx < (int)entities.size()) {
            selected = &entities[sel_idx];
        } else if (sel_idx == -2) {
            selected = nullptr; // selected entity was erased
        }
        // if selected wasn't in entities (e.g. was an enemy), leave it unchanged
    }
    font_r->setColor({1.0f, 0.0f, 0.5f, 1.0f});
    font_r->drawText("Money: " + std::to_string(this->money), font, 0, 32, 24, eng->getDPI());
    font_r->drawText("Food: " + std::to_string(this->food), font, 0, 64, 24, eng->getDPI());
    font_r->drawText("Mouse: " + std::to_string(m_pos.x) + ", " + std::to_string(m_pos.y), font, 0, 128, 24, eng->getDPI());
    // render entity selection bottom left
    renderSelectFrame();
    return above_click;
}

void Game::renderSelectFrame () {
    gore::font* font = font_map.get("OpenSans-Regular.ttf");
    static_triangle_r->setColor({0.0f, 0.5f, 1.0f, 1.0f});
    static_triangle_r->drawQuad({0, 630}, 400, 138);
    font_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    if (selected == nullptr) {
        font_r->drawText("Selection: none", font, 4, 654, 16, eng->getDPI());
        return;
    }
    auto type_name = [](entity_type t) -> std::string {
        switch (t) {
            case entity_type::UNIT:      return "Unit";
            case entity_type::STRUCTURE: return "Structure";
            case entity_type::MOTOR:     return "Motor";
            case entity_type::FARM:      return "Farm";
            case entity_type::MASS:      return "Mass";
            case entity_type::MAP_EDGE:  return "Map Edge";
            default:                     return "Unknown";
        }
    };
    font_r->drawText("Selection: " + type_name(selected->type), font, 4, 654, 16, eng->getDPI());
    font_r->drawText("Level: " + std::to_string(selected->level), font, 4, 676, 16, eng->getDPI());
    font_r->drawText("Pos: " + std::to_string((int)selected->pos.x) + ", " + std::to_string((int)selected->pos.y), font, 4, 698, 16, eng->getDPI());

}

bool Game::pause_menu_loop () {
    if (eng->getKeyReleased(g_Escape)) {
        this->setGameMode(GAME_MODE::GAME_LOOP);
        return false;
    }
    gore::font* font = font_map.get("OpenSans-Regular.ttf");
    triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    for (size_t i = 0; i < entities.size(); i++) {
        if (entities[i].path.size() > 0) {
            // draw the path
            triangle_r->setColor({1.0f, 0.5f, 0.0f, 1.0f});
            for (auto& j : entities[i].path) {
                triangle_r->addQuad(j, 15, 15);
            }
            triangle_r->drawBuffer();
        }
        if (&entities[i] == selected) {
            triangle_r->setColor({0.0f, 1.0f, 0.0f, 1.0f});
            triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
        } else if (entities[i].render) {
            entities[i].render(&entities[i]);
        } else {
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
            triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
        }
    }
    font_r->setColor({1.0f, 0.0f, 0.5f, 1.0f});
    font_r->drawText("Money: " + std::to_string(this->money), font, 0, 32, 24, eng->getDPI());
    font_r->drawText("Food: " + std::to_string(this->food), font, 0, 64, 24, eng->getDPI());
    renderSelectFrame();
    return false;
}

bool Game::level_editor_loop() {
    if (eng->getKeyReleased(g_Escape)) {
        this->setGameMode(GAME_MODE::PAUSE_MENU);
        return false;
    }
    bool above_click = false;
    gore::font* font = font_map.get("OpenSans-Regular.ttf");
    mouse_click_cooldown += delta;
    gore::vec2 m_pos = eng->getMousePos();
    if (mouse_click_cooldown > 0.3f) {
        if (eng->getMouseLeftDown()) {
            above_click = true;
            if (!render_right_click_dropdown) {
                mouse_click_cooldown = 0.0f;
            }
        }
        if (eng->getMouseRightDown()) {
            mouse_click_cooldown = 0.0f;
            above_click = true;
            if (!render_right_click_dropdown) {
                render_right_click_dropdown = true;
                gore::vec2 pos = eng->getMousePos(false, true);
                dropdown_world_pos = m_pos;
                for (size_t i = 0; i < buttons.size(); i++) {
                    buttons[i].pos = { pos.x, pos.y + (float)i * (BUTTON_TEXT_PT + 4) };
                    buttons[i].display = true;
                }
            } else {
                render_right_click_dropdown = false;
                for (auto& b : buttons) b.display = false;
            }
        }
    }
    triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    for (size_t i = 0; i < entities.size(); i++) {
        if (&entities[i] == selected) {
            triangle_r->setColor({0.0f, 1.0f, 0.0f, 1.0f});
            triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
        } else if (entities[i].render) {
            entities[i].render(&entities[i]);
        } else {
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
            triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
        }
    }
    font_r->setColor({1.0f, 0.0f, 0.5f, 1.0f});
    font_r->drawText("Level Editor", font, 0, 32, 24, eng->getDPI());
    font_r->drawText("Mouse: " + std::to_string(m_pos.x) + ", " + std::to_string(m_pos.y), font, 0, 64, 24, eng->getDPI());
    return above_click;
}

void Game::constructGameButtons () {
    buttons.clear();
    auto add_worker = [&](button* b) {
        if (this->money < 25) return;
        this->money -= 25;
        entity e = constructEntity(dropdown_world_pos, {20.0f, 20.0f}, -1, entity_type::UNIT);
        entities.push_back(e);
        spatial_hashmap.insert(&entities.back());
        for (auto& btn : buttons) btn.display = false;
        render_right_click_dropdown = false;
    };
    button worker_button({0, 0}, add_worker, "Buy Worker");
    worker_button.display = false;
    buttons.push_back(worker_button);

    auto add_wall = [&](button* b) {
        if (this->money < 50) return;
        this->money -= 50;
        entity e = constructEntity( dropdown_world_pos, {50.0f, 50.0f}, -1, entity_type::STRUCTURE );
        entities.push_back(e);
        spatial_hashmap.insert(&entities.back());
        for (auto& btn : buttons) btn.display = false;
        render_right_click_dropdown = false;
    };
    button wall_button({0, 0}, add_wall, "Buy Wall");
    wall_button.display = false;
    buttons.push_back(wall_button);
    auto add_farm = [&](button* b) {
        if (this->money < 35) return;
        this->money -= 35;
        entity e = constructEntity( dropdown_world_pos, {34.0f, 34.0f}, -1, entity_type::FARM );
        entities.push_back(e);
        spatial_hashmap.insert(&entities.back());
        for (auto& btn : buttons) btn.display = false;
        render_right_click_dropdown = false;
    };
    button farm_button({0, 0}, add_farm, "Buy Farm");
    farm_button.display = false;
    buttons.push_back(farm_button);
}
void Game::constructPauseMenuButtons () {
    buttons.clear();
    auto save = [&](button* b) {
        this->save(current_save);
    };
    auto load = [&](button* b) {
        this->load(current_save);
    };
    auto quit = [&](button* b) {
        this->play = false;
    };
    const float gap = BUTTON_TEXT_PT + 10;
    auto centered_x = [](std::string text) {
        return (float)WINDOW_WIDTH / 2 - (text.size() * BUTTON_TEXT_PT) / 2.0f;
    };
    button save_btn({centered_x("Save"), (float)WINDOW_HEIGHT / 2}, save, "Save");
    buttons.push_back(save_btn);
    button load_btn({centered_x("Load"), (float)WINDOW_HEIGHT / 2 + gap}, load, "Load");
    buttons.push_back(load_btn);
    button quit_btn({centered_x("Quit"), (float)WINDOW_HEIGHT / 2 + gap * 2}, quit, "Quit");
    buttons.push_back(quit_btn);
}
#define CAMERA_SPEED 10.0f

void Game::cameraUpdate () {
    camera_update += delta;
    cam_move += delta;
    bool update_camera = false;
    if (eng->getKeyDown(g_a)) {
        cam_pos.x -= CAMERA_SPEED;
        update_camera = true;
        cam_move = 0;
    } else if (eng->getKeyDown(g_d)) {
        cam_pos.x += CAMERA_SPEED;
        update_camera = true;
        cam_move = 0;
    } else if (eng->getKeyDown(g_w)) {
        cam_pos.y -= CAMERA_SPEED;
        update_camera = true;
        cam_move = 0;
    } else if (eng->getKeyDown(g_s)) {
        cam_pos.y += CAMERA_SPEED;
        update_camera = true;
        cam_move = 0;
    }
    if (camera_update > 0.008) {
        if (eng->getKeyDown(g_z)) {
            cam_zoom += 0.01f;
            update_camera = true;
            camera_update = 0.0;
        } else if (eng->getKeyDown(g_x)) {
            cam_zoom -= 0.01f;
            update_camera = true;
            camera_update = 0.0;
        } else if (eng->getKeyDown(g_r)) {
            cam_zoom = 1.0f;
            update_camera = true;
            camera_update = 0.0;
        }
    }
    if (update_camera) {
        eng->updateView(cam_pos.x, cam_pos.y, cam_zoom);
    }
}
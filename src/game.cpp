#include "game.hpp"
#include "entity.hpp"
#include "g_engine/file_loading/font_loader.hpp"
#include "g_engine/rendering/font_renderer.hpp"
#include "g_engine/rendering/primitive_renderer.hpp"
#include "path.hpp"
#include <cmath>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <random>
#include <string>

Game::Game(std::unique_ptr<gore::imagerenderer>& image_r, std::unique_ptr<gore::trianglerenderer>& triangle_r, std::unique_ptr<gore::trianglerenderer>& static_triangle_r, std::unique_ptr<gore::linerenderer>& line_r, std::unique_ptr<gore::fontrenderer>& font_r
    , std::unique_ptr<gore::fontrenderer>& static_font_r
) {
    this->image_r = image_r.get();
    this->triangle_r = triangle_r.get();
    this->static_triangle_r = static_triangle_r.get();
    this->font_r = font_r.get();
    this->line_r = line_r.get();
    this->static_font_r = static_font_r.get();
    font_map.setHashFunction(font_hash);
    spatial_hashmap = SpatialHashmap(50, 5000);
    setGameMode(GAME_MODE::MAIN_MENU);
    addImage("unit.png");
    addImage("unit_w1.png");
    addImage("unit_w2.png");
    addImage("unit_a1.png");
    addImage("unit_a2.png");
    addImage("unit_a3.png");
    addImage("unit_a4.png");
    addImage("enemy.png");
    addImage("enemy_a1.png");
    addImage("enemy_w1.png");
    addImage("wall.png");
    addImage("farm.png");
    addImage("engine.png");
    addImage("engine_r1.png");
    addImage("engine_r2.png");
    addImage("engine_r3.png");
    addImage("engine_r4.png");
    addImage("engine_r5.png");
    addImage("engine_r6.png");
    addImage("engine_r7.png");
    addImage("engine_r8.png");
    addImage("engine_r9.png");
    addImage("engine_r10.png");
    addImage("engine_r11.png");
    addImage("engine_r12.png");
    addImage("engine_r13.png");
    addImage("engine_r14.png");
    addImage("engine_r15.png");
    addImage("blade.png");
    addImage("back.png");
    addImage("edge.png");
    addImage("edge_v.png");
    addImage("edge_v2.png");
}
void Game::loop() {
    bool above_click = false;
    switch (mode) {
        case GAME_MODE::MAIN_MENU:
            above_click = main_menu_loop();    
        break;
        case GAME_MODE::PAUSE_MENU:
            above_click = pause_menu_loop();
            break;
        case GAME_MODE::GAME_LOOP:
            cameraUpdate();
            above_click = game_loop();
        break;
        case GAME_MODE::LEVEL_EDITOR:
            cameraUpdate();
            above_click = level_editor_loop();
        break;
    }
    updateButtons(above_click);
}

inline uint32_t floatToBytes (float n) {
    uint32_t m;
    std::memcpy(&m, &n, sizeof(uint32_t));
    return m;
}

void Game::save(std::string path) {
    std::ofstream file(path, std::ios::out | std::ios::binary);
    // write money and food
    size_t siz = entities.size();
    if (level_edit) {
        this->money = 100;
        this->food = 24;
    }
    file.write(reinterpret_cast<const char*>(&this->money), sizeof(this->money));
    file.write(reinterpret_cast<const char*>(&this->food), sizeof(this->food));
    uint32_t cam = floatToBytes(cam_pos.x);
    file.write(reinterpret_cast<const char*>(&cam), sizeof(float));
    cam = floatToBytes(cam_pos.y);
    file.write(reinterpret_cast<const char*>(&cam), sizeof(float));
    cam = floatToBytes(cam_zoom);
    file.write(reinterpret_cast<const char*>(&cam), sizeof(float));
    file.write(reinterpret_cast<const char*>(&siz), sizeof(unsigned long));
    // serialize entities
    for (auto& i : entities) {
        uint8_t v = i.typeToUint8t();
        file.write(reinterpret_cast<const char*>(&v), sizeof(uint8_t));
        uint32_t data = floatToBytes(i.pos.x);
        file.write(reinterpret_cast<const char*>(&data), sizeof(float));
        data = floatToBytes(i.pos.y);
        file.write(reinterpret_cast<const char*>(&data), sizeof(float));
        data = floatToBytes(i.dimen.x);
        file.write(reinterpret_cast<const char*>(&data), sizeof(float));
        data = floatToBytes(i.dimen.y);
        file.write(reinterpret_cast<const char*>(&data), sizeof(float));
        // write out action?

    }
    file.close();
}
void Game::load(std::string path) {
    pathfinder::calculatePathBenchmark(&spatial_hashmap);
    entities.clear();
    enemies.clear();
    selected = nullptr;
    spatial_hashmap.clear();
    std::ifstream file(path, std::ios::binary);
    if (file) {
        file.read(reinterpret_cast<char*>(&this->money), sizeof(this->money));
        file.read(reinterpret_cast<char*>(&this->food), sizeof(this->food));
        // camera
        file.read(reinterpret_cast<char*>(&this->cam_pos.x), sizeof(float));
        file.read(reinterpret_cast<char*>(&this->cam_pos.y), sizeof(float));
        file.read(reinterpret_cast<char*>(&this->cam_zoom), sizeof(float));
        size_t siz = 0;
        file.read(reinterpret_cast<char*>(&siz), sizeof(size_t));
        for (size_t i = 0; i < siz; i++) {
            entity_type type;
            uint8_t v;
            file.read(reinterpret_cast<char*>(&v), sizeof(uint8_t));
            type = entity::uin8tToType(v);
            // read the position and dimensions
            float px, py, dx, dy;
            file.read(reinterpret_cast<char*>(&px), sizeof(float));
            file.read(reinterpret_cast<char*>(&py), sizeof(float));
            file.read(reinterpret_cast<char*>(&dx), sizeof(float));
            file.read(reinterpret_cast<char*>(&dy), sizeof(float));
            entity e = constructEntity({px, py}, {dx, dy}, -1, type);
            entities.push_back(e);
        }
    }
    file.close();
    for (auto& e : entities) spatial_hashmap.insert(&e);
    eng->updateView(cam_pos.x, cam_pos.y, cam_zoom);
}

void Game::new_game () {
    entities.clear();
    enemies.clear();
    selected = nullptr;
    spatial_hashmap.clear();
    this->money = 100;
    this->food = 24;
    this->cam_pos = { 2300, 2300};
    this->cam_zoom = 1.0f;
    this->rpm = 200;
    this->enemy_spawn_timer = 0.0;
    this->enemy_spawn_max = 10.0;
    entity top_left = constructEntity({0, 0}, {5000, 50}, -1, entity_type::MAP_EDGE);
    entity top_right = constructEntity({4950, 0}, {50, 5000}, -1, entity_type::MAP_EDGE);
    entity top_left_2 = constructEntity({0, 0}, {50, 5000}, -1, entity_type::MAP_EDGE);
    entity bottom_left = constructEntity({0, 4950}, {5000, 50}, -1, entity_type::MAP_EDGE);
    entities.push_back(top_left);
    entities.push_back(top_right);
    entities.push_back(top_left_2);
    entities.push_back(bottom_left);
    entity motor = constructEntity({2500, 2500}, {65, 65}, -1, entity_type::MOTOR);
    entities.push_back(motor);
    for (auto& e : entities) spatial_hashmap.insert(&e);
    eng->updateView(cam_pos.x, cam_pos.y, cam_zoom);
}

void Game::levelEditorLoad () {
    entities.clear();
}

void Game::setGameMode(GAME_MODE mode) {
     switch (mode) {
        case GAME_MODE::MAIN_MENU:
            constructMainMenuButtons();
        break;
        case GAME_MODE::PAUSE_MENU:
            constructPauseMenuButtons();
            break;
        case GAME_MODE::GAME_LOOP:
            level_edit = false;
            constructGameButtons();
        break;
        case GAME_MODE::LEVEL_EDITOR:
        {
            level_edit = true;
            new_game();
            this->money = 1000000000;
            this->food = 1000000000;
            constructGameButtons();
        }
        break;
    }
    this->mode = mode;
}

void Game::addFont (std::filesystem::path location, uint32_t dpi) {
    gore::font font = gore::fontloader::loadFont(location, 0, 1024);
    gore::fontraster::rasterizeFont(&font, 96, dpi,  0xFFFFFFFF, 32, 127);
    font_map.insert(location.filename(), font);
}

void Game::renderButton (button b, gore::font* font) {
    static_triangle_r->setColor({0.0f, 0.5f, 1.0f, 1.0f});
    static_triangle_r->drawQuad(b.pos, b.dimen.x, b.dimen.y);
    font_r->drawText(b.text, font, b.pos.x, b.pos.y + BUTTON_TEXT_PT, BUTTON_TEXT_PT, eng->getDPI());
}

void Game::updateButtons (bool above_click) {
    gore::font* font = font_map.get("OpenSans-Regular.ttf");
    font_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    for (auto& i : buttons) {
        if (i.display) renderButton(i, font);
    }
    mouse_click_cooldown += delta;
    if (mouse_click_cooldown > 0.3f) {
        if (eng->getMouseLeftDown() || above_click) {
            mouse_click_cooldown = 0.0f;
            gore::vec2 pos = eng->getMousePos(false, true);
            entity mouse = { pos, {10.0f, 10.0f }};
            for (auto& i : buttons) {
                if (i.display && i.isColliding(mouse)) {
                    i.function(&i);
                    break;
                }
            }
        }
    }
}

entity Game::constructEntity (gore::vec2 pos, gore::vec2 dimen, int imd_id, entity_type type) {
    entity e(pos, dimen, imd_id, type);
    auto unit_func = [&] (entity* e) {
        e->count++;
        const float attack_range = 30.0f;
        spatial_hashmap.remove(e);
        switch (e->action.type) {
        case action_type::NONE:
            if (e->count % 30 == 0) {
                entity* enemy_in_range = spatial_hashmap.getEntityNearby(e, attack_range, entity_type::MASS);
                if (enemy_in_range != nullptr) {
                    int index = -1;
                    for (size_t i = 0; i < enemies.size(); i++) {
                        if (enemy_in_range == &enemies[i]) { index = i; break; }
                    }
                    e->action = { index, action_type::ATTACK };
                }
            }
            break;
        case action_type::ATTACK:
        {
            if (e->action.target < 0 || e->action.target >= (int)enemies.size()) {
                e->action = {-1, action_type::NONE};
                e->path.clear();
                break;
            }
            entity* target = &enemies[e->action.target];
            float dx = target->pos.x - e->pos.x;
            float dy = target->pos.y - e->pos.y;
            float dist = std::sqrtf(dx * dx + dy * dy);
            if (dist <= attack_range) {
                e->path.clear();
                e->attack_cooldown -= delta;
                if (e->attack_cooldown <= 0.0) {
                    addPopup(target->pos, "Damage: " + std::to_string(e->level));
                    target->hp -= e->level;
                    e->attack_cooldown = 1.0;
                }
                if (e->count % 10 == 0) {
                    if (e->img_id < 3) {
                        e->img_id = 3;
                    }
                    e->img_id++;
                    if (e->img_id > 6) {
                        e->img_id = 3;
                    }
                }
            } else {
                // repath on a timer, not every frame
                if (e->count >= 60) {
                    spatial_hashmap.remove(target);
                    e->path = pathfinder::calculatePath(&spatial_hashmap, *e, target->pos);
                    spatial_hashmap.insert(target);
                    e->count = 0;
                }
            }
        }
        break;
        case action_type::COLLECT:
            {
                if (e->action.target < 0 || e->action.target >= (int)entities.size()) {
                    e->action = {-1, action_type::NONE};
                    e->path.clear();
                    break;
                }
                if (e->path.size() > 0 && e->action.target >= 0 && e->path[e->path.size() - 1] == entities[e->action.target].pos) {

                } else if (e->pos == entities[e->action.target].pos) {
                    e->action.type = action_type::RETURN;
                } else {
                    spatial_hashmap.remove(&entities[e->action.target]);
                    spatial_hashmap.remove(&entities[motor_index]);
                    e->path = pathfinder::calculatePath(&spatial_hashmap, *e, entities[e->action.target].pos);
                    spatial_hashmap.insert(&entities[e->action.target]);
                    spatial_hashmap.insert(&entities[motor_index]);
                }
            }
          break;
        case action_type::RETURN:
        {
            if (e->action.target < 0 || e->action.target >= (int)entities.size()) {
                e->action = {-1, action_type::NONE};
                e->path.clear();
                break;
            }
            if (e->count == 120) {
                spatial_hashmap.remove(&entities[motor_index]);
                spatial_hashmap.remove(&entities[e->action.target]);
                e->path = pathfinder::calculatePath(&spatial_hashmap, *e, entities[motor_index].pos);
                spatial_hashmap.insert(&entities[motor_index]);
                spatial_hashmap.insert(&entities[e->action.target]);
            } else if (e->count > 120) {
                if (e->path.size() == 0) {
                    e->count = 0;
                    this->food += 40;
                    e->action.type = action_type::COLLECT;
                }
            }
        }
            break;
        }
        if (e->path.size() > 0) {
            // draw the path
            triangle_r->setColor({1.0f, 0.5f, 0.0f, 1.0f});
            for (auto& j : e->path) {
                triangle_r->addQuad(j, 15, 15);
            }
            triangle_r->drawBuffer();
            gore::vec2 target = e->path[0];
            gore::vec2 dif = target - e->pos;
            if (std::abs(dif.x) < 2.0 && std::abs(dif.y) < 2.0) {
                e->pos = target;
                e->path.erase(e->path.begin());
            } else {
                float angle = std::atan2f(dif.y, dif.x);
                gore::vec2 change = { std::cosf(angle) * 2.0f, std::sinf(angle) * 2.0f };
                e->pos += change;
                if (e->count % 10 == 0) {
                    e->img_id++;
                    if (e->img_id > 2) {
                        e->img_id = 0;
                    }
                }
            }
        }
        spatial_hashmap.insert(e);
    };
    auto motor_func = [&](entity* e) {
        e->count++;
        if (e->count % 5 == 0 && motor_on) {
            e->img_id++;
            if (e->img_id > 15) {
                e->img_id = 0;
            }
        }
        if (food - e->level * 3 >= 0) {
            money += e->level * 2;
        }
        food -= e->level * 3;
        if (food - e->level * 3 <= 0) {
            food = 0;
        }
        if (e->count >= 120) {
            if (motor_on) {
                this->rpm += e->level * 5;
                if (this->rpm > e->level * 200) {
                    this->rpm = e->level * 200;
                }
            } else {
                this->rpm-=50;
            }
            e->count = 0;
        }
        if (this->rpm < 0) {
            this->rpm = 0;
        } 
        if (blades.empty()) {
            float size = (float)e->level * 210.0f;
            float cx = e->pos.x + e->dimen.x / 2.0f;
            float cy = e->pos.y + e->dimen.y / 2.0f;
            const float t = 10.0f; // wall thickness
            entity top    = constructEntity({cx - size/2.0f, cy - size/2.0f},        {size, t},    -1, entity_type::MOTOR_BLADE);
            entity bottom = constructEntity({cx - size/2.0f, cy + size/2.0f - t},    {size, t},    -1, entity_type::MOTOR_BLADE);
            entity left   = constructEntity({cx - size/2.0f, cy - size/2.0f},        {t,    size}, -1, entity_type::MOTOR_BLADE);
            entity right  = constructEntity({cx + size/2.0f - t, cy - size/2.0f},    {t,    size}, -1, entity_type::MOTOR_BLADE);
            top.level = e->level;    top.img_id = 0;
            bottom.level = e->level; bottom.img_id = 1;
            left.level = e->level;   left.img_id = 2;
            right.level = e->level;  right.img_id = 3;
            blades.push_back(top);
            blades.push_back(bottom);
            blades.push_back(left);
            blades.push_back(right);
            //for (auto& b : blades) spatial_hashmap.insert(&b);
        }
    };
    auto blade_func = [&](entity* e) {
        if (e->img_id == 0 && rpm > 0)
            blade_scroll = fmodf(blade_scroll + (float)delta * (float)rpm * 0.05f, 10.0f);
        if (this->rpm > 0) {
            std::vector<entity*> cols = spatial_hashmap.getCollisions(e);
            if (!cols.empty()) {
                // remove hp corresponding to level
                for (auto& i : cols) {
                    i->hp -= e->level;
                    addPopup(i->pos, "Damage: " + std::to_string(e->level));
                    this->rpm -= e->level;
                }
            }
        }
    };
    auto farm_func = [&](entity* e) {
    };
    auto enemy_func = [&](entity* e) {
        e->count++;
        spatial_hashmap.remove(e);
        // scan for targets in attack range first
        const float attack_range = 30.0f;
        std::vector<entity*> nearby = spatial_hashmap.scanAroundEntity(e, attack_range);
        entity* attack_target = nullptr;
        for (auto* s : nearby) {
            if (s->type == entity_type::UNIT || s->type == entity_type::MOTOR ||
                s->type == entity_type::FARM || s->type == entity_type::STRUCTURE) {
                attack_target = s;
                break;
            }
        }
        if (attack_target != nullptr) {
            e->path.clear();
            e->attack_cooldown -= delta;
            if (e->attack_cooldown <= 0.0) {
                if (e->count % 10 == 0) {
                    e->img_id++;
                    if (e->img_id > 1) {
                        e->img_id = 0;
                    }
                }
                attack_target->hp -= e->level;
                addPopup(attack_target->pos, "Damage: " + std::to_string(e->level));
                e->attack_cooldown = 1.0;
            }
        } else if (e->count > 60) {
            // repath toward nearest high-priority target
            std::vector<entity*> scan = spatial_hashmap.scanAroundEntity(e, 200.0f);
            entity* target = &entities[motor_index];
            for (size_t i = 0; i < scan.size(); i++) {
                switch (scan[i]->type) {
                case entity_type::STRUCTURE:
                    break;
                case entity_type::UNIT:
                    target = scan[i];
                    break;
                case entity_type::BUTTON:
                    break;
                case entity_type::MAP_EDGE:
                    break;
                case entity_type::MOTOR:
                    target = scan[i];
                    i = scan.size();
                    break;
                case entity_type::FARM:
                    target = scan[i];
                    i = scan.size();
                    break;
                case entity_type::MASS:
                  break;
                case entity_type::MOTOR_BLADE:
                  break;
                }
            }
            spatial_hashmap.remove(target);
            e->path = pathfinder::calculatePath(&spatial_hashmap, *e, target->pos);
            spatial_hashmap.insert(target);
            e->count = 0;
        }
        if (e->path.size() > 0) {
            gore::vec2 next = e->path[0];
            gore::vec2 dif = next - e->pos;
            if (std::abs(dif.x) < 2.0 && std::abs(dif.y) < 2.0) {
                e->pos = next;
                e->path.erase(e->path.begin());
            } else {
                float angle = std::atan2f(dif.y, dif.x);
                gore::vec2 change = { std::cosf(angle) * 2.0f, std::sinf(angle) * 2.0f };
                e->pos += change;
                if (e->count % 10 == 0) {
                    e->img_id++;
                    if (e->img_id > 2) {
                        e->img_id = 0;
                    }
                }
            }
        }
        spatial_hashmap.insert(e);
    };
    auto draw_health_bar = [&](entity* e) {
        const float bar_w = e->dimen.x;
        const float bar_h = 4.0f;
        const float bar_y = e->pos.y - 8.0f;
        // background
        triangle_r->setColor({0.3f, 0.0f, 0.0f, 1.0f});
        triangle_r->drawQuad({e->pos.x, bar_y}, bar_w, bar_h);
        // foreground
        float pct = std::max(0.0f, (float)e->hp / (float)e->max_hp);
        triangle_r->setColor({0.0f, 1.0f, 0.0f, 1.0f});
        triangle_r->drawQuad({e->pos.x, bar_y}, bar_w * pct, bar_h);
    };
    auto base_render = [&, draw_health_bar](entity* e) {
        triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
        triangle_r->drawQuad(e->pos, e->dimen.x, e->dimen.y);
        if (e->hp < e->max_hp) {
            draw_health_bar(e);
        }
    };
    auto farm_render = [&, draw_health_bar] (entity* e) {
        gore::IMG& img = images["farm.png"];
        if (img) {
            image_r->drawImage(img, e->pos, e->dimen);
        } else {
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
            triangle_r->drawQuad(e->pos, e->dimen.x, e->dimen.y);
        }
        if (e->hp < e->max_hp) {
            draw_health_bar(e);
        }
    };
    auto wall_render = [&, draw_health_bar] (entity* e) {
        gore::IMG& img = images["wall.png"];
        if (img) {
            image_r->drawImage(img, e->pos, e->dimen);
        } else {
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
            triangle_r->drawQuad(e->pos, e->dimen.x, e->dimen.y);
        }
        if (e->hp < e->max_hp) {
            draw_health_bar(e);
        }
    };
    auto unit_render = [&, draw_health_bar](entity* e) {
        gore::IMG* img = &images["unit.png"];
        switch (e->img_id) {
            case 0: img = &images["unit.png"]; break;
            case 1: img = &images["unit_w1.png"]; break;
            case 2: img = &images["unit_w2.png"]; break;
            case 3: img = &images["unit_a1.png"]; break;
            case 4: img = &images["unit_a2.png"]; break;
            case 5: img = &images["unit_a3.png"]; break;
            case 6: img = &images["unit_a4.png"]; break;
        }
        if (*img) {
            image_r->drawImage(*img, e->pos, e->dimen);
        } else {
            triangle_r->setColor({0.2f, 0.6f, 1.0f, 1.0f});
            triangle_r->drawQuad(e->pos, e->dimen.x, e->dimen.y);
        }
        draw_health_bar(e);
    };
    auto edge_render = [&](entity* e) {
        gore::IMG* img = &images["edge.png"];
        if (img) {
            // Determine rotation so the image's left side always faces the map interior.
            // Rotation is around the tile's top-left corner, so pos is compensated
            // to keep each tile in its correct screen position.
            bool horizontal = e->dimen.x > e->dimen.y;
            gore::vec4 uvs;
            if (horizontal) {
                if (e->pos.y < 100.0f) {
                    // top wall — vertical flip
                    uvs = {0, 0, 1, 1};
                    img = &images["edge_v.png"];
                } else {
                    // bottom wall — vertical flip
                    uvs = {0, 0, 1, 1};
                    img = &images["edge_v2.png"];
                }
            } else {
                if (e->pos.x < 100.0f) {
                    // left wall — no flip
                    uvs = {0, 0, 1, 1};
                } else {
                    // right wall — horizontal flip
                    uvs = {1, 0, -1, 1};
                }
            }
            for (float x = e->pos.x; x < e->pos.x + e->dimen.x; x += 50.0f) {
                for (float y = e->pos.y; y < e->pos.y + e->dimen.y; y += 50.0f) {
                    image_r->addImageVertex((*img)->tex, {x, y}, {50.0f, 50.0f}, uvs, 0.0f);
                }
            }
            image_r->drawBuffer();
        } else {
            triangle_r->setColor({0.0f, 1.0f, 0.0f, 1.0f});
            triangle_r->drawQuad(e->pos, e->dimen.x, e->dimen.y);
        }
        edge_count++;
    };
    auto motor_render = [&, draw_health_bar](entity* e) {
        gore::IMG* img = &images["engine.png"];
        switch (e->img_id) {
            case 0: img = &images["engine.png"]; break;
            case 1: img = &images["engine_r1.png"]; break;
            case 2: img = &images["engine_r2.png"]; break;
            case 3: img = &images["engine_r3.png"]; break;
            case 4: img = &images["engine_r4.png"]; break;
            case 5: img = &images["engine_r5.png"]; break;
            case 6: img = &images["engine_r6.png"]; break;
            case 7: img = &images["engine_r7.png"]; break;
            case 8: img = &images["engine_r8.png"]; break;
            case 9: img = &images["engine_r9.png"]; break;
            case 10: img = &images["engine_r10.png"]; break;
            case 11: img = &images["engine_r11.png"]; break;
            case 12: img = &images["engine_r12.png"]; break;
            case 13: img = &images["engine_r13.png"]; break;
            case 14: img = &images["engine_r14.png"]; break;
            case 15: img = &images["engine_r15.png"]; break;
        }
        if (*img) {
            image_r->drawImage(*img, e->pos, e->dimen);
        } else {
            triangle_r->setColor({1.0f, 0.0f, 0.3f, 1.0f});
            triangle_r->drawQuad(e->pos, e->dimen.x, e->dimen.y);
        }
        draw_health_bar(e);
    };
    auto mass_render = [&, draw_health_bar](entity* e) {
        gore::IMG* img = &images["enemy.png"];
        switch (e->img_id) {
            case 0: img = &images["enemy.png"]; break;
            case 1: img = &images["enemy_a1.png"]; break;
            case 2: img = &images["enemy_w1.png"]; break;
        }
        if (*img) {
            image_r->drawImage(*img, e->pos, e->dimen);
        } else {
            triangle_r->setColor({1.0f, 0.0f, 0.0f, 1.0f});
            triangle_r->drawQuad(e->pos, e->dimen.x, e->dimen.y);
        }
        draw_health_bar(e);
    };
    auto blade_render = [&](entity* e) {
        gore::IMG& img = images["blade.png"];
        if (img) {
            float offset = blade_scroll;
            float sx, sy, ex, ey;
            switch (e->img_id) {
                case 0: // top — scroll right: grid shifts right, enter from left
                    sx = e->pos.x + offset - 10.0f; sy = e->pos.y;
                    ex = e->pos.x + e->dimen.x;     ey = e->pos.y + e->dimen.y;
                    break;
                case 1: // bottom — scroll left: grid shifts left, enter from right
                    sx = e->pos.x - offset;     sy = e->pos.y;
                    ex = e->pos.x + e->dimen.x; ey = e->pos.y + e->dimen.y;
                    break;
                case 2: // left — scroll up: grid shifts up, enter from bottom
                    sx = e->pos.x; sy = e->pos.y - offset;
                    ex = e->pos.x + e->dimen.x; ey = e->pos.y + e->dimen.y;
                    break;
                default: // right (3) — scroll down: grid shifts down, enter from top
                    sx = e->pos.x; sy = e->pos.y + offset - 10.0f;
                    ex = e->pos.x + e->dimen.x; ey = e->pos.y + e->dimen.y;
                    break;
            }
            for (float x = sx; x < ex; x += 10.0f)
                for (float y = sy; y < ey; y += 10.0f)
                    image_r->addImageVertex(img->tex, {x, y}, {10.0f, 10.0f});
            image_r->drawBuffer();
        } else {
            triangle_r->setColor({1.0f, 0.0f, 0.3f, 1.0f});
            triangle_r->drawQuad(e->pos, e->dimen.x, e->dimen.y);
        }
    };
    auto motor_select = [&](entity* e, gore::vec2 start, gore::font* font) {
        uint32_t cost = (e->level + 1) * 100;
        font_r->drawText("Upgrade: " + std::to_string(cost), font, start.x, start.y, 16, eng->getDPI());
        std::string turn = ((motor_on) ? "Off" : "On");
        font_r->drawText("Turn " + turn, font, start.x, start.y + 24, 16, eng->getDPI());
        gore::vec2 mouse = eng->getMousePos(false, true);
        entity mouse_e = { mouse, {10.0f, 10.0f} };
        entity upgrade_btn = { {start.x, start.y - 16.0f}, {150.0f, 20.0f} };
        entity turn_btn = { {start.x, start.y + 24}, {150.0f, 20.0f}};
        bool mouse_down = eng->getMouseLeftDown();
        if (mouse_down && upgrade_btn.isColliding(mouse_e)) {
            if ((int64_t)cost <= money) {
                money -= cost;
                e->level++;
                for (auto& b : blades) spatial_hashmap.remove(&b);
                blades.clear();
            }
        } else if (e->count % 90 == 0 && mouse_down && turn_btn.isColliding(mouse_e)) {
            this->motor_on = !motor_on;
        }
    };
    auto blade_select = [&](entity* e, gore::vec2 start, gore::font* font) {
        // render the RPM
        font_r->drawText("RPM: " + std::to_string(rpm), font, start.x + 74, start.y, 16, eng->getDPI());
    };
    e.render = base_render;
    switch (type) {
    case entity_type::STRUCTURE:
        e.hp = 20;
        e.max_hp = 20;
        e.render = wall_render;
        break;
    case entity_type::UNIT:
        e.update = unit_func;
        e.render = unit_render;
        e.hp = 10;
        e.img_id = 0;
        break;
    case entity_type::BUTTON:
        // don't use this function for this
        throw std::runtime_error("DONT USE CONSTRUCT ENTITY FOR BUTTONS DUMMY!");
        break;
    case entity_type::MAP_EDGE:
        e.render = edge_render;
        e.hp = 99999;
        break;
    case entity_type::MOTOR:
        e.update = motor_func;
        e.render = motor_render;
        e.selection = motor_select;
        e.hp = 1000;
        e.max_hp = 1000;
        e.img_id = 0;
      break;
    case entity_type::FARM:
        e.update = farm_func;
        e.render = farm_render;
        e.hp = 20;
        e.max_hp = 20;
      break;
    case entity_type::MASS:
        e.update = enemy_func;
        e.render = mass_render;
        e.img_id = 0;
        e.hp = 10;
      break;
    case entity_type::MOTOR_BLADE:
        e.render = blade_render;
        e.update = blade_func;
        e.selection = blade_select;
        e.hp = 999999999;
        break;
    }
    return e;
}



gore::vec2 Game::randomLocation () {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_int_distribution<int> edge_dist(0, 3);
    static std::uniform_real_distribution<float> pos_dist(51.0f, 4949.0f);

    float along = pos_dist(rng);
    switch (edge_dist(rng)) {
        case 0: return { along,   60.0f   }; // top
        case 1: return { along,   4900.0f }; // bottom
        case 2: return { 60.0f,   along   }; // left
        default:return { 4900.0f, along   }; // right
    }
}

void Game::addPopup (gore::vec2 pos, std::string text) {
    static std::mt19937 rng(std::random_device{}());
    static std::uniform_real_distribution<float> pos_offset(-15.0f, 15.0f);
    gore::vec2 out_pos = { pos.x + pos_offset(rng), pos.y + pos_offset(rng)};
    button button(out_pos, nullptr, text);
    button.count = 0;
    button.hp = 255;
    popups.push_back(button);
}

void Game::renderPopups () {
    gore::font* font = font_map.get("OpenSans-Regular.ttf");
    for (size_t i = 0; i < popups.size();) {
        static_font_r->setColor({1.0f, 1.0f, 1.0f, (float)popups[i].hp/255.0f});
        static_font_r->drawText(popups[i].text, font, popups[i].pos.x, popups[i].pos.y, 10, eng->getDPI());
        popups[i].count++;
        popups[i].hp--;
        if (popups[i].hp <= 0) {
            popups.erase(popups.begin() + i);
        } else {
            i++;
        }
    }
}

void Game::renderBackground () {
    gore::IMG& img = images["back.png"];
    const float width = spatial_hashmap.getGridWidth();
    for (float y = 50.0f; y < width; y += 50.0f) {
        for (float x = 50.0f; x < width; x += 50.0f) {
                image_r->addImageVertex(img->tex, {x, y}, {50.0f, 50.0f});
        }
    }
    image_r->drawBuffer();
}
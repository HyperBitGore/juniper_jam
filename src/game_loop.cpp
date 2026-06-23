#include "entity.hpp"
#include "g_engine/rendering/font_renderer.hpp"
#include "g_engine/util/shader.hpp"
#include "game.hpp"
#include "path.hpp"
#include <string>

bool render_right_click_dropdown = false;
entity* selected = nullptr;

bool Game::game_loop() {
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
            gore::vec2 pos = eng->getMousePos();
            entity mouse = { pos, {10.0f, 10.0f }};
            if (!render_right_click_dropdown) {
                mouse_click_cooldown = 0.0f;
                std::vector<entity*> collisions = spatial_hashmap.getCollisions(&mouse);
                if (collisions.size() > 0 && collisions[0]->type == entity_type::UNIT) {
                    selected = collisions[0];                    
                } else if (selected != nullptr) {
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
                gore::vec2 pos = eng->getMousePos();
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
        spatial_hashmap.remove(&entities[i]);
        if (entities[i].path.size() > 0) {
            // draw the path
            triangle_r->setColor({1.0f, 0.5f, 0.0f, 1.0f});
            for (auto& j : entities[i].path) {
                triangle_r->addQuad(j, 15, 15);
            }
            triangle_r->drawBuffer();
            gore::vec2 target = entities[i].path[0];
            gore::vec2 dif = target - entities[i].pos;
            if (std::abs(dif.x) < 2.0 && std::abs(dif.y) < 2.0) {
                entities[i].pos = target;
                entities[i].path.erase(entities[i].path.begin());
            } else {
                float angle = std::atan2f(dif.y, dif.x);
                gore::vec2 change = { std::cosf(angle) * 2.0f, std::sinf(angle) * 2.0f };
                entities[i].pos += change;
            }
        }
        if (spatial_hashmap.checkCollision(&entities[i])) {
            std::cout << "colliding!" << entities[i].pos.x << ", " << entities[i].pos.y << "\n";
        }
        if (&entities[i] == selected) {
            triangle_r->setColor({0.0f, 1.0f, 0.0f, 1.0f});
            triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
        } else {
            triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
        }
        spatial_hashmap.insert(&entities[i]);
    }
    font_r->setColor({1.0f, 0.0f, 0.5f, 1.0f});
    font_r->drawText("Money: " + std::to_string(this->money), font, 0, 32, 24, eng->getDPI());
    font_r->drawText("Food: " + std::to_string(this->food), font, 0, 64, 24, eng->getDPI());
    font_r->drawText("Mouse: " + std::to_string(m_pos.x) + ", " + std::to_string(m_pos.y), font, 0, 128, 24, eng->getDPI());
    return above_click;
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
        } else {
            triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
        }
    }
    font_r->setColor({1.0f, 0.0f, 0.5f, 1.0f});
    font_r->drawText("Money: " + std::to_string(this->money), font, 0, 32, 24, eng->getDPI());
    font_r->drawText("Food: " + std::to_string(this->food), font, 0, 64, 24, eng->getDPI());

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
                gore::vec2 pos = eng->getMousePos();
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
        triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
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
        entity e = { buttons[0].pos, {5.0f, 5.0f}, -1, entity_type::UNIT };
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
        entity e = { buttons[0].pos, {50.0f, 50.0f}, -1, entity_type::STRUCTURE };
        entities.push_back(e);
        spatial_hashmap.insert(&entities.back());
        for (auto& btn : buttons) btn.display = false;
        render_right_click_dropdown = false;
    };
    button wall_button({0, 0}, add_wall, "Buy Wall");
    wall_button.display = false;
    buttons.push_back(wall_button);
}
void Game::constructLevelEditorButtons () {
    buttons.clear();
    auto add_worker = [&](button* b) {
        entity e = { buttons[0].pos, {5.0f, 5.0f}, -1, entity_type::UNIT };
        entities.push_back(e);
        spatial_hashmap.insert(&entities.back());
        for (auto& btn : buttons) btn.display = false;
        render_right_click_dropdown = false;
    };
    button worker_button({0, 0}, add_worker, "Place Worker");
    worker_button.display = false;
    buttons.push_back(worker_button);

    auto add_wall = [&](button* b) {
        entity e = { buttons[0].pos, {50.0f, 50.0f}, -1, entity_type::STRUCTURE };
        entities.push_back(e);
        spatial_hashmap.insert(&entities.back());
        for (auto& btn : buttons) btn.display = false;
        render_right_click_dropdown = false;
    };
    button wall_button({0, 0}, add_wall, "Place Wall");
    wall_button.display = false;
    buttons.push_back(wall_button);
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


void Game::cameraUpdate () {
    camera_update += delta;
    cam_move += delta;
    bool update_camera = false;
    if (cam_move > 0.001) {
        if (eng->getKeyDown(g_a)) {
            cam_pos.x -= 0.1f;
            update_camera = true;
        } else if (eng->getKeyDown(g_d)) {
            cam_pos.x += 0.1f;
            update_camera = true;
        } else if (eng->getKeyDown(g_w)) {
            cam_pos.y -= 0.1f;
            update_camera = true;
        } else if (eng->getKeyDown(g_s)) {
            cam_pos.y += 0.1f;
            update_camera = true;
        }
    }
    if (camera_update > 0.008) {
        if (eng->getKeyDown(g_z)) {
            cam_zoom += 0.01f;
            update_camera = true;
        } else if (eng->getKeyDown(g_x)) {
            cam_zoom -= 0.01f;
            update_camera = true;
        } else if (eng->getKeyDown(g_r)) {
            cam_zoom = 1.0f;
            update_camera = true;
        }
    }
    if (update_camera) {
        camera_update = 0.0;
        eng->updateView(cam_pos.x, cam_pos.y, cam_zoom);
    }
}
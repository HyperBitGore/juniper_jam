#include "entity.hpp"
#include "g_engine/rendering/font_renderer.hpp"
#include "game.hpp"
#include "path.hpp"

double mouse_click_cooldown = 0.0f;
bool render_right_click_dropdown = false;
entity add_entity = { {0.0f, 0.0f}, {100.0f, 30.0f}};
entity* selected = nullptr;

void Game::game_loop() {
    mouse_click_cooldown += delta;
    if (mouse_click_cooldown > 0.3f) {
        if (eng->getMouseLeftDown()) {
            mouse_click_cooldown = 0.0f;
            gore::vec2 pos = eng->getMousePos();
            entity mouse = { pos, {10.0f, 10.0f }};
            if (render_right_click_dropdown) {
                if (add_entity.isColliding(mouse)) {
                    entities.push_back({add_entity.pos, {5.0f, 5.0f}});
                    render_right_click_dropdown = false;
                }
            } else {
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
            if (!render_right_click_dropdown) {
                render_right_click_dropdown = true;
                gore::vec2 pos = eng->getMousePos();
                add_entity.pos = pos;
            } else {
                render_right_click_dropdown = false;
            }
        }
    }
    if (render_right_click_dropdown) {
        gore::font* font = font_map.get("OpenSans-Regular.ttf");
        triangle_r->setColor({0.0f, 0.5f, 1.0f, 1.0f});
        triangle_r->drawQuad(add_entity.pos, add_entity.dimen.x, add_entity.dimen.y);
        font_r->drawText("Add entity", font, add_entity.pos.x, add_entity.pos.y + 30, 24, eng->getDPI());
    }
    triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
    for (size_t i = 0; i < entities.size(); i++) {
        spatial_hashmap.remove(&entities[i]);
        if (entities[i].path.size() > 0) {
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
            std::cout << "colliding!\n";
        }
        if (&entities[i] == selected) {
            triangle_r->drawBuffer();
            triangle_r->setColor({0.0f, 1.0f, 0.0f, 1.0f});
            triangle_r->drawQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
            triangle_r->setColor({1.0f, 1.0f, 1.0f, 1.0f});
        } else {
            triangle_r->addQuad(entities[i].pos, entities[i].dimen.x, entities[i].dimen.y);
        }
        spatial_hashmap.insert(&entities[i]);
    }
    triangle_r->drawBuffer();
}

void Game::pause_menu_loop () {

}
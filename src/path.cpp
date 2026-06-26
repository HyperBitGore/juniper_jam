#include "path.hpp"
#include "entity.hpp"
#include "game.hpp"
#include <queue>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <unordered_set>

inline bool floatEq (float a, float b, float tolerance = 1e-5f) {
    return std::abs(a - b) <= tolerance;
}

struct cell {
    gore::vec2 pos;
    float g;
    float h;
    float f;
    int parent = -1;
    bool operator<(const cell& c) const { return this->f > c.f; }
};

cell create_cell (gore::vec2 pos, gore::vec2 target) {
    float dx = target.x - pos.x;
    float dy = target.y - pos.y;
    float h = std::sqrt(dx * dx + dy * dy);
    return { pos, 0.0f, h, h, -1 };
}

gore::vec2 raycastScan (SpatialHashmap* map, gore::vec2 start, gore::vec2 end, float radians, float width, uint32_t length) {
    gore::vec2 pos = start;
    float dx = end.x - pos.x;
    float dy = end.y - pos.y;
    float angle = std::atan2f(dy, dx);
    float angle_cos = std::cosf(angle);
    float angle_sin = std::sinf(angle);
    gore::vec2 change = {angle_cos, angle_sin };
    for (uint32_t i = 0; i < length; i++) {
        if (map->raycastTo(pos, end, width) == nullptr) {
            return pos;
        }
        pos += change;
    }
    return { -1, -1};
}

std::vector<gore::vec2> pathBetweenCells (SpatialHashmap* map, gore::vec2 start, gore::vec2 end, float width) {
    entity e = { start, {width, width}};
    if (map->raycastTo(start, end, width) == nullptr) {
        return { start, end };
    }
    // raycast in all directions till one of the directions can raycast to the end position
    std::vector<gore::vec2> out;
    out.push_back(start);
    for (float ang = 0.0f; ang < 360.0f; ang += 15.0f) {
        float rad = ang * M_PI/180.0;
        gore::vec2 ray = raycastScan(map, start, end, rad, width, 500);
        if (ray.x != -1) {
            out.push_back(ray);
            break;
        }
    }
    out.push_back(end);
    if (out.size() == 2) {
        std::cout << "path not found btw\n";
    }
    return out;
}

// convert cell to use spatialhashmap cells
//     - treat like grid for now
//     - once grid version working do intercell pathfinding
// TODO
//      - path to eight corners of cell, instead of just the top left and can skip raycast ending loop?
//      - optimize
//          - need to get path down to 100 microseconds on average (in release build)
std::vector<gore::vec2> findPath (SpatialHashmap* map, entity e, gore::vec2 target) {
    if (map->raycastTo(e.pos, target, e.dimen.x, {entity_type::UNIT, entity_type::MOTOR, entity_type::FARM, entity_type::MASS}) == nullptr) {
        return { e.pos, target };
    }
    std::priority_queue<cell> queue;
    std::vector<cell> closed;
    std::unordered_set<uint64_t> visited;

    auto cell_key = [&](gore::vec2 pos) -> uint64_t {
        uint32_t cx = (uint32_t)(pos.x / map->getCellSize());
        uint32_t cy = (uint32_t)(pos.y / map->getCellSize());
        return ((uint64_t)cx << 32) | cy;
    };

    cell first = create_cell(e.pos, target);
    queue.push(first);
    //visited.insert(cell_key(first.pos));
    const int MAX_NODES = 5000;
    int nodes_expanded = 0;
    while(!queue.empty() && nodes_expanded < MAX_NODES) {
        cell least = queue.top();
        visited.insert(cell_key(least.pos));
        queue.pop();
        nodes_expanded++;
        // add a closed list here eventually
        closed.push_back(least);
        // check if at target
        if (map->raycastTo(least.pos, target, e.dimen.x) == nullptr) {
            // collect cell waypoints
            std::vector<gore::vec2> waypoints;
            waypoints.push_back(target);
            waypoints.push_back(least.pos);
            int idx = least.parent;
            // now check which cell neighbor position has lowest value
            while (idx != -1) {
                std::vector<gore::vec2> neighbors = map->getCellNeighborsCorners(closed[idx].pos.x, closed[idx].pos.y);
                // loop through positions and decide based on lowest f cost
                waypoints.push_back(closed[idx].pos);

                idx = closed[idx].parent;
            }
            std::reverse(waypoints.begin(), waypoints.end());

            // expand each cell-to-cell segment with sub-cell precision
            std::vector<gore::vec2> path;
            for (size_t i = 0; i + 1 < waypoints.size(); i++) {
                std::vector<gore::vec2> segment = pathBetweenCells(map, waypoints[i], waypoints[i + 1], e.dimen.x);
                for (size_t j = 0; j < segment.size(); j++) {
                    path.push_back(segment[j]);
                }
            }
            return path;
        }
        // get neighbor cells and add to queue
        std::vector<gore::vec2> neighbors = map->getCellNeighborPositions(least.pos.x, least.pos.y);
        for (auto& i : neighbors) {
            entity n = { i, e.dimen };
            // only walls and map edges are solid — skip non-solid entities in traversal
            entity* ray = map->raycastTo(least.pos, i, e.dimen.x, {
                entity_type::UNIT, entity_type::FARM, entity_type::MOTOR,
                entity_type::MASS, entity_type::MOTOR_BLADE, entity_type::BUTTON
            });
            if (ray != nullptr) {
                continue;
            }
            std::vector<entity*> cols = map->getCollisions(&n);
            bool wall_blocked = false;
            for (auto* c : cols) {
                if (c->type == entity_type::STRUCTURE || c->type == entity_type::MAP_EDGE) {
                    wall_blocked = true;
                    break;
                }
            }
            if (wall_blocked) continue;

            // check if in closed
            /*bool in_closed = false;
            for (auto& j : closed) {
                if (i.x == j.pos.x && i.y == j.pos.y) {
                    in_closed = true;
                    break;
                }
            }
            if (in_closed) {
                continue;
            }*/
            uint64_t nkey = cell_key(i);
            if (visited.count(nkey)) continue;
            //visited.insert(nkey);
            cell c = create_cell(i, target);
            c.parent = (int)closed.size() - 1;
            c.g = least.g + 10.0f;
            c.f = c.g + c.h;
            queue.push(c);
        }
    }
    return {};
}

std::vector<gore::vec2> pathfinder::calculatePath (SpatialHashmap* map, entity e, gore::vec2 end_pos) {
    // check if target inside a structure, don't path
    {
        entity temp = { end_pos, e.dimen};
        std::vector<entity*> collisions = map->getCollisions(&temp);
        // only solid obstacles block pathing to the target; units/enemies at the target are fine
        collisions.erase(std::remove_if(collisions.begin(), collisions.end(), [](entity* ent) {
            return ent->type != entity_type::STRUCTURE && ent->type != entity_type::MAP_EDGE;
        }), collisions.end());
        if (collisions.size() > 0 || end_pos.x > map->getGridWidth() || end_pos.x < 0 || end_pos.y > map->getGridWidth() || end_pos.y < 0) {
            return {};
        }
    }
    // use A* to calculate path
    std::vector<gore::vec2> found = findPath(map, e, end_pos);
    return found;
}

std::vector<gore::vec2> pathfinder::enemyPath (SpatialHashmap* map, entity e, gore::vec2 end_pos, int motor_level) {
    // walls can only exist inside the blade square, so outside it is always open field
    float size = (float)motor_level * 210.0f;
    float bx0 = end_pos.x - size / 2.0f;
    float by0 = end_pos.y - size / 2.0f;
    float bx1 = end_pos.x + size / 2.0f;
    float by1 = end_pos.y + size / 2.0f;
    bool inside = e.pos.x >= bx0 && e.pos.x + e.dimen.x <= bx1
               && e.pos.y >= by0 && e.pos.y + e.dimen.y <= by1;
    if (!inside) {
        return { e.pos, end_pos };
    }
    std::vector<gore::vec2> found = findPath(map, e, end_pos);
    return found;
}


void pathfinder::calculatePathBenchmark(SpatialHashmap* map) {
    std::random_device rd;
    std::mt19937 gen(rd());
    float gw = static_cast<float>(map->getGridWidth());
    float cs = static_cast<float>(map->getCellSize());
    int cells_per_axis = static_cast<int>(gw / cs);
    std::uniform_int_distribution<int> cell_dis(0, cells_per_axis - 1);
    std::uniform_real_distribution<float> pos_dis(0.0f, gw - cs);

    // insert random wall entities aligned to cell boundaries
    const int num_walls = 500;
    std::vector<entity> walls;
    walls.reserve(num_walls);
    for (int i = 0; i < num_walls; i++) {
        gore::vec2 pos = {
            static_cast<float>(cell_dis(gen)) * cs,
            static_cast<float>(cell_dis(gen)) * cs
        };
        walls.push_back(entity(pos, { cs, cs }, -1, entity_type::STRUCTURE));
    }
    for (auto& w : walls) map->insert(&w);

    const int num_paths = 100;
    const gore::vec2 entity_dimen = { 5.0f, 5.0f };

    int found = 0;
    int not_found = 0;
    auto total_start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < num_paths; i++) {
        gore::vec2 start = { pos_dis(gen), pos_dis(gen) };
        gore::vec2 end   = { pos_dis(gen), pos_dis(gen) };
        entity e(start, entity_dimen);

        std::vector<gore::vec2> path = calculatePath(map, e, end);
        if (path.size() > 0) found++;
        else not_found++;
    }

    auto total_end = std::chrono::high_resolution_clock::now();
    auto total_us = std::chrono::duration_cast<std::chrono::microseconds>(total_end - total_start).count();

    for (auto& w : walls) map->remove(&w);

    std::cout << "calculatePath benchmark (" << num_paths << " paths, " << num_walls << " random walls)\n";
    std::cout << "  found: " << found << "  not found: " << not_found << "\n";
    std::cout << "  total: " << total_us << " us"
              << "  avg: " << (total_us / num_paths) << " us/path\n";
}
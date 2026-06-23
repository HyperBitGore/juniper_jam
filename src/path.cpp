#include "path.hpp"
#include "entity.hpp"
#include "game.hpp"
#include <queue>
#include <cmath>
#include <algorithm>

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
// convert cell to use spatialhashmap cells
//     - treat like grid for now
//     - once grid version working at intercell pathfinding
std::vector<gore::vec2> findPath (SpatialHashmap* map, entity e, gore::vec2 target) {
    if (map->raycastTo(e.pos, target, e.dimen.x) == nullptr) {
        return { e.pos, target };
    }
    std::priority_queue<cell> queue;
    std::vector<cell> closed;
    cell first = create_cell(e.pos, target);
    queue.push(first);
    while(!queue.empty()) {
        cell least = queue.top();
        queue.pop();
        // add a closed list here eventually
        closed.push_back(least);
        // check if at target
        if (map->raycastTo(least.pos, target, e.dimen.x) == nullptr) {
            std::vector<gore::vec2> path;
            path.push_back(target);
            int idx = least.parent;
            while (idx != -1) {
                path.push_back(closed[idx].pos);
                idx = closed[idx].parent;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }
        // get neighbor cells and add to queue
        std::vector<gore::vec2> neighbors = map->getCellNeighborPositions(least.pos.x, least.pos.y); 
        for (auto& i : neighbors) {
            entity n = { i, e.dimen };
            // check if raycast to this neighbor from cur position will collide
            entity* collision = map->checkCollision(&n);
            if (collision != nullptr) {
                continue;
            }

            // check if in closed
            bool in_closed = false;
            for (auto& j : closed) {
                if (i.x == j.pos.x && i.y == j.pos.y) {
                    in_closed = true;
                    break;
                }
            }
            if (in_closed) {
                continue;
            }
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
        if (collisions.size() > 0) {
            return {};
        }
    }
    // use A* to calculate path
    std::vector<gore::vec2> found = findPath(map, e, end_pos);
    return found;
}
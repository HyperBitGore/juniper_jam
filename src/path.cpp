#include "path.hpp"
#include "entity.hpp"
#include "game.hpp"
#include <queue>

inline bool floatEq (float a, float b, float tolerance = 1e-5f) {
    return std::abs(a - b) <= tolerance;
}

struct a_path_point {
    gore::vec2 pos;
    float g; // actual cost to reach node from start
    float h; // heuristic (Euclidean distance for this sqrt(dx^2+dy^2))
    float f; // total estimated cost
    bool operator<(const a_path_point& other) const {
        return this->f > other.f; // min-heap: lowest f has priority
    }
};

struct a_path {
    std::vector<a_path_point> path;
};

inline gore::vec2 raycastTo (SpatialHashmap* map, gore::vec2 start, gore::vec2 target) {
    entity e(start, {1, 1});
    // loop along angle towards target and move until we hit a blocking object
    while (!floatEq(e.pos.x, target.x, 1e-2f) && !floatEq(e.pos.y, target.y, 1e-2f)) {
        std::vector<entity*> collisions = map->getCollisions(&e);
        if (collisions.size() > 0) {
            break;
        }
        float dx = target.x - e.pos.x;
        float dy = target.y - e.pos.y;
        if (std::sqrtf(dx * dx + dy * dy) <= 2.0f) {
            e.pos = target;
            break;
        }
        float angle = std::atan2f(dy, dx);
        float angle_cos = std::cosf(angle);
        float angle_sin = std::sinf(angle);
        gore::vec2 change = {angle_cos, angle_sin };
        e.pos += change;
    }
    return e.pos;
}

inline a_path_point calculatePoint (gore::vec2 pos, gore::vec2 target, float cur_g) {
    a_path_point a;
    float dx = target.x - pos.x;
    float dy = target.y - pos.y;
    a.pos = pos;
    a.g = cur_g;
    a.h = std::sqrtf(dx * dx + dy * dy);
    a.f = a.g + a.h;
    return a;
}

// recursive A* function
// convert this to Theta*, since we can raycast
a_path calculateAPath (SpatialHashmap* map, gore::vec2 cur_pos, gore::vec2 target, float cur_g, gore::vec2 entity_dimen) {
    std::priority_queue<a_path_point> queue;
    a_path_point start = calculatePoint(cur_pos, target, cur_g);
    queue.emplace(start);
    a_path path;
    while(!queue.empty()) {
        a_path_point lowest = queue.top();
        queue.pop();
        const float step = std::min(entity_dimen.x, entity_dimen.y);
        float dx = lowest.pos.x - target.x;
        float dy = lowest.pos.y - target.y;
        if (std::sqrtf(dx*dx + dy*dy) <= step * 0.5f) {
            path.path.push_back(lowest);
            return path;
        } else {
            for (float angle = 0.0f; angle < 360.0f; angle += 15.0f) {
                float rad = angle * (3.14159265f / 180.0f);
                gore::vec2 neighbor_pos = {
                    lowest.pos.x + std::cosf(rad) * step,
                    lowest.pos.y + std::sinf(rad) * step
                };
                entity temp(neighbor_pos, { step, step });
                if (map->getCollisions(&temp).size() > 0) continue;
                queue.emplace(calculatePoint(neighbor_pos, target, lowest.g + step));
            }
        }
    }
    return {};
}


a_path findPath (SpatialHashmap* map, entity e, gore::vec2 end_pos, std::vector<a_path_point> current_path) {
    a_path target;
    // raycast till we hit an obstacle
    gore::vec2 cur_pos = raycastTo(map, e.pos, end_pos);
    if (cur_pos.x == end_pos.x && cur_pos.y == end_pos.y) {
        target.path.push_back({e.pos, 0, 0, 0});
        target.path.push_back({cur_pos, 0, 0, 0});
        return target;
    }
    // calculate A* path from hit point
    a_path a = calculateAPath(map, cur_pos, end_pos, 0, e.dimen);
    // concat a to target
    for (auto& i : a.path) {
        target.path.push_back(i);
    }
    return target;
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
    a_path found = findPath(map, e, end_pos, {});
    // convert the computed path to path for entity to use
    std::vector<gore::vec2> out_path;
    for (auto& i : found.path) {
        out_path.push_back(i.pos);
    }
    return out_path;
}
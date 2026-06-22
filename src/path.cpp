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
// step through spatialhashmap grid cells instead of just free range raycasts
//      - grab lowest cost hashmap cell
//      - parse next hashmap cell
//          - calculate path through cell
//             - step through using a star to dictate the path through the cell, lowest h is the path
//      - get grid neighbors and throw into queue
// parse a spatialhashmap grid cell

struct map_grid_cell {
    std::vector<entity*>* bucket;
    gore::vec2 start;
    gore::vec2 end;
    float g;
    float f;
    float h;
    int parent = -1;
    bool operator<(const map_grid_cell& other) const {
        return this->f > other.f; // min-heap: lowest f has priority
    }
    bool operator==(const map_grid_cell& other) const {
        return this->bucket == other.bucket;
    }
};

map_grid_cell createCell (SpatialHashmap* map, gore::vec2 pos, gore::vec2 target, float g) {
    float dx = target.x - pos.x;
    float dy = target.y - pos.y;
    float h = std::sqrtf(dx * dx + dy * dy);
    return { map->getBucket(pos.x, pos.y), pos, {}, g, g + h, h };
}

std::vector<gore::vec2> pathThroughCell (SpatialHashmap* map, map_grid_cell& cell, gore::vec2 start, gore::vec2 target) {
    float cs = static_cast<float>(map->getCellSize());
    gore::vec2 entry = start;

    // compute grid-aligned cell bounds
    float cx = std::floor(entry.x / cs) * cs;
    float cy = std::floor(entry.y / cs) * cs;
    gore::vec2 cell_min = { cx, cy };
    gore::vec2 cell_max = { cx + cs, cy + cs };

    // if target is inside this cell, path straight to it
    if (target.x >= cell_min.x && target.x <= cell_max.x && target.y >= cell_min.y && target.y <= cell_max.y) {
            cell.start = start;
            cell.end = target;
            return { entry, target };
    }
    // a star raycast through
    struct node {
        gore::vec2 position;
        float g;
        float h;
        float f;
        int parent = -1;
        bool operator<(const node& n) const {
            return this->f > n.f;
        }
    };
    std::priority_queue<node> queue;
    std::vector<node> closed;
    auto createNode = [&](gore::vec2 pos, int parent = -1) {
        node n;
        n.position = pos;
        n.parent = parent;
        n.g = (parent > -1) ? closed[parent].g : 0;
        float dx = target.x - pos.x;
        float dy = target.y - pos.y;
        n.h = std::sqrtf(dx * dx + dy * dy);
        n.f = n.g + n.h;
        return n;
    };
    node first = createNode(entry);
    queue.emplace(first);
    const float step = 1.0f;
    while (!queue.empty()) {
        node n = queue.top();
        queue.pop();
        closed.push_back(n);
        
    }

    // cell is blocked along this direction
    return {};
}

std::vector<a_path_point> calculateAPath (SpatialHashmap* map, gore::vec2 cur_pos, gore::vec2 target, float cur_g, gore::vec2 entity_dimen) {
    std::vector<map_grid_cell> closed;
    std::priority_queue<map_grid_cell> back_log;
    map_grid_cell first = createCell(map, cur_pos, target, cur_g);
    back_log.emplace(first);
    while (!back_log.empty()) {
        map_grid_cell lowest = back_log.top();
        back_log.pop();
        closed.push_back(lowest);
        // check if cell contains the target position
        std::vector<gore::vec2> path = pathThroughCell(map, lowest, lowest.start, target);
        if (path.size() > 0 && path[1].x == target.x && path[1].y == target.y) {
            // exit
            // loop back through the closed list
            int index = lowest.parent;
            std::vector<gore::vec2> path;
            while (index != -1) {
                path.push_back(closed[index].end);
                path.push_back(closed[index].start);
                index = closed[index].parent;
            }
            break;
        } else if (path.size() == 0) {
            // move up the side we entered from to find position that works
            continue; // dead end we no want neighbors
        }
        // if not then we get grid neighbors and add to the back_log
        std::vector<std::vector<entity*>*> neighbors = map->getCellNeighbors(lowest.start.x, lowest.start.y);
        for (auto& i : neighbors) {
            map_grid_cell c = createCell(map, path[1], target, cur_g + lowest.h);
            c.parent = closed.size() - 1;
            bool add = true;
            // make sure we haven't visited before
            for (auto& j : closed) {
                if (c == j && j.start.x == c.start.x && j.end.y == c.end.y) {
                    add = false;
                    break;
                }
            }
            if (add) {
                back_log.emplace(c);
            }
        }
    }
    return {};
}


std::vector<a_path_point> findPath (SpatialHashmap* map, entity e, gore::vec2 end_pos, std::vector<a_path_point> current_path) {
    std::vector<a_path_point> target;
    // raycast till we hit an obstacle
    gore::vec2 cur_pos = raycastTo(map, e.pos, end_pos);
    if (cur_pos.x == end_pos.x && cur_pos.y == end_pos.y) {
        target.push_back({e.pos, 0, 0, 0});
        target.push_back({cur_pos, 0, 0, 0});
        return target;
    }
    // calculate A* path from hit point
    std::vector<a_path_point> a = calculateAPath(map, e.pos, end_pos, 0, e.dimen);
    // concat a to target
    for (auto& i : a) {
        target.push_back(i);
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
    std::vector<a_path_point> found = findPath(map, e, end_pos, {});
    // convert the computed path to path for entity to use
    std::vector<gore::vec2> out_path;
    for (auto& i : found) {
        out_path.push_back(i.pos);
    }
    return out_path;
}
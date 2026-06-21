#include "path.hpp"
#include "entity.hpp"
#include "game.hpp"

struct a_path_point {
    gore::vec2 pos;
    uint32_t g; // actual cost to reach node from start
    uint32_t h; // heuristic (Euclidean distance for this sqrt(dx^2+dy^2))
    uint32_t f; // total estimated cost
};

struct a_path {
    std::vector<a_path_point> path;
};

// recursive A* function
a_path calculateAPath (SpatialHashmap* map, gore::vec2 cur_pos, gore::vec2 target) {

    return {};
}


a_path findPath (SpatialHashmap* map, gore::vec2 start_pos, gore::vec2 end_pos, std::vector<a_path_point> current_path) {
    a_path target;
    // raycast till we hit an obstacle

    gore::vec2 cur_pos;
    // calculate A* path from hit point
    a_path a = calculateAPath(map, cur_pos, end_pos);
    // concat a to target
    
    return target;
}

path pathfinder::calculatePath (SpatialHashmap* map, entity e, gore::vec2 end_pos) {
    // check if target inside a structure, move to near by spot?
    {
        entity temp = { end_pos, { 2.0f, 2.0f }};
        std::vector<entity*> collisions = map->getCollisions(&temp);
        if (collisions.size() > 0) {

        }
    }
    // use A* to calculate path
    a_path found = findPath(map, e.pos, end_pos, {});
    // convert the computed path to path for entity to use
    path out_path;

    return out_path;
}
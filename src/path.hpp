#pragma once
#include "entity.hpp"
#include "game.hpp"
#include <vector>

struct path_point {
    gore::vec2 pos;
    int next_point_index;
};
struct path {
    std::vector<path_point> points;
    
};

class pathfinder {
    private:

    public:
    pathfinder () = delete;
    static path calculatePath (SpatialHashmap* map, entity e, gore::vec2 end_pos);
};
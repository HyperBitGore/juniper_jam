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
// realistic pathfinding, entity moves along wall until it can see goal point other, if wall ends, then it will follow next wall it sees?
//  - entity save path it walked, then walks back if it encounters dead-end
//  - constantly raycasts in every direction and goal point direction, to look for new path
class pathfinder {
    private:

    public:
    pathfinder () = delete;
    static std::vector<gore::vec2> calculatePath (SpatialHashmap* map, entity e, gore::vec2 end_pos);
};
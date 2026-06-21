#pragma once
#include "g_engine/util/shader.hpp"

class entity {
    public:
        gore::vec2 pos;
        gore::vec2 dimen;
        int32_t img_id;
        gore::vec2 target = { -1.0f, -1.0f };
        entity (gore::vec2 pos, gore::vec2 dimen, int32_t imd_id = -1) {
            this->pos = pos;
            this->dimen = dimen;
            this->img_id = imd_id;
        }
        bool isColliding(const entity& e) const {
            return (pos.x + dimen.x > e.pos.x &&         // this.right > other.left
                    pos.x < e.pos.x + e.dimen.x &&       // this.left < other.right
                    pos.y + dimen.y > e.pos.y &&         // this.bottom > other.top
                    pos.y < e.pos.y + e.dimen.y);        // this.top < other.bottom
        }
};
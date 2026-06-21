#pragma once
#include "g_engine/util/shader.hpp"

enum class entity_type { STRUCTURE, UNIT };

class entity {
    public:
        gore::vec2 pos;
        gore::vec2 dimen;
        int32_t img_id;
        gore::vec2 target = { -1.0f, -1.0f };
        entity_type type;
        entity (gore::vec2 pos, gore::vec2 dimen, int32_t imd_id = -1, entity_type type = entity_type::UNIT) {
            this->pos = pos;
            this->dimen = dimen;
            this->img_id = imd_id;
            this->type = type;
        }
        bool isColliding(const entity& e) const {
            return (pos.x + dimen.x > e.pos.x &&         // this.right > other.left
                    pos.x < e.pos.x + e.dimen.x &&       // this.left < other.right
                    pos.y + dimen.y > e.pos.y &&         // this.bottom > other.top
                    pos.y < e.pos.y + e.dimen.y);        // this.top < other.bottom
        }
};
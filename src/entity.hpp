#pragma once
#include "g_engine/util/shader.hpp"

enum class entity_type { STRUCTURE, UNIT, BUTTON };
enum class action_type { ATTACK, COLLECT};
// for ai
struct action {
    gore::vec2 target;
    action_type type;
};

class entity {
    public:
        gore::vec2 pos;
        gore::vec2 dimen;
        int32_t img_id;
        entity_type type;
        std::vector<gore::vec2> path;
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
        uint8_t typeToUint8t () {
            switch (type) {
            case entity_type::STRUCTURE:
                return 0;
            case entity_type::UNIT:
                return 1;
            case entity_type::BUTTON:
                return 2;
            }
            return 255;
        }
        static entity_type uin8tToType (uint8_t type) {
            switch (type) {
                case 0:
                return entity_type::STRUCTURE;
                case 1:
                return entity_type::UNIT;
                case 2:
                return entity_type::BUTTON;
            }
            return entity_type::UNIT;
        }
};
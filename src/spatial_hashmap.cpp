#include "spatial_hashmap.hpp"
#include <cstdint>
#include <vector>

SpatialHashmap::SpatialHashmap () {
    cell_size = 25;
    grid_width = 5000;
    bucket_length = (grid_width * grid_width) / cell_size;
    buckets.reserve(bucket_length);
    for (size_t i = 0; i < bucket_length; i++) {
        buckets.push_back({});
    }
}

SpatialHashmap::SpatialHashmap (uint32_t cell_size, uint32_t grid_width) {
    this->cell_size = cell_size;
    this->grid_width = grid_width;
    bucket_length = (grid_width * grid_width) / cell_size;
    buckets.reserve(bucket_length);
    for (size_t i = 0; i < bucket_length; i++) {
        buckets.push_back({});
    }
}
uint32_t SpatialHashmap::hash(float x, float y) {
    uint32_t cell_x = (uint32_t)std::floor(x / cell_size);
    uint32_t cell_y = (uint32_t)std::floor(y / cell_size);
    return cell_x + cell_y * (grid_width / cell_size);
}

// since four corners isn't enough we are going to make a general algorithm for getting our guy into every correct hash
inline std::vector<uint32_t> SpatialHashmap::calculateHashes (entity* e) {
    // loop across entity dimensions, stopping at every hash boundary
    if (e->pos.x < 0 || e->pos.y < 0 || e->pos.x + e->dimen.x > grid_width || e->pos.y + e->dimen.y > grid_width) {
        return {};
    }
    std::vector<uint32_t> hashes;
    for (uint32_t y = (uint32_t)std::floor(e->pos.y); y <= (uint32_t)std::floor(e->pos.y + e->dimen.y);) {
        for (uint32_t x = (uint32_t)std::floor(e->pos.x); x <= (uint32_t)std::floor(e->pos.x + e->dimen.x);) {
            hashes.push_back(hash(x, y));
            uint32_t remainder = x % cell_size;
            // std::cout << "x: " << x << "\n";
            x += (remainder > 0) ? (cell_size - remainder) : cell_size;
        }
        uint32_t remainder = (y % cell_size);
        // std::cout << "y: " << y << "\n";
        y += (remainder > 0) ? (cell_size - remainder) : cell_size;
    }
    return hashes;
}

// insert after updating the entity position or we are fucked!
bool SpatialHashmap::insert(entity* e) {
    std::vector<uint32_t> hashes = calculateHashes(e);
    if (hashes.size() > 0) {
        for (auto& i : hashes) {
            buckets[i].push_back(e);
        }
        return true;
    }

    return false;
}

int32_t findentityIndex (std::vector<entity*> bucket, entity* e) {
    for (size_t i = 0; i < bucket.size(); i++) {
        if (e == bucket[i]) {
            return i;
        }
    }
    return -1;
}
// remove before updating entity position or else we are fucked!
void SpatialHashmap::remove(entity* e) {
    std::vector<uint32_t> hashes = calculateHashes(e);
    for (auto& i : hashes) {
        int32_t h_index = findentityIndex(buckets[i], e);
        if (h_index > -1) {
            buckets[i].erase(buckets[i].begin() + h_index);
        }
    }
}

inline entity* checkBucketCollision (std::vector<entity*> bucket, entity* e) {
    for (auto& i : bucket) {
        if (i != e && e->isColliding(*i)) {
            return i;
        }
    }
    return nullptr;
}

entity* SpatialHashmap::checkCollision (entity* e) {
    std::vector<uint32_t> hashes = calculateHashes(e);
    for (auto& i : hashes) {
        entity* b = checkBucketCollision(buckets[i], e);
        if (b && b != e) {
            return b;
        }
    }
    return nullptr;
}

std::vector<entity*> SpatialHashmap::getCollisions (entity* a) {
    std::vector<entity*> collisions;
    std::vector<uint32_t> hashes = calculateHashes(a);
    for (auto& i : hashes) {
        entity* b = checkBucketCollision(buckets[i], a);
        if (b && b != a) {
            collisions.push_back(b);
        }
    }
    return collisions;
}

std::vector<entity*>* SpatialHashmap::getBucket (float x, float y) {
    uint32_t h = hash(x, y);
    return &buckets[h];
}

std::vector<std::vector<entity*>*> SpatialHashmap::getCellNeighbors (float x, float y) {
    float cs = static_cast<float>(cell_size);
    float cx = std::floor(x / cs) * cs;
    float cy = std::floor(y / cs) * cs;

    std::vector<std::vector<entity*>*> out;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            float nx = cx + dx * cs;
            float ny = cy + dy * cs;
            if (nx < 0 || ny < 0 || nx >= grid_width || ny >= grid_width) continue;
            out.push_back(&buckets[hash(nx, ny)]);
        }
    }
    return out;
}

std::vector<gore::vec2> SpatialHashmap::getCellNeighborPositions (float x, float y) {
    float cs = static_cast<float>(cell_size);
    float cx = std::floor(x / cs) * cs;
    float cy = std::floor(y / cs) * cs;
    std::vector<gore::vec2> out;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            float nx = cx + dx * cs;
            float ny = cy + dy * cs;
            if (nx < 0 || ny < 0 || nx >= grid_width || ny >= grid_width) continue;
            out.push_back({nx, ny});
        }
    }
    return out;
}
std::vector<gore::vec2> SpatialHashmap::getCellNeighborsCorners (float x, float y) {
    float cs = static_cast<float>(cell_size);
    float cx = std::floor(x / cs) * cs;
    float cy = std::floor(y / cs) * cs;
    const float edge = cs - 1.0f;
    std::vector<gore::vec2> out;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;
            float nx = cx + dx * cs;
            float ny = cy + dy * cs;
            if (nx < 0 || ny < 0 || nx >= grid_width || ny >= grid_width) continue;
            // get the corners of this cell
            out.push_back({nx, ny});
            out.push_back({nx + edge, ny});
            out.push_back({nx + edge, ny + edge});
            out.push_back({nx, ny + edge});
            // the middle of the sides
            out.push_back({nx + (cs / 2.0f), ny});
            out.push_back({nx, ny + (cs / 2.0f)});
            out.push_back({nx + edge, ny + (cs / 2.0f)});
            out.push_back({nx + (cs / 2.0f), ny + edge});
        }
    }
    return out;
}

inline bool floatEq (float a, float b, float tolerance = 1e-5f) {
    return std::abs(a - b) <= tolerance;
}

entity* SpatialHashmap::raycastTo (gore::vec2 start, gore::vec2 target, float width) {
    entity e(start, {width, width});
    // loop along angle towards target and move until we hit a blocking object
    while (!floatEq(e.pos.x, target.x, 1e-2f) || !floatEq(e.pos.y, target.y, 1e-2f)) {
        entity* collision = checkCollision(&e);
        if (collision != nullptr) {
            return collision;
        }
        float dx = target.x - e.pos.x;
        float dy = target.y - e.pos.y;
        if (std::sqrtf(dx * dx + dy * dy) <= width) {
            e.pos = target;
            break;
        }
        float angle = std::atan2f(dy, dx);
        float angle_cos = std::cosf(angle);
        float angle_sin = std::sinf(angle);
        gore::vec2 change = {angle_cos * width, angle_sin * width };
        e.pos += change;
    }
    return nullptr;
}

std::vector<entity*> SpatialHashmap::raycastToCollisions (gore::vec2 start, gore::vec2 target, float width) {
    entity e(start, {width, width});
    // loop along angle towards target and move until we hit a blocking object
    while (!floatEq(e.pos.x, target.x, 1e-2f) || !floatEq(e.pos.y, target.y, 1e-2f)) {
        std::vector<entity*> collisions = getCollisions(&e);
        if (collisions.size() > 0) {
            return collisions;
        }
        float dx = target.x - e.pos.x;
        float dy = target.y - e.pos.y;
        if (std::sqrtf(dx * dx + dy * dy) <= width) {
            e.pos = target;
            break;
        }
        float angle = std::atan2f(dy, dx);
        float angle_cos = std::cosf(angle);
        float angle_sin = std::sinf(angle);
        gore::vec2 change = {angle_cos * width, angle_sin * width };
        e.pos += change;
    }
    return {};
}

std::vector<entity*> SpatialHashmap::scanAroundEntity (entity* e, float distance) {
    entity scan = { {e->pos.x - distance, e->pos.y - distance}, { e->dimen.x + (distance*2), e->dimen.y + (distance*2)}};
    std::vector<entity*> out = getCollisions(&scan);
    return out;
}
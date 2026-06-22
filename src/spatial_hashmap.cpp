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
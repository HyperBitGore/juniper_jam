#include "entity.hpp"
#include <random>
#include <chrono>

class SpatialHashmap {
private:
    std::vector<std::vector<entity*>> buckets;
    size_t bucket_length = 0;
    uint32_t grid_width;
    uint32_t cell_size;
    uint32_t hash (float x, float y);
    std::vector<uint32_t> calculateHashes (entity* e);
public:
    SpatialHashmap();
    SpatialHashmap (uint32_t cell_size, uint32_t grid_width);
    // copy
    SpatialHashmap (const SpatialHashmap& map) {
        bucket_length = map.bucket_length;
        grid_width = map.grid_width;
        cell_size = map.cell_size;
        buckets.reserve(bucket_length);
        for (size_t i = 0; i < bucket_length; i++) {
            buckets.push_back(map.buckets[i]);
        }
    }
    bool insert(entity* e);
    void remove(entity* e);
    // raycasts towards a point, returns first collision or nullptr if reached
    entity* raycastTo (gore::vec2 start, gore::vec2 target, float width);
    entity* checkCollision (entity* e);
    std::vector<entity*> getCollisions (entity* a);
    uint32_t getGridWidth () {
        return grid_width;
    }
    uint32_t getCellSize () {
        return cell_size;
    }
    std::vector<entity*>* getBucket (float x, float y);
    std::vector<std::vector<entity*>*> getCellNeighbors (float x, float y);
    std::vector<gore::vec2> getCellNeighborPositions (float x, float y);


    void hashTest () {
        std::cout << "hash test\n";
        std::cout << "0, 0: " << hash(0, 0) << "\n";
        std::cout << "50, 0: " << hash(50, 0) << "\n";
        std::cout << "49, 0: " << hash(49, 0) << "\n";
        std::cout << "0, 50: " << hash(0, 50) << "\n";
        std::cout << "0, 49: " << hash(0, 49) << "\n";
    }
    static void mapBenchmark () {
        std::vector<std::unique_ptr<entity>> entities;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1000.0f);
        for (int i = 0; i < 10000; i++) {
            gore::vec2 pos = {dis(gen), dis(gen)};
            gore::vec2 dimen = {50.0f, 50.0f};
            std::unique_ptr<entity> e = std::make_unique<entity>(pos, dimen, -1);
            entities.push_back(std::move(e));
        }
        SpatialHashmap map1(50, 5000);
        auto start = std::chrono::high_resolution_clock::now();
        for (size_t i = 0; i < 10000; i++) {
            map1.insert(entities[i].get());
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "Inserted " << entities.size() << " entities in " 
              << duration.count() << " microseconds\n";
    }
};
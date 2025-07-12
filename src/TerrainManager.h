#pragma once

#include <vector>
#include "Terrain.h"
#include "Scene.h"

class TerrainManager {
public:
    float GetHeightAt(float x, float z) const;

    TerrainManager(Device* device, VkCommandPool commandPool, Scene* scene, VkImage texture, float tileSize, int resolution, int gridWidth, int gridHeight);
    ~TerrainManager();

private:
    std::vector<Terrain*> terrainTiles; //a list of pointers to all the Terrain tiles we generated

    float tileSize; //how big each square terrain tile
    int resolution; // how many subdivisions per tile
};

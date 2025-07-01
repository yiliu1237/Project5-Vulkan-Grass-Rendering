#include "TerrainManager.h"
#include "Image.h"

float TerrainManager::GetHeightAt(float x, float z) const {
    for (Terrain* tile : terrainTiles) {
        if (tile->Contains(x, z)) {
            return tile->GetHeightAt(x, z);
        }
    }
    return 0.0f;
}


TerrainManager::TerrainManager(Device* device, VkCommandPool commandPool, Scene* scene,
    VkImage texture, float tileSize, int resolution, int gridWidth, int gridHeight)
    : tileSize(tileSize), resolution(resolution)
{
    float startX = -0.5f * gridWidth * tileSize;
    float startZ = -0.5f * gridHeight * tileSize;

    for (int j = 0; j < gridHeight; ++j) {
        for (int i = 0; i < gridWidth; ++i) {
            float worldX = startX + i * tileSize;
            float worldZ = startZ + j * tileSize;

            Terrain * tile = new Terrain(device, commandPool, tileSize, resolution, worldX, worldZ);
            tile->SetTexture(texture); //Important!

            scene->AddModel(tile);
            terrainTiles.push_back(tile);

            // Add blades to this tile
            Blades* tileBlades = new Blades(device, commandPool, tileSize, worldX, worldZ);
            if(i == 2 && j == 2) scene->AddBlades(tileBlades);

        }
    }
}

TerrainManager::~TerrainManager() {
    for (Terrain* tile : terrainTiles) {
        delete tile;
    }
    terrainTiles.clear();
}

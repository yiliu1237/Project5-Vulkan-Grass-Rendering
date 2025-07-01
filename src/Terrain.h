#pragma once

#include "Model.h"
#include "NoiseUtils.h"
#include "Vertex.h"

class Terrain : public Model {
private: 
    float terrainSize;
    int terrainResolution;
    std::vector<Vertex> vertices;

    float offsetX, offsetZ;

public:
    Terrain(Device* device, VkCommandPool commandPool, float size, int resolution, float offsetX = 0.0f, float offsetZ = 0.0f);
    float GetHeightAt(float x, float z) const;

    bool Contains(float x, float z) const;

    glm::vec2 GetOffset() const { return glm::vec2(offsetX, offsetZ); }
};

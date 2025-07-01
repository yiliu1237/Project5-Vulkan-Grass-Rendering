#include "Terrain.h"
#include "BufferUtils.h"


float Terrain::GetHeightAt(float x, float z) const {
    float halfSize = terrainSize / 2.0f;
    float gridSpacing = terrainSize / terrainResolution;

    // Clamp to terrain bounds
    if (x < -halfSize || x > halfSize || z < -halfSize || z > halfSize) {
        return 0.0f; // Or some default/fallback value
    }

    // Transform world x/z to grid space
    float localX = (x + halfSize) / gridSpacing;
    float localZ = (z + halfSize) / gridSpacing;

    int x0 = static_cast<int>(floor(localX));
    int z0 = static_cast<int>(floor(localZ));
    int x1 = x0 + 1;
    int z1 = z0 + 1;

    // Clamp indices to avoid out-of-bounds
    x0 = glm::clamp(x0, 0, terrainResolution);
    x1 = glm::clamp(x1, 0, terrainResolution);
    z0 = glm::clamp(z0, 0, terrainResolution);
    z1 = glm::clamp(z1, 0, terrainResolution);

    // Compute bilinear weights
    float tx = localX - x0;
    float tz = localZ - z0;

    // Get indices into the flattened vertex array
    int index00 = z0 * (terrainResolution + 1) + x0;
    int index10 = z0 * (terrainResolution + 1) + x1;
    int index01 = z1 * (terrainResolution + 1) + x0;
    int index11 = z1 * (terrainResolution + 1) + x1;

    float h00 = vertices[index00].pos.y;
    float h10 = vertices[index10].pos.y;
    float h01 = vertices[index01].pos.y;
    float h11 = vertices[index11].pos.y;

    // Bilinear interpolation
    float h0 = glm::mix(h00, h10, tx);
    float h1 = glm::mix(h01, h11, tx);
    float h = glm::mix(h0, h1, tz);

    return h;
}


bool Terrain::Contains(float x, float z) const {
    float halfSize = terrainSize / 2.0f;

    return (
        x >= offsetX - halfSize && x <= offsetX + halfSize &&
        z >= offsetZ - halfSize && z <= offsetZ + halfSize
        );
}




// Terrain.cpp
Terrain::Terrain(Device* device, VkCommandPool commandPool, float size, int resolution, float offsetX, float offsetZ)
    : Model(device, commandPool, {}, {}), offsetX(offsetX), offsetZ(offsetZ)
{
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    float halfSize = size / 2.0f;
    float step = size / resolution;

    for (int z = 0; z <= resolution; z++) {
        for (int x = 0; x <= resolution; x++) {
            float xpos = -halfSize + x * step + offsetX;
            float zpos = -halfSize + z * step + offsetZ;
            //float ypos = NoiseUtils::Noise(xpos * 0.5f, zpos * 0.5f) * 2.0f;
            float ypos = 0.0;

            vertices.push_back({
                glm::vec3(xpos, ypos, zpos),
                glm::vec3(0, 1, 0),
                glm::vec2(x / (float)resolution, z / (float)resolution)
                });
        }
    }

    for (int z = 0; z < resolution; z++) {
        for (int x = 0; x < resolution; x++) {
            uint32_t topLeft = z * (resolution + 1) + x;
            uint32_t topRight = topLeft + 1;
            uint32_t bottomLeft = (z + 1) * (resolution + 1) + x;
            uint32_t bottomRight = bottomLeft + 1;

            indices.insert(indices.end(), {
                topLeft, bottomLeft, topRight,
                topRight, bottomLeft, bottomRight
                });
        }
    }

    this->vertices = vertices;
    this->indices = indices;

    BufferUtils::CreateVertexIndexBuffers(device, commandPool, vertices, indices,
        vertexBuffer, vertexBufferMemory,
        indexBuffer, indexBufferMemory);
}

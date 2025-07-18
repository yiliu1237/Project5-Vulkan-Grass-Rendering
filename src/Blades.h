#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <array>
#include "Model.h"
#include "NoiseUtils.h"

constexpr static unsigned int NUM_BLADES = 1 << 15;
constexpr static float MIN_HEIGHT = 1.3f;
constexpr static float MAX_HEIGHT = 2.5f;
constexpr static float MIN_WIDTH = 0.1f;
constexpr static float MAX_WIDTH = 0.14f;
constexpr static float MIN_BEND = 7.0f;
constexpr static float MAX_BEND = 13.0f;

struct Blade {
    // Position and direction
    glm::vec4 v0;
    // Bezier point and height
    glm::vec4 v1;
    // Physical model guide and width
    glm::vec4 v2;
    // Up vector and stiffness coefficient
    glm::vec4 up;

    int bladeType = 2;     // blade shape type 
    int pad0;          // Padding to maintain 16-byte alignment
    int pad1;
    int pad2;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 1;
        bindingDescription.stride = sizeof(Blade);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions = {};

        // v0
        attributeDescriptions[0].binding = 1;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Blade, v0);

        // v1
        attributeDescriptions[1].binding = 1;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Blade, v1);

        // v2
        attributeDescriptions[2].binding = 1;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Blade, v2);

        // up
        attributeDescriptions[3].binding = 1;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Blade, up);


        // bladeType
        attributeDescriptions[4].binding = 1;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32_SINT;  // 1 int
        attributeDescriptions[4].offset = offsetof(Blade, bladeType);


        return attributeDescriptions;
    }
};

struct BladeDrawIndirect {
    uint32_t vertexCount;
    uint32_t instanceCount;
    uint32_t firstVertex;
    uint32_t firstInstance;
};

struct TransformationInfo {
    glm::vec4 transform;
};

class Blades : public Model {
private:
    VkBuffer bladesBuffer;
    VkBuffer culledBladesBuffer;
    VkBuffer numBladesBuffer;
    VkBuffer transformBuffer;

    VkDeviceMemory bladesBufferMemory;
    VkDeviceMemory culledBladesBufferMemory;
    VkDeviceMemory numBladesBufferMemory;
    VkDeviceMemory transBufferMemory;

    void* data;
    TransformationInfo transformData;

public:
    Blades(Device* device, VkCommandPool commandPool, float tileSize, float tileOffsetX, float tileOffsetZ);
    VkBuffer GetBladesBuffer() const;
    VkBuffer GetCulledBladesBuffer() const;
    VkBuffer GetNumBladesBuffer() const;

    VkBuffer GetTransformationBuffer() const;
    TransformationInfo GetTransformationData() const;
    void UpdateTransformation(const glm::vec4 transformation);
    ~Blades();
};

#include "BufferUtils.h"
#include "Instance.h"

#include <cstring>

void BufferUtils::CreateBuffer(Device* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // Create buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device->GetVkDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vertex buffer");
    }

    // Query buffer's memory requirements
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device->GetVkDevice(), buffer, &memRequirements);

    // Allocate memory in device
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = device->GetInstance()->GetMemoryTypeIndex(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device->GetVkDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
      throw std::runtime_error("Failed to allocate vertex buffer");
    }

    // Associate allocated memory with vertex buffer
    vkBindBufferMemory(device->GetVkDevice(), buffer, bufferMemory, 0);
}

void BufferUtils::CopyBuffer(Device* device, VkCommandPool commandPool, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->GetVkDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion = {};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->GetQueue(QueueFlags::Graphics), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->GetQueue(QueueFlags::Graphics));
    vkFreeCommandBuffers(device->GetVkDevice(), commandPool, 1, &commandBuffer);
}

void BufferUtils::CreateBufferFromData(Device* device, VkCommandPool commandPool, void* bufferData, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    // Create the staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    VkBufferUsageFlags stagingUsage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    VkMemoryPropertyFlags stagingProperties = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    BufferUtils::CreateBuffer(device, bufferSize, stagingUsage, stagingProperties, stagingBuffer, stagingBufferMemory);

    // Fill the staging buffer
    void *data;
    vkMapMemory(device->GetVkDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, bufferData, static_cast<size_t>(bufferSize));
    vkUnmapMemory(device->GetVkDevice(), stagingBufferMemory);

    // Create the buffer
    VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | bufferUsage;
    VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    BufferUtils::CreateBuffer(device, bufferSize, usage, flags, buffer, bufferMemory);

    // Copy data from staging to buffer
    BufferUtils::CopyBuffer(device, commandPool, stagingBuffer, buffer, bufferSize);

    // No need for the staging buffer anymore
    vkDestroyBuffer(device->GetVkDevice(), stagingBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), stagingBufferMemory, nullptr);
}

void BufferUtils::CreateVertexIndexBuffers(Device* device, VkCommandPool commandPool,
    const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
    VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory,
    VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory)
{
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();

    // Create staging buffers
    VkBuffer stagingVertexBuffer;
    VkDeviceMemory stagingVertexBufferMemory;
    CreateBuffer(device, vertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingVertexBuffer, stagingVertexBufferMemory);

    void* vertexData;
    vkMapMemory(device->GetVkDevice(), stagingVertexBufferMemory, 0, vertexBufferSize, 0, &vertexData);
    memcpy(vertexData, vertices.data(), (size_t)vertexBufferSize);
    vkUnmapMemory(device->GetVkDevice(), stagingVertexBufferMemory);

    VkBuffer stagingIndexBuffer;
    VkDeviceMemory stagingIndexBufferMemory;
    CreateBuffer(device, indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingIndexBuffer, stagingIndexBufferMemory);

    void* indexData;
    vkMapMemory(device->GetVkDevice(), stagingIndexBufferMemory, 0, indexBufferSize, 0, &indexData);
    memcpy(indexData, indices.data(), (size_t)indexBufferSize);
    vkUnmapMemory(device->GetVkDevice(), stagingIndexBufferMemory);

    // Create device-local buffers
    CreateBuffer(device, vertexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        vertexBuffer, vertexBufferMemory);

    CreateBuffer(device, indexBufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        indexBuffer, indexBufferMemory);

    // Copy from staging to device-local buffers
    CopyBuffer(device, commandPool, stagingVertexBuffer, vertexBuffer, vertexBufferSize);
    CopyBuffer(device, commandPool, stagingIndexBuffer, indexBuffer, indexBufferSize);

    // Clean up staging buffers
    vkDestroyBuffer(device->GetVkDevice(), stagingVertexBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), stagingVertexBufferMemory, nullptr);
    vkDestroyBuffer(device->GetVkDevice(), stagingIndexBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), stagingIndexBufferMemory, nullptr);
}


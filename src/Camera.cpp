#include <iostream>

#define GLM_FORCE_RADIANS
// Use Vulkan depth range of 0.0 to 1.0 instead of OpenGL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"
#include "BufferUtils.h"

Camera::Camera(Device* device, float aspectRatio) : device(device) {
    r = 10.0f;
    theta = 0.0f;
    phi = 0.0f;
    cameraBufferObject.viewMatrix = glm::lookAt(glm::vec3(0.0f, 1.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    cameraBufferObject.projectionMatrix = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);
    cameraBufferObject.projectionMatrix[1][1] *= -1; // y-coordinate is flipped

    BufferUtils::CreateBuffer(device, sizeof(CameraBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);
    vkMapMemory(device->GetVkDevice(), bufferMemory, 0, sizeof(CameraBufferObject), 0, &mappedData);
    memcpy(mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}


void Camera::MoveForward(float amount) {
    glm::vec3 forward = glm::normalize(lookAt - position);
    position += forward * amount;
    lookAt += forward * amount;
    cameraBufferObject.viewMatrix = glm::lookAt(position, lookAt, up);
    memcpy(mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

void Camera::MoveRight(float amount) {
    glm::vec3 right = glm::normalize(glm::cross(lookAt - position, up));
    position += right * amount;
    lookAt += right * amount;
    cameraBufferObject.viewMatrix = glm::lookAt(position, lookAt, up);
    memcpy(mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

void Camera::MoveUp(float amount) {
    position += up * amount;
    lookAt += up * amount;
    cameraBufferObject.viewMatrix = glm::lookAt(position, lookAt, up);
    memcpy(mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}



VkBuffer Camera::GetBuffer() const {
    return buffer;
}

void Camera::UpdateOrbit(float deltaX, float deltaY, float deltaZ) {
    theta += deltaX;
    phi += deltaY;
    r = glm::clamp(r - deltaZ, 1.0f, 50.0f);

    float radTheta = glm::radians(theta);
    float radPhi = glm::radians(phi);

    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), radTheta, glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), radPhi, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 finalTransform = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f)) * rotation * glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 1.0f, r));

    cameraBufferObject.viewMatrix = glm::inverse(finalTransform);


    // Extract position from final transform
    glm::vec4 camPos = finalTransform * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    position = glm::vec3(camPos);
    lookAt = glm::vec3(0.0f, 1.0f, 0.0f);  // target of orbit

    cameraBufferObject.viewMatrix = glm::lookAt(position, lookAt, up);
    memcpy(mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}


void Camera::UpdateLook(float deltaX, float deltaY, float deltaZ) {
    float sensitivity = 0.1f;
    yaw += deltaX * sensitivity;
    pitch += deltaY * sensitivity;

    // Clamp pitch to avoid flipping
    pitch = glm::clamp(pitch, -89.0f, 89.0f);

    // Calculate the new front direction from yaw and pitch
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction = glm::normalize(direction);

    // Zoom by moving position forward/backward along view direction
    position += direction * deltaZ;

    // Update lookAt based on new direction
    lookAt = position + direction;

    cameraBufferObject.viewMatrix = glm::lookAt(position, lookAt, up);
    memcpy(mappedData, &cameraBufferObject, sizeof(CameraBufferObject));
}

glm::vec3 Camera::GetPosition() const {
    glm::mat4 inverseView = glm::inverse(cameraBufferObject.viewMatrix);
    return glm::vec3(inverseView[3]);
}


glm::mat4 Camera::GetProjectionMatrix() const {
    return cameraBufferObject.projectionMatrix;
}


glm::mat4 Camera::GetViewMatrix() const {
    return cameraBufferObject.viewMatrix;
}


Camera::~Camera() {
  vkUnmapMemory(device->GetVkDevice(), bufferMemory);
  vkDestroyBuffer(device->GetVkDevice(), buffer, nullptr);
  vkFreeMemory(device->GetVkDevice(), bufferMemory, nullptr);
}

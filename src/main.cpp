#include <vulkan/vulkan.h>
#include <iostream>
#include "Instance.h"
#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Image.h"
#include "Terrain.h"
#include "TerrainManager.h"

Device* device;
SwapChain* swapChain;
Renderer* renderer;
Camera* camera;
//Blades* blades;
//Terrain* terrain;
TerrainManager* terrainManager;
Scene* scene;


float collisionRadius = 0.5f;



namespace {
    void resizeCallback(GLFWwindow* window, int width, int height) {
        if (width == 0 || height == 0) return;

        vkDeviceWaitIdle(device->GetVkDevice());
        swapChain->Recreate(width, height);
        renderer->RecreateFrameResources();
    }

    bool leftMouseDown = false;
    bool rightMouseDown = false;
    bool middleMouseDown = false;
    double previousX = 0.0;
    double previousY = 0.0;



    void mouseDownCallback(GLFWwindow* window, int button, int action, int mods) {
        // Track mouse button state using references
        auto getButtonStateRef = [](int btn) -> bool& {
            static bool dummy = false;
            switch (btn) {
            case GLFW_MOUSE_BUTTON_LEFT:   return leftMouseDown;
            case GLFW_MOUSE_BUTTON_RIGHT:  return rightMouseDown;
            case GLFW_MOUSE_BUTTON_MIDDLE: return middleMouseDown;
            default:                        return dummy; // ignore other buttons
            }
           };

        bool& buttonState = getButtonStateRef(button);

        if (action == GLFW_PRESS) {
            buttonState = true;
            glfwGetCursorPos(window, &previousX, &previousY);
        }
        else if (action == GLFW_RELEASE) {
            buttonState = false;
        }
    }




    void mouseMoveCallback(GLFWwindow* window, double xPos, double yPos) {
        constexpr double sensitivity = 0.5;

        // Always store deltas relative to previous cursor position
        float deltaX = static_cast<float>((previousX - xPos) * sensitivity);
        float deltaY = static_cast<float>((previousY - yPos) * sensitivity);

        // If left mouse is down, rotate the camera
        if (leftMouseDown) {
            camera->UpdateLook(deltaX, deltaY, 0.0f);
        }

        // If right mouse is down, perform terrain raycast and update blade transformations
        else if (rightMouseDown) {
            // Convert cursor to Normalized Device Coordinates (NDC)
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            float xNDC = (2.0f * static_cast<float>(xPos)) / width - 1.0f;
            float yNDC = (2.0f * static_cast<float>(yPos)) / height - 1.0f;

            // Ray in clip space
            glm::vec4 rayClip(xNDC, yNDC, -1.0f, 1.0f);

            // Transform to eye space
            glm::vec4 rayEye = glm::inverse(camera->GetProjectionMatrix()) * rayClip;
            rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

            // Transform to world space
            glm::vec3 rayWorld = glm::normalize(glm::vec3(glm::inverse(camera->GetViewMatrix()) * rayEye));

            glm::vec3 rayOrigin = camera->GetPosition();
            glm::vec3 rayDirection = rayWorld;

            constexpr float step = 0.1f;
            constexpr float maxDistance = 10000.0f;

            // Raymarch along the direction until we hit the terrain
            for (float t = 0.0f; t < maxDistance; t += step) {
                glm::vec3 point = rayOrigin + rayDirection * t;
                float terrainHeight = terrainManager->GetHeightAt(point.x, point.z);

                if (point.y <= terrainHeight) {
                    glm::vec4 transform = glm::vec4(point.x, terrainHeight, point.z, collisionRadius * 2.5f);
                    for (Blades* b : scene->GetBlades()) {
                        b->UpdateTransformation(transform);
                    }
                    break;
                }
            }
        }

        // Update the previous cursor position (for both modes)
        previousX = xPos;
        previousY = yPos;
    }



    void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
        constexpr float scrollSensitivity = 0.1f;
        constexpr float zoomSensitivity = 0.5f;

        if (middleMouseDown) {
            // Adjust collision size based on scroll input
            collisionRadius += static_cast<float>(yOffset * scrollSensitivity);

            // Update transformation for all blade groups using new collision size
            const float newScale = collisionRadius * 2.5f;
            for (Blades* bladeGroup : scene->GetBlades()) {
                glm::vec4 prevTransform = bladeGroup->GetTransformationData().transform;

                bladeGroup->UpdateTransformation(glm::vec4(
                    prevTransform.x,
                    prevTransform.y,
                    prevTransform.z,
                    newScale
                ));
            }
        }
        else {
            // Zoom camera in/out using scroll
            float deltaZ = static_cast<float>(yOffset * zoomSensitivity);
            camera->UpdateLook(0.0f, 0.0f, deltaZ);
        }
    }




    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            switch (key) {
            case GLFW_KEY_W:
                camera->MoveForward(1.0f);
                break;
            case GLFW_KEY_S:
                camera->MoveForward(-1.0f);
                break;
            case GLFW_KEY_A:
                camera->MoveRight(-1.0f);
                break;
            case GLFW_KEY_D:
                camera->MoveRight(1.0f);
                break;
            case GLFW_KEY_Q:
                camera->MoveUp(-1.0f);
                break;
            case GLFW_KEY_E:
                camera->MoveUp(1.0f);
                break;
            }
        }
    }


}

int main() {

    static constexpr char* applicationName = "Vulkan Grass Rendering";
    InitializeWindow(640, 480, applicationName);

    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    Instance* instance = new Instance(applicationName, glfwExtensionCount, glfwExtensions);

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance->GetVkInstance(), GetGLFWWindow(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }

    instance->PickPhysicalDevice({ VK_KHR_SWAPCHAIN_EXTENSION_NAME }, QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, surface);

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.tessellationShader = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    device = instance->CreateDevice(QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, deviceFeatures);

    swapChain = device->CreateSwapChain(surface, 5);

    camera = new Camera(device, 640.f / 480.f);

    VkCommandPoolCreateInfo transferPoolInfo = {};
    transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transferPoolInfo.queueFamilyIndex = device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Transfer];
    transferPoolInfo.flags = 0;

    VkCommandPool transferCommandPool;
    if (vkCreateCommandPool(device->GetVkDevice(), &transferPoolInfo, nullptr, &transferCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    VkImage grassImage;
    VkDeviceMemory grassImageMemory;
    Image::FromFile(device,
        transferCommandPool,
        "images/grass.jpg",
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        grassImage,
        grassImageMemory
    );

    
    //float planeDim = 15.f;
    //float halfWidth = planeDim * 0.5f;
    //Model* plane = new Model(device, transferCommandPool,
    //    {
    //        { { -halfWidth, 0.0f, halfWidth }, { 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
    //        { { halfWidth, 0.0f, halfWidth }, { 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
    //        { { halfWidth, 0.0f, -halfWidth }, { 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
    //        { { -halfWidth, 0.0f, -halfWidth }, { 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } }
    //    },
    //    { 0, 1, 2, 2, 3, 0 }
    //);
    //plane->SetTexture(grassImage);

    scene = new Scene(device);

  

    //terrain = new Terrain(device, transferCommandPool, planeDim, 100);
    //terrain->SetTexture(grassImage); //Important!

    int gridWidth = 3;
    int gridHeight = 3;
    float tileSize = 15.0f;
    int resolution = 100;

    terrainManager = new TerrainManager(device, transferCommandPool, scene, grassImage, tileSize, resolution, gridWidth, gridHeight);


    for (auto* b : scene->GetBlades()) {
        std::cout << b->GetNumBladesBuffer() << std::endl;
    }


    renderer = new Renderer(device, swapChain, scene, camera);

    glfwSetWindowSizeCallback(GetGLFWWindow(), resizeCallback);
    glfwSetMouseButtonCallback(GetGLFWWindow(), mouseDownCallback);
    glfwSetCursorPosCallback(GetGLFWWindow(), mouseMoveCallback);
    glfwSetScrollCallback(GetGLFWWindow(), scrollCallback);
    glfwSetKeyCallback(GetGLFWWindow(), keyCallback);


    while (!ShouldQuit()) {
        glfwPollEvents();
        scene->UpdateTime();

        //terrainManager->Update(camera->GetPosition());


        // FPS logging
        static float fpsTimer = 0.0f;
        fpsTimer += scene->GetTime().deltaTime;
        if (fpsTimer > 1.0f) {
            //std::cout << "FPS: " << scene->GetFPS() << std::endl;
            fpsTimer = 0.0f;
        }

        renderer->Frame();
    }

    vkDeviceWaitIdle(device->GetVkDevice());

    vkDestroyImage(device->GetVkDevice(), grassImage, nullptr);
    vkFreeMemory(device->GetVkDevice(), grassImageMemory, nullptr);

    vkDestroyCommandPool(device->GetVkDevice(), transferCommandPool, nullptr);

    delete scene;

    //delete terrain;
    delete terrainManager;
    //delete blades;
    delete camera;
    delete renderer;
    delete swapChain;
    delete device;
    delete instance;

    DestroyWindow();
    return 0;
}

#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <drakon/Renderable.h>

#include <vulkan/vulkan.h>

namespace drakon {
enum class RendererBackend {
    Vulkan,
};

struct Renderer {
    Renderer() = default;
    Renderer(RendererBackend backend);
    virtual ~Renderer() = default;
    bool                  render(std::vector<Renderable*> renderables);
    bool                  cleanup();
    void                  setClearColor(const std::array<float, 4> clearColor);
    std::array<float, 4>& getClearColor();
    RendererBackend       getBackend() const;
    bool                  compileGlslShader(const std::string& filename) const;

    bool init(void* windowHandle, uint32_t width, uint32_t height);

  protected:
    RendererBackend      backend            = RendererBackend::Vulkan;
    std::array<float, 4> clearColor         = {0.1f, 0.12f, 0.18f, 1.0f};
    void*                nativeWindowHandle = nullptr;
    uint32_t             windowWidth        = 1280;
    uint32_t             windowHeight       = 720;

    VkInstance                   vkInstance     = VK_NULL_HANDLE;
    VkSurfaceKHR                 vkSurface      = VK_NULL_HANDLE;
    VkPhysicalDevice             physicalDevice = VK_NULL_HANDLE;
    VkDevice                     vkDevice       = VK_NULL_HANDLE;
    VkQueue                      graphicsQueue  = VK_NULL_HANDLE;
    VkQueue                      presentQueue   = VK_NULL_HANDLE;
    VkSwapchainKHR               swapchain      = VK_NULL_HANDLE;
    std::vector<VkImage>         swapchainImages;
    std::vector<VkImageView>     swapchainViews;
    VkFormat                     swapchainFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D                   swapchainExtent = {};
    VkRenderPass                 renderPass      = VK_NULL_HANDLE;
    std::vector<VkFramebuffer>   framebuffers;
    VkCommandPool                commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore>     imageAvailableSemaphores;
    std::vector<VkSemaphore>     renderFinishedSemaphores;
    std::vector<VkFence>         inFlightFences;
    uint32_t                     currentFrame = 0;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
    };

    bool               createVulkanInstance();
    bool               createVulkanSurface();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    bool               pickPhysicalDevice();
    bool               createLogicalDevice();
    bool               createSwapchain();
    bool               createImageViews();
    bool               createRenderPass();
    bool               createFramebuffers();
    bool               createCommandPool();
    bool               createCommandBuffers();
    bool               createSyncObjects();
    bool               recordCommandBuffer(VkCommandBuffer                 commandBuffer,
                                           uint32_t                        imageIndex,
                                           const std::vector<Renderable*>& renderables);
    void               destroySwapchainResources();
};
} // namespace drakon
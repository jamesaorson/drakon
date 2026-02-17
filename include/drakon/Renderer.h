#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include <drakon/Renderable.h>

#if defined(WIN32) || defined(_WIN64)
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#endif

#if defined(DRAKON_HAS_VULKAN)
#include <vulkan/vulkan.h>
#endif

namespace drakon {
enum class RendererBackend {
    Auto,
    DirectX12,
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

    bool init(void* windowHandle, uint32_t width, uint32_t height);

  protected:
    RendererBackend      backend    = RendererBackend::Auto;
    std::array<float, 4> clearColor = {0.1f, 0.12f, 0.18f, 1.0f};
#if defined(WIN32) || defined(_WIN64)
    HWND windowHandle = nullptr;
    UINT windowWidth  = 1280;
    UINT windowHeight = 720;

    Microsoft::WRL::ComPtr<IDXGIFactory6>             dxgiFactory;
    Microsoft::WRL::ComPtr<ID3D12Device>              d3dDevice;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue>        commandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain3>           swapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>      rtvHeap;
    Microsoft::WRL::ComPtr<ID3D12Resource>            renderTargets[2];
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator>    commandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
    Microsoft::WRL::ComPtr<ID3D12Fence>               fence;
    HANDLE                                            fenceEvent        = nullptr;
    UINT                                              frameIndex        = 0;
    UINT                                              rtvDescriptorSize = 0;
    UINT64                                            fenceValue        = 0;

    // D3D12 specific members
    bool createCommandObjects();
    bool createCommandQueue();
    bool createDevice();
    bool createFence();
    bool createRenderTargets();
    bool createRtvHeap();
    bool createSwapChain();
    bool waitForGpu();
#endif

#if defined(DRAKON_HAS_VULKAN)
    void*    nativeWindowHandle = nullptr;
    uint32_t windowWidth        = 1280;
    uint32_t windowHeight       = 720;

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
    bool               recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void               destroySwapchainResources();
#endif
};
} // namespace drakon
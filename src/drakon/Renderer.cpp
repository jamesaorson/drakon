#include <drakon/Renderer.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <string>
#include <vector>

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace {
constexpr uint32_t                   MAX_FRAMES_IN_FLIGHT = 2;
constexpr std::array<const char*, 1> DEVICE_EXTENSIONS    = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
constexpr std::array<const char*, 1> VALIDATION_LAYERS    = {"VK_LAYER_KHRONOS_validation"};
#if defined(NDEBUG)
constexpr bool ENABLE_VALIDATION = false;
#else
constexpr bool ENABLE_VALIDATION = true;
#endif

struct SwapchainSupportDetails {
    VkSurfaceCapabilitiesKHR        capabilities = {};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR>   presentModes;
};

SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
    SwapchainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount > 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount > 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D
chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t windowWidth, uint32_t windowHeight) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }

    VkExtent2D actualExtent = {windowWidth, windowHeight};
    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actualExtent;
}

bool checkValidationLayerSupport() {
    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : VALIDATION_LAYERS) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}
} // namespace

drakon::Renderer::Renderer(RendererBackend backend) : backend(backend) {}

void drakon::Renderer::setClearColor(std::array<float, 4> clearColor) {
    this->clearColor[0] = clearColor[0];
    this->clearColor[1] = clearColor[1];
    this->clearColor[2] = clearColor[2];
    this->clearColor[3] = clearColor[3];
}

std::array<float, 4>& drakon::Renderer::getClearColor() { return this->clearColor; }

drakon::RendererBackend drakon::Renderer::getBackend() const { return this->backend; }

bool drakon::Renderer::createVulkanInstance() {
    if (ENABLE_VALIDATION && !checkValidationLayerSupport()) {
        std::cerr << "Requested Vulkan validation layers are not available." << std::endl;
        return false;
    }

    VkApplicationInfo appInfo  = {};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "drakon";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
    appInfo.pEngineName        = "drakon";
    appInfo.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_0;

    uint32_t     extensionCount = 0;
    const char** extensions     = glfwGetRequiredInstanceExtensions(&extensionCount);
    if (extensions == nullptr) {
        std::cerr << "Failed to query GLFW Vulkan instance extensions." << std::endl;
        return false;
    }

    std::vector<const char*> enabledExtensions(extensions, extensions + extensionCount);

    VkInstanceCreateInfo createInfo    = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo        = &appInfo;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();

    if (ENABLE_VALIDATION) {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }

    if (vkCreateInstance(&createInfo, nullptr, &this->vkInstance) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan instance." << std::endl;
        return false;
    }

    return true;
}

bool drakon::Renderer::createVulkanSurface() {
    auto* glfwWindow = reinterpret_cast<GLFWwindow*>(this->nativeWindowHandle);
    if (glfwCreateWindowSurface(this->vkInstance, glfwWindow, nullptr, &this->vkSurface) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan surface." << std::endl;
        return false;
    }
    return true;
}

drakon::Renderer::QueueFamilyIndices drakon::Renderer::findQueueFamilies(VkPhysicalDevice device) const {
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t index = 0;
    for (const auto& queueFamily : queueFamilies) {
        if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
            indices.graphicsFamily = index;
        }

        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, index, this->vkSurface, &presentSupport);
        if (presentSupport == VK_TRUE) {
            indices.presentFamily = index;
        }

        if (indices.isComplete()) {
            break;
        }
        ++index;
    }

    return indices;
}

bool drakon::Renderer::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0) {
        std::cerr << "No Vulkan-capable devices found." << std::endl;
        return false;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(this->vkInstance, &deviceCount, devices.data());

    for (const auto& candidate : devices) {
        QueueFamilyIndices indices = this->findQueueFamilies(candidate);
        if (!indices.isComplete() || !checkDeviceExtensionSupport(candidate)) {
            continue;
        }

        SwapchainSupportDetails support = querySwapchainSupport(candidate, this->vkSurface);
        if (support.formats.empty() || support.presentModes.empty()) {
            continue;
        }

        this->physicalDevice = candidate;
        return true;
    }

    std::cerr << "No suitable Vulkan physical device found." << std::endl;
    return false;
}

bool drakon::Renderer::createLogicalDevice() {
    QueueFamilyIndices indices             = this->findQueueFamilies(this->physicalDevice);
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float                                queuePriority = 1.0f;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    queueCreateInfos.reserve(uniqueQueueFamilies.size());

    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex        = queueFamily;
        queueCreateInfo.queueCount              = 1;
        queueCreateInfo.pQueuePriorities        = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    VkDeviceCreateInfo createInfo      = {};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pEnabledFeatures        = &deviceFeatures;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(DEVICE_EXTENSIONS.size());
    createInfo.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();

    if (ENABLE_VALIDATION) {
        createInfo.enabledLayerCount   = static_cast<uint32_t>(VALIDATION_LAYERS.size());
        createInfo.ppEnabledLayerNames = VALIDATION_LAYERS.data();
    }

    if (vkCreateDevice(this->physicalDevice, &createInfo, nullptr, &this->vkDevice) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan logical device." << std::endl;
        return false;
    }

    vkGetDeviceQueue(this->vkDevice, indices.graphicsFamily.value(), 0, &this->graphicsQueue);
    vkGetDeviceQueue(this->vkDevice, indices.presentFamily.value(), 0, &this->presentQueue);

    return true;
}

bool drakon::Renderer::createSwapchain() {
    SwapchainSupportDetails supportDetails = querySwapchainSupport(this->physicalDevice, this->vkSurface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(supportDetails.formats);
    VkPresentModeKHR   presentMode   = chooseSwapchainPresentMode(supportDetails.presentModes);
    VkExtent2D extent = chooseSwapchainExtent(supportDetails.capabilities, this->windowWidth, this->windowHeight);

    uint32_t imageCount = supportDetails.capabilities.minImageCount + 1;
    if (supportDetails.capabilities.maxImageCount > 0 && imageCount > supportDetails.capabilities.maxImageCount) {
        imageCount = supportDetails.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface                  = this->vkSurface;
    createInfo.minImageCount            = imageCount;
    createInfo.imageFormat              = surfaceFormat.format;
    createInfo.imageColorSpace          = surfaceFormat.colorSpace;
    createInfo.imageExtent              = extent;
    createInfo.imageArrayLayers         = 1;
    createInfo.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices              = this->findQueueFamilies(this->physicalDevice);
    uint32_t           queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices   = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;
    }

    createInfo.preTransform   = supportDetails.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(this->vkDevice, &createInfo, nullptr, &this->swapchain) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan swapchain." << std::endl;
        return false;
    }

    vkGetSwapchainImagesKHR(this->vkDevice, this->swapchain, &imageCount, nullptr);
    this->swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(this->vkDevice, this->swapchain, &imageCount, this->swapchainImages.data());

    this->swapchainFormat = surfaceFormat.format;
    this->swapchainExtent = extent;

    return true;
}

bool drakon::Renderer::createImageViews() {
    this->swapchainViews.resize(this->swapchainImages.size());

    for (size_t i = 0; i < this->swapchainImages.size(); i++) {
        VkImageViewCreateInfo createInfo           = {};
        createInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image                           = this->swapchainImages[i];
        createInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format                          = this->swapchainFormat;
        createInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel   = 0;
        createInfo.subresourceRange.levelCount     = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount     = 1;

        if (vkCreateImageView(this->vkDevice, &createInfo, nullptr, &this->swapchainViews[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create Vulkan image view." << std::endl;
            return false;
        }
    }

    return true;
}

bool drakon::Renderer::createRenderPass() {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format                  = this->swapchainFormat;
    colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment            = 0;
    colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass          = 0;
    dependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask       = 0;
    dependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount        = 1;
    renderPassInfo.pAttachments           = &colorAttachment;
    renderPassInfo.subpassCount           = 1;
    renderPassInfo.pSubpasses             = &subpass;
    renderPassInfo.dependencyCount        = 1;
    renderPassInfo.pDependencies          = &dependency;

    if (vkCreateRenderPass(this->vkDevice, &renderPassInfo, nullptr, &this->renderPass) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan render pass." << std::endl;
        return false;
    }

    return true;
}

bool drakon::Renderer::createFramebuffers() {
    this->framebuffers.resize(this->swapchainViews.size());

    for (size_t i = 0; i < this->swapchainViews.size(); ++i) {
        VkImageView attachments[] = {this->swapchainViews[i]};

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass              = this->renderPass;
        framebufferInfo.attachmentCount         = 1;
        framebufferInfo.pAttachments            = attachments;
        framebufferInfo.width                   = this->swapchainExtent.width;
        framebufferInfo.height                  = this->swapchainExtent.height;
        framebufferInfo.layers                  = 1;

        if (vkCreateFramebuffer(this->vkDevice, &framebufferInfo, nullptr, &this->framebuffers[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create Vulkan framebuffer." << std::endl;
            return false;
        }
    }

    return true;
}

bool drakon::Renderer::createCommandPool() {
    QueueFamilyIndices queueFamilyIndices = this->findQueueFamilies(this->physicalDevice);

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags                   = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex        = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(this->vkDevice, &poolInfo, nullptr, &this->commandPool) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan command pool." << std::endl;
        return false;
    }

    return true;
}

bool drakon::Renderer::createCommandBuffers() {
    this->commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool                 = this->commandPool;
    allocInfo.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount          = static_cast<uint32_t>(this->commandBuffers.size());

    if (vkAllocateCommandBuffers(this->vkDevice, &allocInfo, this->commandBuffers.data()) != VK_SUCCESS) {
        std::cerr << "Failed to allocate Vulkan command buffers." << std::endl;
        return false;
    }

    return true;
}

bool drakon::Renderer::createSyncObjects() {
    this->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    this->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    this->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags             = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(this->vkDevice, &semaphoreInfo, nullptr, &this->imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(this->vkDevice, &semaphoreInfo, nullptr, &this->renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateFence(this->vkDevice, &fenceInfo, nullptr, &this->inFlightFences[i]) != VK_SUCCESS) {
            std::cerr << "Failed to create Vulkan synchronization objects." << std::endl;
            return false;
        }
    }

    return true;
}

bool drakon::Renderer::recordCommandBuffer(VkCommandBuffer                 commandBuffer,
                                           uint32_t                        imageIndex,
                                           const std::vector<Renderable*>& renderables) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        std::cerr << "Failed to begin recording Vulkan command buffer." << std::endl;
        return false;
    }

    VkClearValue clearValue     = {};
    clearValue.color.float32[0] = this->clearColor[0];
    clearValue.color.float32[1] = this->clearColor[1];
    clearValue.color.float32[2] = this->clearColor[2];
    clearValue.color.float32[3] = this->clearColor[3];

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass            = this->renderPass;
    renderPassInfo.framebuffer           = this->framebuffers[imageIndex];
    renderPassInfo.renderArea.offset     = {0, 0};
    renderPassInfo.renderArea.extent     = this->swapchainExtent;
    renderPassInfo.clearValueCount       = 1;
    renderPassInfo.pClearValues          = &clearValue;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    for (auto* renderable : renderables) {
        if (renderable == nullptr) {
            continue;
        }
        renderable->draw(commandBuffer, this->vkDevice, this->renderPass, this->swapchainExtent);
    }
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        std::cerr << "Failed to record Vulkan command buffer." << std::endl;
        return false;
    }

    return true;
}

void drakon::Renderer::destroySwapchainResources() {
    for (auto framebuffer : this->framebuffers) {
        vkDestroyFramebuffer(this->vkDevice, framebuffer, nullptr);
    }
    this->framebuffers.clear();

    if (this->renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(this->vkDevice, this->renderPass, nullptr);
        this->renderPass = VK_NULL_HANDLE;
    }

    for (auto imageView : this->swapchainViews) {
        vkDestroyImageView(this->vkDevice, imageView, nullptr);
    }
    this->swapchainViews.clear();

    if (this->swapchain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(this->vkDevice, this->swapchain, nullptr);
        this->swapchain = VK_NULL_HANDLE;
    }
}

bool drakon::Renderer::init(void* windowHandle, uint32_t width, uint32_t height) {
    this->backend            = RendererBackend::Vulkan;
    this->nativeWindowHandle = windowHandle;
    this->windowWidth        = width;
    this->windowHeight       = height;

    if (!this->createVulkanInstance()) {
        return false;
    }
    if (!this->createVulkanSurface()) {
        return false;
    }
    if (!this->pickPhysicalDevice()) {
        return false;
    }
    if (!this->createLogicalDevice()) {
        return false;
    }
    if (!this->createSwapchain()) {
        return false;
    }
    if (!this->createImageViews()) {
        return false;
    }
    if (!this->createRenderPass()) {
        return false;
    }
    if (!this->createFramebuffers()) {
        return false;
    }
    if (!this->createCommandPool()) {
        return false;
    }
    if (!this->createCommandBuffers()) {
        return false;
    }
    if (!this->createSyncObjects()) {
        return false;
    }

    return true;
}

bool drakon::Renderer::render(std::vector<Renderable*> renderables) {
    vkWaitForFences(this->vkDevice, 1, &this->inFlightFences[this->currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex    = 0;
    VkResult acquireResult = vkAcquireNextImageKHR(this->vkDevice,
                                                   this->swapchain,
                                                   UINT64_MAX,
                                                   this->imageAvailableSemaphores[this->currentFrame],
                                                   VK_NULL_HANDLE,
                                                   &imageIndex);

    if (acquireResult != VK_SUCCESS && acquireResult != VK_SUBOPTIMAL_KHR) {
        std::cerr << "Failed to acquire Vulkan swapchain image." << std::endl;
        return false;
    }

    vkResetFences(this->vkDevice, 1, &this->inFlightFences[this->currentFrame]);
    vkResetCommandBuffer(this->commandBuffers[this->currentFrame], 0);

    if (!this->recordCommandBuffer(this->commandBuffers[this->currentFrame], imageIndex, renderables)) {
        return false;
    }

    VkSemaphore          waitSemaphores[]   = {this->imageAvailableSemaphores[this->currentFrame]};
    VkPipelineStageFlags waitStages[]       = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore          signalSemaphores[] = {this->renderFinishedSemaphores[this->currentFrame]};

    VkSubmitInfo submitInfo         = {};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount   = 1;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &this->commandBuffers[this->currentFrame];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    if (vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, this->inFlightFences[this->currentFrame]) != VK_SUCCESS) {
        std::cerr << "Failed to submit Vulkan draw command buffer." << std::endl;
        return false;
    }

    VkPresentInfoKHR presentInfo   = {};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores    = signalSemaphores;

    VkSwapchainKHR swapchains[] = {this->swapchain};
    presentInfo.swapchainCount  = 1;
    presentInfo.pSwapchains     = swapchains;
    presentInfo.pImageIndices   = &imageIndex;

    VkResult presentResult = vkQueuePresentKHR(this->presentQueue, &presentInfo);
    if (presentResult != VK_SUCCESS && presentResult != VK_SUBOPTIMAL_KHR) {
        std::cerr << "Failed to present Vulkan swapchain image." << std::endl;
        return false;
    }

    this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    return true;
}

bool drakon::Renderer::cleanup() {
    if (this->vkDevice != VK_NULL_HANDLE) {
        vkDeviceWaitIdle(this->vkDevice);
    }

    for (size_t i = 0; i < this->imageAvailableSemaphores.size(); ++i) {
        vkDestroySemaphore(this->vkDevice, this->imageAvailableSemaphores[i], nullptr);
    }
    this->imageAvailableSemaphores.clear();

    for (size_t i = 0; i < this->renderFinishedSemaphores.size(); ++i) {
        vkDestroySemaphore(this->vkDevice, this->renderFinishedSemaphores[i], nullptr);
    }
    this->renderFinishedSemaphores.clear();

    for (size_t i = 0; i < this->inFlightFences.size(); ++i) {
        vkDestroyFence(this->vkDevice, this->inFlightFences[i], nullptr);
    }
    this->inFlightFences.clear();

    this->destroySwapchainResources();

    if (this->commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(this->vkDevice, this->commandPool, nullptr);
        this->commandPool = VK_NULL_HANDLE;
    }

    if (this->vkDevice != VK_NULL_HANDLE) {
        vkDestroyDevice(this->vkDevice, nullptr);
        this->vkDevice = VK_NULL_HANDLE;
    }

    if (this->vkSurface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(this->vkInstance, this->vkSurface, nullptr);
        this->vkSurface = VK_NULL_HANDLE;
    }

    if (this->vkInstance != VK_NULL_HANDLE) {
        vkDestroyInstance(this->vkInstance, nullptr);
        this->vkInstance = VK_NULL_HANDLE;
    }

    return true;
}

bool drakon::Renderer::compileGlslShader(const std::string& filename) const {
    if (filename.empty()) {
        std::cerr << "Shader filename cannot be empty." << std::endl;
        return false;
    }

    const std::string outputFilename = filename + ".spv";
    const std::string command        = "glslc \"" + filename + "\" -o \"" + outputFilename + "\"";

    const int result = std::system(command.c_str());
    if (result != 0) {
        std::cerr << "Failed to compile GLSL shader: " << filename << std::endl;
        return false;
    }

    return true;
}

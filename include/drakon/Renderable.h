#pragma once

#include <vulkan/vulkan.h>

namespace drakon {
struct Renderable {
    virtual void draw(VkCommandBuffer commandBuffer, VkDevice device, VkRenderPass renderPass, VkExtent2D extent) = 0;

  protected:
    bool isInitialized = false;

    VkPipelineLayout pipelineLayout   = VK_NULL_HANDLE;
    VkPipeline       graphicsPipeline = VK_NULL_HANDLE;

    bool ensurePipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D extent);
};
} // namespace drakon

#pragma once

#if defined(WIN32) || defined(_WIN64)
#include <drakon/RenderPipeline.h>

#include <d3d12.h>
#include <wrl/client.h>
#endif

#if defined(DRAKON_HAS_VULKAN)
#include <vulkan/vulkan.h>
#endif

namespace drakon {
struct Renderable {
#if defined(WIN32) || defined(_WIN64)
    virtual void draw(ID3D12GraphicsCommandList& commandList) = 0;
#endif
#if defined(DRAKON_HAS_VULKAN)
    virtual void draw(VkCommandBuffer commandBuffer, VkDevice device, VkRenderPass renderPass, VkExtent2D extent) = 0;
#endif
  protected:
    bool isInitialized = false;

#if defined(WIN32) || defined(_WIN64)
    RenderPipelineConfig pipelineConfig = RenderPipeline::createDefaultPipelineConfig(nullptr);
    RenderPipelineState  renderState;

    bool ensurePipeline(ID3D12GraphicsCommandList& commandList) {
        if (this->isInitialized)
            return true;
        if (RenderPipeline::initialize(commandList, this->pipelineConfig, this->renderState)) {
            this->isInitialized = true;
        }
        return this->isInitialized;
    }
#endif
};
} // namespace drakon

#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>

#include <drakon/Game.h>
#include <drakon/Renderable.h>

namespace {
std::vector<char> readFile(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return {};
    }

    const std::streamsize fileSize = file.tellg();
    if (fileSize <= 0) {
        std::cerr << "Shader file is empty: " << path << std::endl;
        return {};
    }

    std::vector<char> buffer(static_cast<size_t>(fileSize));
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    return buffer;
}

VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code) {
    if (code.empty()) {
        return VK_NULL_HANDLE;
    }

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize                 = code.size();
    createInfo.pCode                    = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule = VK_NULL_HANDLE;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        std::cerr << "Failed to create Vulkan shader module." << std::endl;
        return VK_NULL_HANDLE;
    }

    return shaderModule;
}
} // namespace

struct TriangleRenderable : public drakon::Renderable {
    explicit TriangleRenderable(std::filesystem::path shaderDirectory) : shaderDirectory(std::move(shaderDirectory)) {}

    void draw(VkCommandBuffer commandBuffer, VkDevice device, VkRenderPass renderPass, VkExtent2D extent) override {
        if (!this->ensurePipeline(device, renderPass, extent)) {
            return;
        }

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->graphicsPipeline);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }

  private:
    std::filesystem::path shaderDirectory;

    bool ensurePipeline(VkDevice device, VkRenderPass renderPass, VkExtent2D extent) {
        if (this->graphicsPipeline != VK_NULL_HANDLE) {
            return true;
        }

        auto vertCode = readFile(this->shaderDirectory / "triangle.vert.spv");
        auto fragCode = readFile(this->shaderDirectory / "triangle.frag.spv");
        if (vertCode.empty() || fragCode.empty()) {
            return false;
        }

        VkShaderModule vertShaderModule = createShaderModule(device, vertCode);
        VkShaderModule fragShaderModule = createShaderModule(device, fragCode);
        if (vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE) {
            if (vertShaderModule != VK_NULL_HANDLE) {
                vkDestroyShaderModule(device, vertShaderModule, nullptr);
            }
            if (fragShaderModule != VK_NULL_HANDLE) {
                vkDestroyShaderModule(device, fragShaderModule, nullptr);
            }
            return false;
        }

        VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
        vertShaderStageInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage                           = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module                          = vertShaderModule;
        vertShaderStageInfo.pName                           = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
        fragShaderStageInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage                           = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module                          = fragShaderModule;
        fragShaderStageInfo.pName                           = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

        VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
        vertexInputInfo.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount   = 0;
        vertexInputInfo.pVertexBindingDescriptions      = nullptr;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        vertexInputInfo.pVertexAttributeDescriptions    = nullptr;

        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport = {};
        viewport.x          = 0.0f;
        viewport.y          = 0.0f;
        viewport.width      = static_cast<float>(extent.width);
        viewport.height     = static_cast<float>(extent.height);
        viewport.minDepth   = 0.0f;
        viewport.maxDepth   = 1.0f;

        VkRect2D scissor = {};
        scissor.offset   = {0, 0};
        scissor.extent   = extent;

        VkPipelineViewportStateCreateInfo viewportState = {};
        viewportState.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount                     = 1;
        viewportState.pViewports                        = &viewport;
        viewportState.scissorCount                      = 1;
        viewportState.pScissors                         = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable                       = VK_FALSE;
        rasterizer.rasterizerDiscardEnable                = VK_FALSE;
        rasterizer.polygonMode                            = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth                              = 1.0f;
        rasterizer.cullMode                               = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace                              = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable                        = VK_FALSE;

        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable                  = VK_FALSE;
        multisampling.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable                       = VK_FALSE;
        colorBlending.logicOp                             = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount                     = 1;
        colorBlending.pAttachments                        = &colorBlendAttachment;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &this->pipelineLayout) != VK_SUCCESS) {
            std::cerr << "Failed to create Vulkan pipeline layout." << std::endl;
            vkDestroyShaderModule(device, vertShaderModule, nullptr);
            vkDestroyShaderModule(device, fragShaderModule, nullptr);
            return false;
        }

        VkGraphicsPipelineCreateInfo pipelineInfo = {};
        pipelineInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount                   = 2;
        pipelineInfo.pStages                      = shaderStages;
        pipelineInfo.pVertexInputState            = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState          = &inputAssembly;
        pipelineInfo.pViewportState               = &viewportState;
        pipelineInfo.pRasterizationState          = &rasterizer;
        pipelineInfo.pMultisampleState            = &multisampling;
        pipelineInfo.pColorBlendState             = &colorBlending;
        pipelineInfo.layout                       = this->pipelineLayout;
        pipelineInfo.renderPass                   = renderPass;
        pipelineInfo.subpass                      = 0;
        pipelineInfo.basePipelineHandle           = VK_NULL_HANDLE;

        const VkResult createPipelineResult =
            vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &this->graphicsPipeline);

        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        vkDestroyShaderModule(device, fragShaderModule, nullptr);

        if (createPipelineResult != VK_SUCCESS) {
            std::cerr << "Failed to create Vulkan graphics pipeline." << std::endl;
            if (this->pipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(device, this->pipelineLayout, nullptr);
                this->pipelineLayout = VK_NULL_HANDLE;
            }
            return false;
        }

        return true;
    }
};

struct Game : public drakon::Game {
    using drakon::Game::Game;

    std::array<float, 4> clearColorDirection = {0.1f, 0.2f, 0.3f, 0.0f};

    void init() override {
        std::cout << "Initializing Vulkan game" << std::endl;
        const std::filesystem::path shaderDirectory = std::filesystem::path(__FILE__).parent_path() / "shaders";

        if (!this->renderer.compileGlslShader((shaderDirectory / "triangle.vert").string())) {
            std::cerr << "Failed to compile vertex shader." << std::endl;
            return;
        }
        if (!this->renderer.compileGlslShader((shaderDirectory / "triangle.frag").string())) {
            std::cerr << "Failed to compile fragment shader." << std::endl;
            return;
        }

        this->renderables.push_back(new TriangleRenderable(shaderDirectory));
    }

    void tick(const drakon::Delta delta) override { this->updateClearColor(delta); }

  private:
    void updateClearColor(const drakon::Delta delta) {
        auto& clearColor = this->renderer.getClearColor();
        for (size_t i = 0; i < clearColor.size(); ++i) {
            clearColor[i] += this->clearColorDirection[i] * delta;
            if (clearColor[i] > 1.0f) {
                clearColor[i] = 1.0f;
                this->clearColorDirection[i] *= -1.0f;
            } else if (clearColor[i] < 0.0f) {
                clearColor[i] = 0.0f;
                this->clearColorDirection[i] *= -1.0f;
            }
        }
    }
};

int main() {
    Game game("Hello Vulkan", drakon::RendererBackend::Vulkan);
    game.run();
    return 0;
}

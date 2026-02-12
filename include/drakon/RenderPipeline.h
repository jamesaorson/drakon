#pragma once

#if defined(WIN32) || defined(_WIN64)
#include <d3d12.h>
#include <wrl/client.h>
#endif

namespace drakon {
struct RenderPipelineConfig {
#if defined(WIN32) || defined(_WIN64)
    const char*                   shaderSource       = nullptr;
    const char*                   vertexEntry        = "VSMain";
    const char*                   pixelEntry         = "PSMain";
    D3D12_ROOT_SIGNATURE_FLAGS    rootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
    D3D12_BLEND_DESC              blendDesc          = {};
    D3D12_RASTERIZER_DESC         rasterDesc         = {};
    D3D12_DEPTH_STENCIL_DESC      depthStencilDesc   = {};
    DXGI_FORMAT                   rtvFormat          = DXGI_FORMAT_R8G8B8A8_UNORM;
    UINT                          sampleCount        = 1;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType       = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
#endif
};

struct RenderPipelineState {
#if defined(WIN32) || defined(_WIN64)
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
#endif
};

struct RenderPipeline {
#if defined(WIN32) || defined(_WIN64)
    static RenderPipelineConfig createDefaultPipelineConfig(const char* shaderSource);
    static bool                 initialize(ID3D12GraphicsCommandList&  commandList,
                                           const RenderPipelineConfig& config,
                                           RenderPipelineState&        outState);
#endif
};
} // namespace drakon

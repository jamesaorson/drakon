#include "TriangleRenderable.h"

#if defined(WIN32) || defined(_WIN64)

#include <d3d12.h>

const char* TriangleRenderable::shaderSource =
"struct VSOut { float4 pos : SV_Position; float4 color : COLOR0; };"
"VSOut VSMain(uint id : SV_VertexID) {"
"  float2 positions[3] = { float2(0.0f, 0.5f), float2(0.5f, -0.5f), float2(-0.5f, -0.5f) };"
"  float4 colors[3] = { float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, 0.0f, 1.0f, 1.0f) };"
"  VSOut o;"
"  o.pos = float4(positions[id], 0.0f, 1.0f);"
"  o.color = colors[id];"
"  return o;"
"}"
"float4 PSMain(VSOut input) : SV_Target { return input.color; }";

drakon::RenderPipelineConfig TriangleRenderable::defaultPipelineConfig = drakon::RenderPipeline::createDefaultPipelineConfig(TriangleRenderable::shaderSource);

TriangleRenderable::TriangleRenderable() {
	this->pipelineConfig = TriangleRenderable::defaultPipelineConfig;
}

void TriangleRenderable::draw(ID3D12GraphicsCommandList& commandList) {
	if (!this->ensurePipeline(commandList)) return;
	if (!this->renderState.pipelineState || !this->renderState.rootSignature) return;

	commandList.SetGraphicsRootSignature(this->renderState.rootSignature.Get());
	commandList.SetPipelineState(this->renderState.pipelineState.Get());
	commandList.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList.DrawInstanced(3, 1, 0, 0);
}
#endif

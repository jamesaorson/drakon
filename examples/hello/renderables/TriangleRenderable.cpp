#include "TriangleRenderable.h"

#if defined(WIN32) || defined(_WIN64)

#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

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

void TriangleRenderable::draw(ID3D12GraphicsCommandList& commandList) {
	Renderable::draw(commandList);
	if (!Renderable::pipelineState || !Renderable::rootSignature) return;

	commandList.SetGraphicsRootSignature(Renderable::rootSignature.Get());
	commandList.SetPipelineState(Renderable::pipelineState.Get());
	commandList.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList.DrawInstanced(3, 1, 0, 0);
}

void TriangleRenderable::initialize(ID3D12GraphicsCommandList& commandList) {
	if (this->isInitialized) return;

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	if (FAILED(commandList.GetDevice(IID_PPV_ARGS(&device)))) {
		return;
	}

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	Microsoft::WRL::ComPtr<ID3DBlob> rootSignatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	HRESULT hr = D3D12SerializeRootSignature(
		&rootSignatureDesc,
		D3D_ROOT_SIGNATURE_VERSION_1,
		&rootSignatureBlob,
		&errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			std::cerr << "Root signature error: "
				<< static_cast<const char*>(errorBlob->GetBufferPointer())
				<< std::endl;
		}
		return;
	}

	hr = device->CreateRootSignature(
		0,
		rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&Renderable::rootSignature));
	if (FAILED(hr)) {
		std::cerr << "CreateRootSignature failed." << std::endl;
		return;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	hr = D3DCompile(
		TriangleRenderable::shaderSource,
		std::strlen(TriangleRenderable::shaderSource),
		nullptr,
		nullptr,
		nullptr,
		"VSMain",
		"vs_5_0",
		0,
		0,
		&vertexShader,
		&errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			std::cerr << "Vertex shader error: "
				<< static_cast<const char*>(errorBlob->GetBufferPointer())
				<< std::endl;
		}
		return;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
	hr = D3DCompile(
		TriangleRenderable::shaderSource,
		std::strlen(TriangleRenderable::shaderSource),
		nullptr,
		nullptr,
		nullptr,
		"PSMain",
		"ps_5_0",
		0,
		0,
		&pixelShader,
		&errorBlob);
	if (FAILED(hr)) {
		if (errorBlob) {
			std::cerr << "Pixel shader error: "
				<< static_cast<const char*>(errorBlob->GetBufferPointer())
				<< std::endl;
		}
		return;
	}

	D3D12_BLEND_DESC blendDesc = {};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0] = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};

	D3D12_RASTERIZER_DESC rasterDesc = {};
	rasterDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterDesc.FrontCounterClockwise = FALSE;
	rasterDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterDesc.DepthClipEnable = TRUE;
	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.AntialiasedLineEnable = FALSE;
	rasterDesc.ForcedSampleCount = 0;
	rasterDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {};
	depthStencilDesc.DepthEnable = FALSE;
	depthStencilDesc.StencilEnable = FALSE;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	psoDesc.BlendState = blendDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = rasterDesc;
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.InputLayout = { nullptr, 0 };
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;

	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState));
	if (FAILED(hr)) {
		std::cerr << "CreateGraphicsPipelineState failed." << std::endl;
		return;
	}

	Renderable::initialize(commandList);
}
#endif

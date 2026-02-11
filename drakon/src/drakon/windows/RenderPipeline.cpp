#if defined(WIN32) || defined(_WIN64)

#include <drakon/RenderPipeline.h>

#include <d3dcompiler.h>

#include <cstring>
#include <iostream>

static D3D12_BLEND_DESC defaultBlendDesc() {
	D3D12_BLEND_DESC desc = {};
	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = FALSE;
	desc.RenderTarget[0] = {
		FALSE, FALSE,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};
	return desc;
}

static D3D12_RASTERIZER_DESC defaultRasterizerDesc() {
	D3D12_RASTERIZER_DESC desc = {};
	desc.FillMode = D3D12_FILL_MODE_SOLID;
	desc.CullMode = D3D12_CULL_MODE_BACK;
	desc.FrontCounterClockwise = FALSE;
	desc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	desc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	desc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	desc.DepthClipEnable = TRUE;
	desc.MultisampleEnable = FALSE;
	desc.AntialiasedLineEnable = FALSE;
	desc.ForcedSampleCount = 0;
	desc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	return desc;
}

static D3D12_DEPTH_STENCIL_DESC defaultDepthStencilDesc() {
	D3D12_DEPTH_STENCIL_DESC desc = {};
	desc.DepthEnable = FALSE;
	desc.StencilEnable = FALSE;
	return desc;
}

drakon::RenderPipelineConfig drakon::RenderPipeline::createDefaultPipelineConfig(const char* shaderSource) {
	RenderPipelineConfig config;
	config.shaderSource = shaderSource;
	config.blendDesc = defaultBlendDesc();
	config.rasterDesc = defaultRasterizerDesc();
	config.depthStencilDesc = defaultDepthStencilDesc();
	return config;
}

bool drakon::RenderPipeline::initialize(ID3D12GraphicsCommandList& commandList, const drakon::RenderPipelineConfig& config, drakon::RenderPipelineState& outState) {
	if (!config.shaderSource) return false;

	Microsoft::WRL::ComPtr<ID3D12Device> device;
	if (FAILED(commandList.GetDevice(IID_PPV_ARGS(&device)))) return false;

	D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
	rootSignatureDesc.Flags = config.rootSignatureFlags;
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
		return false;
	}

	hr = device->CreateRootSignature(
		0,
		rootSignatureBlob->GetBufferPointer(),
		rootSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&outState.rootSignature));
	if (FAILED(hr)) {
		std::cerr << "CreateRootSignature failed." << std::endl;
		return false;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	hr = D3DCompile(
		config.shaderSource,
		std::strlen(config.shaderSource),
		nullptr,
		nullptr,
		nullptr,
		config.vertexEntry,
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
		return false;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
	hr = D3DCompile(
		config.shaderSource,
		std::strlen(config.shaderSource),
		nullptr,
		nullptr,
		nullptr,
		config.pixelEntry,
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
		return false;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.pRootSignature = outState.rootSignature.Get();
	psoDesc.VS = { vertexShader->GetBufferPointer(), vertexShader->GetBufferSize() };
	psoDesc.PS = { pixelShader->GetBufferPointer(), pixelShader->GetBufferSize() };
	psoDesc.BlendState = config.blendDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.RasterizerState = config.rasterDesc;
	psoDesc.DepthStencilState = config.depthStencilDesc;
	psoDesc.InputLayout = { nullptr, 0 };
	psoDesc.PrimitiveTopologyType = config.topologyType;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = config.rtvFormat;
	psoDesc.SampleDesc.Count = config.sampleCount;

	hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&outState.pipelineState));
	if (FAILED(hr)) {
		std::cerr << "CreateGraphicsPipelineState failed." << std::endl;
		return false;
	}

	return true;
}
#endif

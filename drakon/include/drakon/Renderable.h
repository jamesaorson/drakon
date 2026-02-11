#pragma once

#include <functional>

#if defined(WIN32) || defined(_WIN64)
#include <d3d12.h>
#include <wrl/client.h>
#endif

namespace drakon {
	struct Renderable {
#if defined(WIN32) || defined(_WIN64)
		virtual void draw(ID3D12GraphicsCommandList& commandList);
		virtual void initialize(ID3D12GraphicsCommandList& commandList);
#endif
		protected:
			bool isInitialized = false;

#if defined(WIN32) || defined(_WIN64)
			static Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
			static Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
#endif
	};
}

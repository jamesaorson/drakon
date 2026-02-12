#pragma once

#include <array>
#include <functional>
#include <vector>

#include <drakon/Renderable.h>

#if defined(WIN32) || defined(_WIN64)
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#endif

namespace drakon {
	struct Renderer {
		virtual ~Renderer() = default;
		bool render(std::vector<Renderable*> renderables);
		bool cleanup();
		void setClearColor(const std::array<float, 4> clearColor);
		std::array<float, 4>& getClearColor();

#if defined(WIN32) || defined(_WIN64)
		bool init(HWND hwnd);
#endif

	protected:
		std::array<float, 4> clearColor = { 0.1f, 0.12f, 0.18f, 1.0f };
#if defined(WIN32) || defined(_WIN64)
		HWND windowHandle = nullptr;
		UINT windowWidth = 1280;
		UINT windowHeight = 720;

		Microsoft::WRL::ComPtr<IDXGIFactory6> dxgiFactory;
		Microsoft::WRL::ComPtr<ID3D12Device> d3dDevice;
		Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
		Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
		Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[2];
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
		Microsoft::WRL::ComPtr<ID3D12Fence> fence;
		HANDLE fenceEvent = nullptr;
		UINT frameIndex = 0;
		UINT rtvDescriptorSize = 0;
		UINT64 fenceValue = 0;

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
	};
}
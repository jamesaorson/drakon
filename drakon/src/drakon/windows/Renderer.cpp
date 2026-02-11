#if defined(WIN32) || defined(_WIN64)

#include <drakon/Renderer.h>

#include <iostream>

bool drakon::Renderer::createCommandObjects() {
	HRESULT hr = this->d3dDevice->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&this->commandAllocator));
	if (FAILED(hr)) {
		std::cerr << "CreateCommandAllocator failed." << std::endl;
		return false;
	}

	hr = this->d3dDevice->CreateCommandList(
		0, D3D12_COMMAND_LIST_TYPE_DIRECT, this->commandAllocator.Get(), nullptr, IID_PPV_ARGS(&this->commandList));
	if (FAILED(hr)) {
		std::cerr << "CreateCommandList failed." << std::endl;
		return false;
	}

	hr = this->commandList->Close();
	if (FAILED(hr)) {
		std::cerr << "CommandList close failed." << std::endl;
		return false;
	}

	return true;
}

bool drakon::Renderer::createCommandQueue() {
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;

	HRESULT hr = this->d3dDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&this->commandQueue));
	if (FAILED(hr)) {
		std::cerr << "CreateCommandQueue failed." << std::endl;
		return false;
	}

	return true;
}

bool drakon::Renderer::createDevice() {
	UINT factoryFlags = 0;
	HRESULT hr = CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&this->dxgiFactory));
	if (FAILED(hr)) {
		std::cerr << "CreateDXGIFactory2 failed." << std::endl;
		return false;
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	for (UINT i = 0; this->dxgiFactory->EnumAdapters1(i, &adapter) == S_OK; ++i) {
		DXGI_ADAPTER_DESC1 desc = {};
		adapter->GetDesc1(&desc);

		if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) {
			continue;
		}

		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&this->d3dDevice)))) {
			return true;
		}
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> warpAdapter;
	hr = this->dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter));
	if (FAILED(hr)) {
		std::cerr << "EnumWarpAdapter failed." << std::endl;
		return false;
	}

	hr = D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&this->d3dDevice));
	if (FAILED(hr)) {
		std::cerr << "D3D12CreateDevice failed." << std::endl;
		return false;
	}

	return true;
}

bool drakon::Renderer::createFence() {
	HRESULT hr = this->d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&this->fence));
	if (FAILED(hr)) {
		std::cerr << "CreateFence failed." << std::endl;
		return false;
	}

	this->fenceValue = 1;
	this->fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (this->fenceEvent == nullptr) {
		std::cerr << "CreateEvent failed." << std::endl;
		return false;
	}

	return true;
}

bool drakon::Renderer::createRenderTargets() {
	D3D12_CPU_DESCRIPTOR_HANDLE handle = this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < 2; ++i) {
		HRESULT hr = this->swapChain->GetBuffer(i, IID_PPV_ARGS(&this->renderTargets[i]));
		if (FAILED(hr)) {
			std::cerr << "GetBuffer failed." << std::endl;
			return false;
		}

		this->d3dDevice->CreateRenderTargetView(this->renderTargets[i].Get(), nullptr, handle);
		handle.ptr += this->rtvDescriptorSize;
	}

	return true;
}

bool drakon::Renderer::createRtvHeap() {
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = 2;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = this->d3dDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&this->rtvHeap));
	if (FAILED(hr)) {
		std::cerr << "CreateDescriptorHeap failed." << std::endl;
		return false;
	}

	this->rtvDescriptorSize = this->d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	return true;
}

bool drakon::Renderer::createSwapChain() {
	DXGI_SWAP_CHAIN_DESC1 desc = {};
	desc.BufferCount = 2;
	desc.Width = this->windowWidth;
	desc.Height = this->windowHeight;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.SampleDesc.Count = 1;

	Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;
	HRESULT hr = this->dxgiFactory->CreateSwapChainForHwnd(
		this->commandQueue.Get(),
		this->windowHandle,
		&desc,
		nullptr,
		nullptr,
		&swapChain1);

	if (FAILED(hr)) {
		std::cerr << "CreateSwapChainForHwnd failed." << std::endl;
		return false;
	}

	hr = swapChain1.As(&this->swapChain);
	if (FAILED(hr)) {
		std::cerr << "SwapChain cast failed." << std::endl;
		return false;
	}

	this->dxgiFactory->MakeWindowAssociation(this->windowHandle, DXGI_MWA_NO_ALT_ENTER);
	this->frameIndex = this->swapChain->GetCurrentBackBufferIndex();

	return true;
}

bool drakon::Renderer::init(HWND hwnd) {
	this->windowHandle = hwnd;
	if (!this->createDevice()) {
		return false;
	}
	if (!this->createCommandQueue()) {
		return false;
	}
	if (!this->createSwapChain()) {
		return false;
	}
	if (!this->createRtvHeap()) {
		return false;
	}
	if (!this->createRenderTargets()) {
		return false;
	}
	if (!this->createCommandObjects()) {
		return false;
	}
	if (!this->createFence()) {
		return false;
	}
	return true;
}

bool drakon::Renderer::render() {
	this->commandAllocator->Reset();
	this->commandList->Reset(this->commandAllocator.Get(), nullptr);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Transition.pResource = this->renderTargets[this->frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	this->commandList->ResourceBarrier(1, &barrier);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = this->rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += this->frameIndex * this->rtvDescriptorSize;

	this->commandList->ClearRenderTargetView(rtvHandle, this->clearColor.data(), 0, nullptr);

	std::swap(barrier.Transition.StateBefore, barrier.Transition.StateAfter);
	this->commandList->ResourceBarrier(1, &barrier);

	this->commandList->Close();

	ID3D12CommandList* lists[] = { this->commandList.Get() };
	this->commandQueue->ExecuteCommandLists(1, lists);

	this->swapChain->Present(1, 0);

	const UINT64 currentFence = this->fenceValue;
	this->commandQueue->Signal(this->fence.Get(), currentFence);
	this->fenceValue++;

	if (this->fence->GetCompletedValue() < currentFence) {
		this->fence->SetEventOnCompletion(currentFence, this->fenceEvent);
		WaitForSingleObject(this->fenceEvent, INFINITE);
	}

	this->frameIndex = this->swapChain->GetCurrentBackBufferIndex();

	return true;
}

bool drakon::Renderer::waitForGpu() {
	if (this->commandQueue == nullptr || this->fence == nullptr || this->fenceEvent == nullptr) {
		return false;
	}

	const UINT64 currentFence = this->fenceValue;
	this->commandQueue->Signal(this->fence.Get(), currentFence);
	this->fenceValue++;

	if (this->fence->GetCompletedValue() < currentFence) {
		this->fence->SetEventOnCompletion(currentFence, this->fenceEvent);
		WaitForSingleObject(this->fenceEvent, INFINITE);
	}
	return true;
}

bool drakon::Renderer::cleanup() {
	this->waitForGpu();
	if (this->fenceEvent != nullptr) {
		CloseHandle(this->fenceEvent);
		this->fenceEvent = nullptr;
	}
	return true;
}
#endif
#include <drakon/Renderable.h>

#if defined(WIN32) || defined(_WIN64)
Microsoft::WRL::ComPtr<ID3D12PipelineState> drakon::Renderable::pipelineState;
Microsoft::WRL::ComPtr<ID3D12RootSignature> drakon::Renderable::rootSignature;

void drakon::Renderable::draw(ID3D12GraphicsCommandList& commandList) {
	if (!this->isInitialized) initialize(commandList);
}

void drakon::Renderable::initialize(ID3D12GraphicsCommandList& commandList) {
	this->isInitialized = true;
}
#endif

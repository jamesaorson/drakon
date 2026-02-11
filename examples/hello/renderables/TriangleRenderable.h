#pragma once

#include <drakon/Renderable.h>
#include <drakon/RenderPipeline.h>

#if defined(WIN32) || defined(_WIN64)
#include <d3d12.h>
#endif

struct TriangleRenderable : public drakon::Renderable {
#if defined(WIN32) || defined(_WIN64)
	TriangleRenderable();

	static const char* shaderSource;
	static drakon::RenderPipelineConfig defaultPipelineConfig;

	void draw(ID3D12GraphicsCommandList& commandList) override;
#endif
};
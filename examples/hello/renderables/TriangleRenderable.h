#pragma once

#include <iostream>

#include <drakon/Renderable.h>

#if defined(WIN32) || defined(_WIN64)
#include <d3d12.h>
#endif

struct TriangleRenderable : public drakon::Renderable {
#if defined(WIN32) || defined(_WIN64)
	static const char* shaderSource;

	void draw(ID3D12GraphicsCommandList& commandList) override;
	void initialize(ID3D12GraphicsCommandList& commandList) override;
#endif
};
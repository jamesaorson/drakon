#pragma once

#include <functional>

#if defined(WIN32) || defined(_WIN64)
#include <d3d12.h>
#endif

namespace drakon {
	struct Renderable {
#if defined(WIN32) || defined(_WIN64)
		virtual void draw(ID3D12GraphicsCommandList& commandList) = 0;
#endif
	};
}

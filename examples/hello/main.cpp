#include <cstring>
#include <iostream>

#include <drakon/Game.h>
#include <drakon/Renderable.h>
#include <drakon/RenderPipeline.h>

#if defined(WIN32) || defined(_WIN64)
#include <d3d12.h>
#endif

struct TriangleRenderable : public drakon::Renderable {
#if defined(WIN32) || defined(_WIN64)
    TriangleRenderable() {
		this->pipelineConfig = TriangleRenderable::defaultPipelineConfig;
	}

    static inline const char* shaderSource =
        "struct VSOut { float4 pos : SV_Position; float4 color : COLOR0; };"
        "VSOut VSMain(uint id : SV_VertexID) {"
        "  float2 positions[3] = { float2(0.0f, 0.5f), float2(0.5f, -0.5f), float2(-0.5f, -0.5f) };"
        "  float4 colors[3] = { float4(1.0f, 0.0f, 0.0f, 1.0f), float4(0.0f, 1.0f, 0.0f, 1.0f), float4(0.0f, "
        "0.0f, 1.0f, 1.0f) };"
        "  VSOut o;"
        "  o.pos = float4(positions[id], 0.0f, 1.0f);"
        "  o.color = colors[id];"
        "  return o;"
        "}"
        "float4 PSMain(VSOut input) : SV_Target { return input.color; }";

    static inline drakon::RenderPipelineConfig defaultPipelineConfig =
        drakon::RenderPipeline::createDefaultPipelineConfig(TriangleRenderable::shaderSource);

    void draw(ID3D12GraphicsCommandList& commandList) override {
        if (!this->ensurePipeline(commandList))
            return;
        if (!this->renderState.pipelineState || !this->renderState.rootSignature)
            return;

        commandList.SetGraphicsRootSignature(this->renderState.rootSignature.Get());
        commandList.SetPipelineState(this->renderState.pipelineState.Get());
        commandList.IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        commandList.DrawInstanced(3, 1, 0, 0);
    }
#endif
};

struct Game : public drakon::Game {
	// Inherit constructors
	using drakon::Game::Game;

	std::array<float, 4> clearColorDirection = { 0.1f, 0.2f, 0.3f, 0.0f };

	void init() override
	{
		std::cout << "Initializing game" << std::endl;
		this->renderables.push_back(new TriangleRenderable);
	}

	void tick(const drakon::Delta delta) override
	{
		std::cout << "Ticking game after " << delta << " seconds" << std::endl;
		this->updateClearColor(delta);
	}

private:
	void updateClearColor(const drakon::Delta delta) {
		auto& clearColor = this->renderer.getClearColor();
		for (size_t i = 0; i < clearColor.size(); ++i) {
			clearColor[i] += this->clearColorDirection[i] * delta;
			if (clearColor[i] > 1.0f) {
				clearColor[i] = 1.0f;
				this->clearColorDirection[i] *= -1.0f;
			}
			else if (clearColor[i] < 0.0f) {
				clearColor[i] = 0.0f;
				this->clearColorDirection[i] *= -1.0f;
			}
		}
	}
};

int main()
{
	Game game("Hello");
	game.run();
	return 0;
}

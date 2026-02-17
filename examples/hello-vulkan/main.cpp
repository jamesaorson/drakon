#include <iostream>

#include <drakon/Game.h>

struct Game : public drakon::Game {
    using drakon::Game::Game;

    std::array<float, 4> clearColorDirection = {0.1f, 0.2f, 0.3f, 0.0f};

    void init() override { std::cout << "Initializing Vulkan game" << std::endl; }

    void tick(const drakon::Delta delta) override { this->updateClearColor(delta); }

  private:
    void updateClearColor(const drakon::Delta delta) {
        auto& clearColor = this->renderer.getClearColor();
        for (size_t i = 0; i < clearColor.size(); ++i) {
            clearColor[i] += this->clearColorDirection[i] * delta;
            if (clearColor[i] > 1.0f) {
                clearColor[i] = 1.0f;
                this->clearColorDirection[i] *= -1.0f;
            } else if (clearColor[i] < 0.0f) {
                clearColor[i] = 0.0f;
                this->clearColorDirection[i] *= -1.0f;
            }
        }
    }
};

int main() {
    Game game("Hello Vulkan", drakon::RendererBackend::Vulkan);
    game.run();
    return 0;
}

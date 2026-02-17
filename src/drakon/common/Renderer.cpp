#include <drakon/Renderer.h>

drakon::Renderer::Renderer(RendererBackend backend) : backend(backend) {}

void drakon::Renderer::setClearColor(std::array<float, 4> clearColor) {
    this->clearColor[0] = clearColor[0];
    this->clearColor[1] = clearColor[1];
    this->clearColor[2] = clearColor[2];
    this->clearColor[3] = clearColor[3];
}

std::array<float, 4>& drakon::Renderer::getClearColor() { return this->clearColor; }

drakon::RendererBackend drakon::Renderer::getBackend() const { return this->backend; }

#include <drakon/Game.h>

#include <iostream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <chrono>

void drakon::Game::run() {
    // Init before tracking time
    this->init();

    if (this->makeWindow() != 0) {
        this->cleanup();
        return;
    }

    auto          startTime = std::chrono::steady_clock::now();
    drakon::Delta delta     = 0;
    while (this->isRunning) {
        auto                                 currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<drakon::Delta> duration    = currentTime - startTime;
        delta                                            = duration.count();
        startTime                                        = currentTime;
        this->processEvents();
        // First frame will always have a near-0 value
        this->tick(delta);
        this->renderer.render(this->renderables);
    }
    this->done();
    this->cleanup();
}

int drakon::Game::makeWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;
        return 1;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    GLFWwindow* window = glfwCreateWindow(static_cast<int>(this->windowWidth),
                                          static_cast<int>(this->windowHeight),
                                          this->title.c_str(),
                                          nullptr,
                                          nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();
        return 1;
    }

    this->windowHandle = window;

    if (!this->renderer.init(this->windowHandle, this->windowWidth, this->windowHeight)) {
        std::cerr << "Failed to initialize renderer." << std::endl;
        glfwDestroyWindow(window);
        this->windowHandle = nullptr;
        glfwTerminate();
        return 1;
    }

    return 0;
}

void drakon::Game::processEvents() {
    glfwPollEvents();
    if (this->windowHandle != nullptr) {
        auto* window = reinterpret_cast<GLFWwindow*>(this->windowHandle);
        if (glfwWindowShouldClose(window)) {
            this->isRunning = false;
        }
    }
}

void drakon::Game::cleanup() {
    this->renderer.cleanup();

    if (this->windowHandle != nullptr) {
        glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(this->windowHandle));
        this->windowHandle = nullptr;
    }
    glfwTerminate();
}

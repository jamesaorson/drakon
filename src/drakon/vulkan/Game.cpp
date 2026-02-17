#if !defined(WIN32) && !defined(_WIN64)

#include <drakon/Game.h>

#include <iostream>

#if defined(DRAKON_HAS_GLFW)
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

int drakon::Game::makeWindow() {
#if !defined(DRAKON_HAS_GLFW)
    std::cerr << "GLFW is required on this platform to create windows." << std::endl;
    return 1;
#else
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
#endif
}

void drakon::Game::processEvents() {
#if defined(DRAKON_HAS_GLFW)
    glfwPollEvents();
    if (this->windowHandle != nullptr) {
        auto* window = reinterpret_cast<GLFWwindow*>(this->windowHandle);
        if (glfwWindowShouldClose(window)) {
            this->isRunning = false;
        }
    }
#endif
}

void drakon::Game::cleanup() {
    this->renderer.cleanup();

#if defined(DRAKON_HAS_GLFW)
    if (this->windowHandle != nullptr) {
        glfwDestroyWindow(reinterpret_cast<GLFWwindow*>(this->windowHandle));
        this->windowHandle = nullptr;
    }
    glfwTerminate();
#endif
}

#endif

#pragma once

#include <drakon/Renderer.h>
#include <drakon/Renderable.h>
#include <string>

namespace drakon {
typedef float Delta;

struct Game {
    Game() = default;
    Game(std::string title) : title(std::move(title)) {}

    void run();
    void cleanup();

  protected:
    bool                     isRunning = true;
    std::string              title     = "Drakon Game";
    drakon::Renderer         renderer;
    std::vector<Renderable*> renderables;

    // OS and render engine specific window creation logic
    int  makeWindow();
    void processEvents();
    // Abstract methods to be implemented by consuming party
    virtual void init() {}
    virtual void tick(const Delta delta) = 0;
    virtual void done() {}
};
} // namespace drakon
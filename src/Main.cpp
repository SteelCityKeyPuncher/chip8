#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "Window.h"

namespace {
constexpr double kFrameTime = 1.0 / 60.0;
} // namespace

int main(int, char **) {
  try {
    Window window;

    // Used to determine the duration since the previous game logic update
    // TODOdouble lastUpdateTime = 0.0;

    // Used to determine the duration since the previous render
    // Set to a negative value to guarantee that rendering occurs in the first
    // iteration
    double lastFrameTime = -1.0;

    // Fixed time step rendering logic
    // Run the update logic as fast as possible
    // If enough time has elapsed to actually render, then do so
    while (window.IsOpen()) {
      const auto currentTime = glfwGetTime();
      // TODOconst float deltaTime = currentTime - lastUpdateTime;
      // lastUpdateTime = currentTime;

      // TODO: Update logic here

      // See if we can render in this loop
      if ((currentTime - lastFrameTime) >= kFrameTime) {
        // It's time to render
        // Prepare the window for rendering (clear color and depth buffers)
        window.RenderStart();

        // TODO render here

        // Finish up window rendering (swap buffers)
        window.RenderEnd();
      }
    }

  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

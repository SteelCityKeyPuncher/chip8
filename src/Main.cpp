#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include "Chip8.h"

namespace {
// Constants
constexpr double kFrameTime = 1.0 / 60.0;

// Local types
struct glfwDeleter {
  void operator()(GLFWwindow *window) { glfwDestroyWindow(window); }
};

// Local variables
std::unique_ptr<GLFWwindow, glfwDeleter> window;
Chip8 chip8;

// Local functions
void parseArguments(int argc, char **argv);
void initializeGraphics();
void runLoop();
void glfwErrorCallback(int error, const char *description);
void glfwWindowSizeCallback(GLFWwindow *window, int, int);
} // namespace

int main(int argc, char **argv) {
  try {
    parseArguments(argc, argv);
    initializeGraphics();
    runLoop();
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

namespace {
void parseArguments(int argc, char **argv) {
  if (argc != 2) {
    throw std::runtime_error("Expected one argument (ROM path).");
  }

  chip8.LoadRom(argv[1]);
}

void initializeGraphics() {
  if (!glfwInit()) {
    throw std::runtime_error("Unable to initialize GLFW.");
  }

  glfwSetErrorCallback(glfwErrorCallback);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  auto monitor = glfwGetPrimaryMonitor();
  const auto videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

  window.reset(glfwCreateWindow(videoMode->width, videoMode->height, "CHIP-8",
                                monitor, nullptr));
  if (!window) {
    throw std::runtime_error("Unable to create GLFW window.");
  }

  // Update the framebuffer when the window is resized
  // This is necessary because, even in fullscreen mode, the screen may be
  // resized a few times (at least in X11/Ubuntu)
  glfwSetWindowSizeCallback(window.get(), glfwWindowSizeCallback);
  glfwMakeContextCurrent(window.get());

  if (!gladLoadGL()) {
    throw std::runtime_error("Unable to initialize glad.");
  }

  // Hide the mouse cursor
  glfwSetInputMode(window.get(), GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

  // Clear the screen to a dark blue
  glClearColor(0.0f, 0.0f, 0.25f, 1.0f);

  // TODO should be configurable
  // 0 = could tear, but swapping buffers doesn't block
  // 1 = no tearing, blocked at vsync rate
  glfwSwapInterval(1);

  // Initialize graphics for the CPU (load shader, etc.)
  chip8.InitializeGraphics();
}

void runLoop() {
  // Used to determine the duration since the previous game logic update
  double lastUpdateTime = 0.0;

  // Used to determine the duration since the previous render
  // Set to a negative value to guarantee that rendering occurs in the first
  // iteration
  double lastFrameTime = -1.0;

  // Fixed time step rendering logic
  // Run the update logic as fast as possible
  // If enough time has elapsed to actually render, then do so
  while (!glfwWindowShouldClose(window.get())) {
    const auto currentTime = glfwGetTime();
    const auto deltaTime = static_cast<float>(currentTime - lastUpdateTime);
    lastUpdateTime = currentTime;

    // Update the CPU
    chip8.Update(window.get(), deltaTime);

    // See if we can render in this loop
    if ((currentTime - lastFrameTime) >= kFrameTime) {
      // It's time to render
      // Prepare the window for rendering (clear color buffer)
      glClear(GL_COLOR_BUFFER_BIT);

      // Render from the CPU
      chip8.Draw();

      // Finish up window rendering (swap buffers)
      glfwSwapBuffers(window.get());
      glfwPollEvents();

      // Close the window if the ESC key is pressed
      if (glfwGetKey(window.get(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window.get(), true);
      }
    }
  }
}

void glfwErrorCallback(int error, const char *description) {
  throw std::runtime_error("GLFW error " + std::to_string(error) + ": " +
                           description);
}

void glfwWindowSizeCallback(GLFWwindow *window, int, int) {
  int frameBufferWidth;
  int frameBufferHeight;

  glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
  glViewport(0, 0, frameBufferWidth, frameBufferHeight);
}
} // namespace

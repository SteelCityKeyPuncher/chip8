#include <stdexcept>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Window.h"

namespace {
void errorCallback(int error, const char *description);

void windowSizeCallback(GLFWwindow *window, int, int) {
  int frameBufferWidth;
  int frameBufferHeight;
  glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);
  glViewport(0, 0, frameBufferWidth, frameBufferHeight);
}
} // namespace

Window::Window() {
  glfwSetErrorCallback(errorCallback);

  if (!glfwInit()) {
    throw std::runtime_error("Unable to initialize GLFW.");
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  auto monitor = glfwGetPrimaryMonitor();
  const auto videoMode = glfwGetVideoMode(glfwGetPrimaryMonitor());

  this->glfwWindow = glfwCreateWindow(videoMode->width, videoMode->height,
                                      "Chip8", monitor, nullptr);
  if (!this->glfwWindow) {
    throw std::runtime_error("Unable to create GLFW window.");
  }

  // Update the framebuffer when the window is resized
  // This is necessary because, even in fullscreen mode, the screen may be
  // resized a few times (at least in X11/Ubuntu)
  glfwSetWindowSizeCallback(this->glfwWindow, windowSizeCallback);
  glfwMakeContextCurrent(this->glfwWindow);

  if (!gladLoadGL()) {
    throw std::runtime_error("Unable to initialize glad.");
  }

  // Backface culling
  glEnable(GL_CULL_FACE);

  // Alpha blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Depth buffer
  glEnable(GL_DEPTH_TEST);

  // Clear the screen to a dark blue
  glClearColor(0.0f, 0.0f, 0.25f, 1.0f);

  // TODO should be configurable
  // 0 = could tear, but swapping buffers doesn't block
  // 1 = no tearing, blocked at vsync rate
  glfwSwapInterval(1);
}

Window::~Window() {
  glfwDestroyWindow(this->glfwWindow);
  glfwTerminate();
}

bool Window::IsOpen() const { return !glfwWindowShouldClose(this->glfwWindow); }

void Window::RenderStart() const {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Window::RenderEnd() const {
  glfwSwapBuffers(this->glfwWindow);
  glfwPollEvents();

  // TODO temporary way of closing the window
  if (glfwGetKey(this->glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(this->glfwWindow, true);
  }
}

namespace {
void errorCallback(int error, const char *description) {
  throw std::runtime_error("GLFW error " + std::to_string(error) + ": " +
                           description);
}
} // namespace

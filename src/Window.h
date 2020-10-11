#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <GLFW/glfw3.h>

class Window {
public:
  Window();
  ~Window();

  [[nodiscard]] bool IsOpen() const;
  void RenderStart() const;
  void RenderEnd() const;

private:
  GLFWwindow *glfwWindow = nullptr;
};

#endif // WINDOW_H_INCLUDED

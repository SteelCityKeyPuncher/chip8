#ifndef CHIP8_H_INCLUDED
#define CHIP8_H_INCLUDED

#include <array>
#include <cstdint>
#include <stack>
#include <string>

#include <GLFW/glfw3.h>
#include <glad/glad.h>

class Chip8 {
public:
  Chip8();
  ~Chip8();

  void InitializeGraphics();
  void LoadRom(const std::string &romPath);
  void Update(GLFWwindow *window, float deltaTime);
  void Draw();

private:
  void executeOneInstruction();
  [[nodiscard]] bool isPixelOn(uint16_t x, uint16_t y) const;
  void togglePixel(uint16_t x, uint16_t y);

private:
  // CPU stuff
  std::array<uint8_t, 4096> memory = {};
  std::array<uint8_t, 16> V = {};
  uint16_t I = 0;
  uint16_t PC = 0x200;
  std::stack<uint16_t> stack;
  std::array<bool, 16> keys = {};
  uint8_t delayTimer = 0;
  float delayTimerAccumulator = 0.f;
  uint8_t soundTimer = 0;
  // TODO need floating point helper for this one too

  // Graphics stuff
  GLuint shader;
  GLuint texture;
  GLuint VAO;
  GLuint VBO;
  GLuint EBO;
  std::array<uint8_t, 64 * 32 * 3> pixels = {};
};

#endif // CHIP8_H_INCLUDED

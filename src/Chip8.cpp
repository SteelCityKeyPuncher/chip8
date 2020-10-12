#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <limits>
#include <vector>

#include "Chip8.h"
#include "Util.h"

namespace {
// Constants
constexpr std::array<uint8_t, 80> kInternalFont = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10,
    0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10,
    0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0,
    0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
    0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80,
    0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80};

constexpr std::array<int, 16> kKeyMap = {
    GLFW_KEY_X, GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_Q, GLFW_KEY_W,
    GLFW_KEY_E, GLFW_KEY_A, GLFW_KEY_S, GLFW_KEY_D, GLFW_KEY_Y, GLFW_KEY_C,
    GLFW_KEY_4, GLFW_KEY_R, GLFW_KEY_F, GLFW_KEY_V};

// Functions
GLuint compileShader(const std::string &path, GLenum type);
GLuint linkShader(GLuint vertexShader, GLuint fragmentShader);
} // namespace

Chip8::Chip8() {
  // Copy the font to memory
  std::memcpy(&this->memory[0], kInternalFont.data(), kInternalFont.size());

  // Seed the random number generator
  // Ideally, the RNG from the STL would be better here
  srand(static_cast<unsigned int>(time(nullptr)));
}

Chip8::~Chip8() {
  if (this->EBO != static_cast<GLuint>(-1)) {
    glDeleteBuffers(1, &this->EBO);
  }

  if (this->VBO != static_cast<GLuint>(-1)) {
    glDeleteBuffers(1, &this->VBO);
  }

  if (this->VAO != static_cast<GLuint>(-1)) {
    glDeleteVertexArrays(1, &this->VAO);
  }

  if (this->shader != static_cast<GLuint>(-1)) {
    glDeleteProgram(this->shader);
  }
}

void Chip8::InitializeGraphics() {
  const auto vertexShader =
      compileShader("assets/shaders/Default.vs", GL_VERTEX_SHADER);
  const auto fragmentShader =
      compileShader("assets/shaders/Default.fs", GL_FRAGMENT_SHADER);

  this->shader = linkShader(vertexShader, fragmentShader);

  constexpr std::array<float, 6 * 5> vertices = {
      // Top left
      -1.f,
      1.f,
      0.0f,
      0.f,
      0.f,
      // Bottom left
      -1.f,
      -1.f,
      0.0f,
      0.f,
      1.f,
      // Bottom Right
      1.0f,
      -1.f,
      0.0f,
      1.f,
      1.f,
      // Top left
      -1.f,
      1.f,
      0.0f,
      0.f,
      0.f,
      // Bottom Right
      1.0f,
      -1.f,
      0.0f,
      1.f,
      1.f,
      // Top Right
      1.0f,
      1.f,
      0.0f,
      1.f,
      0.f,
  };

  glGenVertexArrays(1, &this->VAO);
  glGenBuffers(1, &this->VBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and
  // then configure vertex attributes(s).
  glBindVertexArray(this->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(),
               GL_STATIC_DRAW);

  // Position attribute (x, y, z)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)nullptr);
  glEnableVertexAttribArray(0);

  // Texture coordinates attribute (u, v)
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  glGenTextures(1, &this->texture);
  glBindTexture(GL_TEXTURE_2D, this->texture);

  // Wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // Filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 32, 0, GL_RGB, GL_UNSIGNED_BYTE,
               this->pixelBuffer.data());
}

void Chip8::LoadRom(const std::string &romPath) {
  // Read the ROM file into memory at 0x200
  auto fileData = Util::FileReadBinary(romPath);
  if (fileData.size() > this->memory.size() - 0x200) {
    throw std::runtime_error("The ROM is too large.");
  }

  std::memcpy(&this->memory[0x200], &fileData[0], fileData.size());
}

void Chip8::SetCpuRate(uint16_t instructionsPerSecond) {
  this->updateRate = instructionsPerSecond;
  this->updateTime = 1.f / this->updateRate;
}

void Chip8::Update(GLFWwindow *window, float deltaTime) {
  // Close the window if the ESC key is pressed
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
    if (this->updateRate < std::numeric_limits<uint16_t>::max()) {
      this->SetCpuRate(this->updateRate + 1);
    }
  }

  if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
    if (this->updateRate > 0) {
      this->SetCpuRate(this->updateRate - 1);
    }
  }

  for (size_t i = 0; i < 16; i++) {
    this->keys.at(i) = (glfwGetKey(window, kKeyMap[i]) == GLFW_PRESS);
  }

  this->delayTimerAccumulator += deltaTime;
  while (this->delayTimerAccumulator >= 0.f) {
    if (this->delayTimer != 0) {
      --this->delayTimer;
    }

    this->delayTimerAccumulator -= 1.f / 60.f;
  }

  // Execute CPU instructions at a constant rate
  this->updateAccumulator += deltaTime;
  while (this->updateAccumulator >= 0.f) {
    this->executeOneInstruction();
    this->updateAccumulator -= this->updateTime;
  }
}

void Chip8::Draw() {
  glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 64, 32, GL_RGB, GL_UNSIGNED_BYTE,
                  this->pixelBuffer.data());

  glUseProgram(this->shader);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, this->texture);

  glBindVertexArray(this->VAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Chip8::executeOneInstruction() {
  // Local lambda for invalid opcodes
  auto invalidOpcode = [](uint16_t opcode) {
    std::array<char, 64> buffer;
    sprintf(buffer.data(), "Invalid opcode: 0x%02X", opcode);
    throw std::runtime_error(buffer.data());
  };

  const uint16_t opcode =
      this->memory.at(this->PC + 1) | (this->memory.at(this->PC) << 8);

  this->PC += 2;

  switch (opcode & 0xF000) {
  case 0x0000:
    switch (opcode) {
    case 0x00E0:
      std::memset(this->pixels.data(), 0, this->pixels.size());
      std::memset(this->pixelBuffer.data(), 0, this->pixelBuffer.size());
      break;

    case 0x00EE:
      if (this->stack.empty()) {
        throw std::runtime_error("Corrupted stack.");
      }

      this->PC = this->stack.top();
      this->stack.pop();
      break;

    default:
      break;
    }
    break;

  case 0x1000:
    this->PC = opcode & 0x0FFF;
    break;

  case 0x2000:
    this->stack.push(this->PC);
    this->PC = opcode & 0x0FFF;
    break;

  case 0x3000:
    if (this->V.at((opcode & 0x0F00) >> 8) == (opcode & 0x00FF)) {
      this->PC += 2;
    }
    break;

  case 0x4000:
    if (this->V.at((opcode & 0x0F00) >> 8) != (opcode & 0x00FF)) {
      this->PC += 2;
    }
    break;

  case 0x5000:
    if (this->V.at((opcode & 0x0F00) >> 8) ==
        this->V.at((opcode & 0x00F0) >> 4)) {
      this->PC += 2;
    }
    break;

  case 0x6000:
    this->V.at((opcode & 0x0F00) >> 8) = opcode & 0x00FF;
    break;

  case 0x7000:
    this->V.at((opcode & 0x0F00) >> 8) += opcode & 0x00FF;
    break;

  case 0x8000: {
    switch (opcode & 0x000F) {
    case 0x0000:
      this->V.at((opcode & 0x0F00) >> 8) = this->V.at((opcode & 0x00F0) >> 4);
      break;

    case 0x0001:
      this->V.at((opcode & 0x0F00) >> 8) |= this->V.at((opcode & 0x00F0) >> 4);
      break;

    case 0x0002:
      this->V.at((opcode & 0x0F00) >> 8) &= this->V.at((opcode & 0x00F0) >> 4);
      break;

    case 0x0003:
      this->V.at((opcode & 0x0F00) >> 8) ^= this->V.at((opcode & 0x00F0) >> 4);
      break;

    case 0x0004: {
      int16_t sum = this->V.at((opcode & 0x0F00) >> 8) +
                    this->V.at((opcode & 0x00F0) >> 4);

      if (sum >= 256) {
        this->V.at(0xF) = 1;
        sum -= 256;
      } else {
        this->V.at(0xF) = 0;
      }

      this->V.at((opcode & 0x0F00) >> 8) = static_cast<uint8_t>(sum);
    } break;

    case 0x0005: {
      int16_t diff = this->V.at((opcode & 0x0F00) >> 8) -
                     this->V.at((opcode & 0x00F0) >> 4);

      if (diff < 0) {
        diff += 256;
        this->V.at(0xF) = 0;
      } else {
        this->V.at(0xF) = 1;
      }

      this->V.at((opcode & 0x0F00) >> 8) = static_cast<uint8_t>(diff);
    } break;

    case 0x0006:
      this->V.at(0xF) = this->V.at((opcode & 0x0F00) >> 8) & 0x1;
      this->V.at((opcode & 0x0F00) >> 8) >>= 1;
      break;

    case 0x0007: {
      int16_t diff = this->V.at((opcode & 0x00F0) >> 4) -
                     this->V.at((opcode & 0x0F00) >> 8);

      if (diff < 0) {
        diff += 256;
        this->V.at(0xF) = 0;
      } else {
        this->V.at(0xF) = 1;
      }

      this->V.at((opcode & 0x0F00) >> 8) = static_cast<uint8_t>(diff);
    } break;

    case 0x000E:
      this->V.at(0xF) = this->V.at((opcode & 0x0F00) >> 8) & 0x80;
      this->V.at((opcode & 0x0F00) >> 8) <<= 1;
      break;

    default:
      invalidOpcode(opcode);
    }
  } break;

  case 0x9000:
    if (this->V.at((opcode & 0x0F00) >> 8) !=
        this->V.at((opcode & 0x00F0) >> 4))
      this->PC += 2;
    break;

  case 0xA000:
    this->I = opcode & 0x0FFF;
    break;

  case 0xB000:
    this->PC = (opcode & 0x0FFF) + this->V.at(0);
    break;

  case 0xC000:
    this->V.at((opcode & 0x0F00) >> 8) = static_cast<uint8_t>(rand() & opcode);
    break;

  case 0xD000: {
    const auto xStart = this->V.at((opcode & 0x0F00) >> 8);
    const auto yStart = this->V.at((opcode & 0x00F0) >> 4);

    this->V.at(0xF) = 0;

    for (uint8_t yOffset = 0; yOffset < (opcode & 0x000F); yOffset++) {
      const auto data = this->memory.at(this->I + yOffset);

      for (uint8_t xOffset = 0; xOffset < 8; xOffset++) {
        if ((data & (0x80 >> xOffset)) != 0) {
          if (this->isPixelOn(xStart + xOffset, yStart + yOffset)) {
            this->V.at(0xF) = 1;
          }

          this->togglePixel(xStart + xOffset, yStart + yOffset);
        }
      }
    }
  } break;

  case 0xE000: {
    switch (opcode & 0x00FF) {
    case 0x009E:
      if (this->keys.at(this->V.at((opcode & 0x0F00) >> 8))) {
        this->PC += 2;
      }
      break;

    case 0x00A1:
      if (!this->keys.at(this->V.at((opcode & 0x0F00) >> 8))) {
        this->PC += 2;
      }
      break;

    default:
      invalidOpcode(opcode);
    }
  } break;

  case 0xF000: {
    switch (opcode & 0x00FF) {
    case 0x0007:
      this->V.at((opcode & 0x0F00) >> 8) = this->delayTimer;
      break;

    case 0x000A: {
      bool keyPressed = false;

      for (uint8_t index = 0; index < this->keys.size(); index++) {
        if (this->keys.at(index)) {
          keyPressed = true;
          this->V.at((opcode & 0x0F00) >> 8) = index;
          break;
        }
      }

      if (!keyPressed) {
        this->PC -= 2;
      }
    } break;

    case 0x0015:
      this->delayTimer = this->V.at((opcode & 0x0F00) >> 8);
      break;

    case 0x0018:
      this->soundTimer = this->V.at((opcode & 0x0F00) >> 8);
      break;

    case 0x001E:
      this->I += this->V.at((opcode & 0x0F00) >> 8);
      break;

    case 0x0029:
      this->I = this->V.at((opcode & 0x0F00) >> 8) * 5;
      break;

    case 0x0033: {
      auto value = this->V.at((opcode & 0x0F00) >> 8);
      for (int i = 3; i > 0; --i) {
        this->memory[this->I + i - 1] = value % 10;
        value /= 10;
      }
    } break;

    case 0x0055:
      for (int offset = 0; offset <= (opcode & 0x0F00) >> 8; offset++)
        this->memory.at(this->I + offset) = this->V.at(offset);

      break;

    case 0x0065:
      for (int offset = 0; offset <= (opcode & 0x0F00) >> 8; offset++) {
        this->V.at(offset) = this->memory.at(this->I + offset);
      }
      break;

    default:
      invalidOpcode(opcode);
    }
  } break;

  default:
    invalidOpcode(opcode);
  }
}

[[nodiscard]] bool Chip8::isPixelOn(uint16_t x, uint16_t y) const {
  x %= 64;
  y %= 32;

  return this->pixels.at((y * 64) + x);
}

void Chip8::togglePixel(uint16_t x, uint16_t y) {
  x %= 64;
  y %= 32;

  // Toggle the pixel boolean
  auto &pixel = this->pixels.at((y * 64) + x);
  pixel = !pixel;

  auto pixelData = &this->pixelBuffer.at((y * 64 + x) * 3);

  if (pixel) {
    // "On" pixels are amber
    pixelData[0] = 0xFF;
    pixelData[1] = 0xBB;
    pixelData[2] = 0x00;
  } else {
    // "Off" pixels are black
    pixelData[0] = 0;
    pixelData[1] = 0;
    pixelData[2] = 0;
  }
}

namespace {
GLuint compileShader(const std::string &path, GLenum type) {
  // Load the file
  auto shaderData = Util::FileReadBinary(path);

  // Add a null terminator because it's a C string
  shaderData.push_back(0);

  const auto shaderChars = reinterpret_cast<const GLchar *>(shaderData.data());
  const auto shader = glCreateShader(type);

  glShaderSource(shader, 1, &shaderChars, nullptr);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    std::array<GLchar, 512> infoOutput;
    glGetShaderInfoLog(shader, static_cast<GLsizei>(infoOutput.size()), nullptr,
                       infoOutput.data());
    glDeleteShader(shader);
    throw std::runtime_error("Failed to compile shader " + path + "\n" +
                             infoOutput.data());
  }

  return shader;
}

GLuint linkShader(GLuint vertexShader, GLuint fragmentShader) {
  const auto program = glCreateProgram();

  glAttachShader(program, vertexShader);
  glAttachShader(program, fragmentShader);
  glLinkProgram(program);
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    std::array<GLchar, 512> infoOutput;
    glGetShaderInfoLog(program, static_cast<GLsizei>(infoOutput.size()),
                       nullptr, infoOutput.data());
    glDeleteProgram(program);
    throw std::runtime_error("Failed to link shaders\n" +
                             std::string(infoOutput.data()));
  }

  return program;
}
} // namespace

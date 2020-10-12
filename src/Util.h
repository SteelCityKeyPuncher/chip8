#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace Util {
std::vector<uint8_t> FileReadBinary(const std::string &path) {
  auto file = std::ifstream(path, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Could not open the ROM.");
  }

  file.seekg(0, std::ios::end);

  std::vector<uint8_t> fileData;
  fileData.resize(static_cast<size_t>(file.tellg()));

  file.seekg(0, std::ios::beg);
  file.read(reinterpret_cast<char *>(&fileData[0]), fileData.size());
  file.close();

  return fileData;
}
} // namespace Util

#endif // UTIL_H_INCLUDED

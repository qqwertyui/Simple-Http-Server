#ifndef UTILS_HPP
#define UTILS_HPP

#include <cstddef>
#include <vector>
#include <memory>
#include <string>

namespace Utils {
  void hex_dump(std::vector<std::byte> &data);
  std::unique_ptr<char[]> __uitoa(unsigned int number);
  std::vector<std::byte> read_file(std::string &path);
  std::vector<std::string> split(std::string &text, char delimiter);
}

#endif
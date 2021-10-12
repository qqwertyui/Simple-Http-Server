#include "Utils.hpp"
#include "HttpException.hpp"
#include <cstdio>
#include <cstring>
#include <fstream>
#include <sstream>

namespace Utils {
  int number_of_digits(int input) {
    int result = 0;
    while(input > 0) {
      input /= 10;
      result += 1;
    }
    return result;
  }

  std::unique_ptr<char[]> __uitoa(unsigned int number) {
    int number_digits = number_of_digits(number);
    std::unique_ptr<char[]> result = std::make_unique<char[]>(number_digits + 2);
    sprintf(result.get(), "%u", number);
    result.get()[number_digits + 1] = 0;
    return result;
  }

  std::vector<std::byte> read_file(std::string &path) {
    std::ifstream file(path, std::ifstream::binary);
    if(file.good() == false) {
      throw HttpException("Unable to read file", HttpException::ErrorLevel::LOW);
    }
    file.seekg(0, std::ifstream::end);
    unsigned int size = file.tellg();
    file.seekg(0, std::ifstream::beg);

    std::vector<std::byte> file_content(size);
    file.read((char*)file_content.data(), file_content.size());
    file.close();
    return file_content;
  }

  std::vector<std::string> split(std::string &text, char delimiter) {
    std::vector<std::string> result;
    std::istringstream iss(text);

    std::string line;
    while(std::getline(iss, line, delimiter)) {
      result.push_back(line);
    }
    return result;
  }
} // namespace


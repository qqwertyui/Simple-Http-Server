#ifndef MIME_HPP
#define MIME_HPP

#include <array>
#include <map>
#include <string>

class Mime {
public:
  static std::string get_type(std::string filepath);

  // look at Mime.cpp for mime types
  static constexpr const char *DEFAULT = "application/octet-stream";
};

#endif
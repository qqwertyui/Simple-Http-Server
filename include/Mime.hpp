#ifndef MIME_HPP
#define MIME_HPP

#include <string>
#include <array>
#include <map>

class Mime {
public:
  static std::string get_type(std::string filepath);

  // look at Mime.cpp for mime types
  static constexpr const char *DEFAULT = "application/octet-stream";
};

#endif
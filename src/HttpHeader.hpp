#ifndef HTTPHEADER_HPP
#define HTTPHEADER_HPP

#include <string>
#include <string_view>
#include <utility>

class HttpHeader {
public:
  HttpHeader(std::string key, std::string value);

  std::string get_key();
  std::string get_value();
  std::string get_all();
  unsigned int get_size();

private:
  static constexpr std::string_view SEPARATOR = ": ";
  enum { KEY = 0, VALUE = 1 };

  std::pair<std::string, std::string> entry;
  std::string header;
};

#endif
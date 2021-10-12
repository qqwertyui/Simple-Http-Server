#include "HttpException.hpp"

HttpException::HttpException(const char *what, ErrorLevel level)
  : msg(what), level(level) {
}

HttpException::ErrorLevel HttpException::get_level() const {
  return this->level;
}

const char* HttpException::what() const noexcept {
  return this->msg;
}
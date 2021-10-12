#ifndef HTTPEXCEPTION_HPP
#define HTTPEXCEPTION_HPP

#include <exception>
#include <string>

class HttpException : public std::exception {
public:
  enum ErrorLevel {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2
  };

  HttpException(const char *what, ErrorLevel level);
  ErrorLevel get_level() const;
  const char *what() const noexcept override;

private:
  const char *msg;
  ErrorLevel level;
};

#endif
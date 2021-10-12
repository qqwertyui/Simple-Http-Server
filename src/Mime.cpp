#include "Mime.hpp"

#include <stdexcept>

std::string Mime::get_type(std::string filepath) {
  static std::map<std::string, std::string> MIMES = {
    { "html", "text/html" },
    { "htm", "text/html" },
    { "css", "text/css" },
    { "gz", "application/gzip" },
    { "json", "application/json" },
    { "jpg", "image/jpeg" },
    { "jpeg", "image/jpeg" },
    { "gif", "image/gif" },
    { "png", "image/png" },
    { "mp3", "audio/mpeg" },
    { "wav", "audio/wav" }
  };

  std::string::size_type pos = filepath.find_last_of(".");
  if(pos == std::string::npos) {
    return Mime::DEFAULT;
  }
  std::string extension = filepath.substr(pos+1);
  std::string result;
  try {
    result = MIMES.at(extension);
  } catch(const std::out_of_range &e) {
    result = Mime::DEFAULT;
  }
  return result;
}

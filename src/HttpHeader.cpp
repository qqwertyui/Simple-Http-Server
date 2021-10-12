#include "HttpHeader.hpp"

HttpHeader::HttpHeader(std::string key, std::string value) {
  this->entry = std::make_pair(key, value);
  this->header.append(key).append(HttpHeader::SEPARATOR).append(value);
}

std::string HttpHeader::get_key() {
  return std::get<HttpHeader::KEY>(this->entry);
}

std::string HttpHeader::get_value() {
  return std::get<HttpHeader::VALUE>(this->entry);
}

std::string HttpHeader::get_all() {
  return this->header;
}

unsigned int HttpHeader::get_size() {
  return this->header.size();
}

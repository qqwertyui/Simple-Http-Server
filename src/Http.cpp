#include "Http.hpp"
#include "Mime.hpp"
#include "Utils.hpp"
#include "HttpException.hpp"
#include "ConnectionHandler.hpp"

#include <linux/limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <thread>

#include <glog/logging.h>

HttpHeader* Base_http_connection::get_header_by_key(std::string key) {
  for(HttpHeader *h : this->headers) {
    if(key.compare(h->get_key()) == 0) {
      return h;
    }
  }
  return nullptr;
}

std::vector<HttpHeader*>& Base_http_connection::get_headers() {
  return this->headers;
}

std::vector<std::byte>& Base_http_connection::get_body() {
  return this->body;
}

void Base_http_connection::add_header(std::string key, std::string value) {
  this->headers.push_back(new HttpHeader(key, value));
}

void Base_http_connection::add_header(HttpHeader *hdr) {
  this->headers.push_back(hdr);
}

Request::Request(int pfd) {
  std::vector<std::byte> raw;

  std::byte buffer;
  int status = 0;
  while((status = recv(pfd, &buffer, sizeof(std::byte), 0)) != 0) {
    if(status == -1) {
      throw HttpException("Socket error / Connection timed out", 
        HttpException::ErrorLevel::MEDIUM);
    }
    raw.push_back(buffer);
    if(Http::is_end(raw) == true) {
      break;
    }
  }
  this->parse(raw);
}

void Request::parse(std::vector<std::byte> &raw) {
  if(raw.size() == 0) {
    throw HttpException("Empty request", HttpException::ErrorLevel::LOW);
  }
  char *begin = (char*)raw.data();
  char *end = strstr(begin, Http::CRLF);
  std::string status(begin, end);

  std::vector<std::string> tokens = Utils::split(status, ' ');
  if(tokens.size() < 3) {
    throw HttpException("Invalid status line", HttpException::ErrorLevel::LOW);
  }
  this->status.method = tokens[0];
  this->status.path = tokens[1];
  this->status.version = tokens[2];
  
  bool valid_method = false;
  for(std::string method : Http::AVALIBLE_METHODS) {
    if(tokens[0].compare(method) == 0) {
      valid_method = true;
      break;
    }
  }
  if(valid_method == false) {
    throw HttpException("Invalid method", HttpException::ErrorLevel::LOW);
  }

  if(tokens[2].size() > 4) {
    std::string prefix = tokens[2].substr(0,5); // 0 -> HTTP/ <- 5
    if(prefix.compare("HTTP/") != 0) {
      throw HttpException("Invalid HTTP version", HttpException::ErrorLevel::LOW);
    }
  }

  char *line_beg = end + std::strlen(Http::CRLF);
  char *line_end = nullptr;
  while((line_end = strstr(line_beg, Http::CRLF)) != nullptr) {
    if(line_beg == line_end) {
      break;
    }
    std::string current(line_beg, line_end);
    current.erase(std::remove_if(current.begin(), current.end(), isspace), current.end());
    std::string::size_type length = current.find_first_of(':');
    if(length == std::string::npos) {
      throw HttpException("Invalid header", HttpException::ErrorLevel::LOW);
    }

    std::string key = current.substr(0, length);
    std::string value = current.substr(length + 1); // skip colon (so +1)
    this->headers.push_back(new HttpHeader(key, value));
    line_beg = line_end + std::strlen(Http::CRLF);
  }
}

std::string Request::get_method() {
  return this->status.method;
}

std::string Request::get_path() {
  return this->status.path;
}

std::string Request::get_version() {
  return this->status.version;
}

std::string Http::sanitize_path(std::string path) {
  std::array<std::string, 2> dangerous = {
    "../",
    "..\\"
  };
  for(std::string current : dangerous) {
    std::string::size_type current_length = current.size();
    std::string::size_type offset = std::string::npos;
    while((offset = path.find(current)) != std::string::npos) {
      VLOG(1) << "Path travelsal detected: [DOCUMENT_ROOT]" << path;
      path.erase(offset, current_length);
    }
  }
  return path;
}

std::string Http::add_root(std::string path) {
  path = sanitize_path(path);
  return std::string(this->document_root).append(path);
}

bool Http::is_end(std::vector<std::byte> &data) {
  constexpr int CRLF_LONG_LENGTH = std::strlen(Http::CRLF_LONG);
  if(data.size() >= CRLF_LONG_LENGTH) {
    char *end = (char*)&data[data.size()-CRLF_LONG_LENGTH];
    if(memcmp(end, Http::CRLF_LONG, CRLF_LONG_LENGTH) == 0) {
      return true;
    }
  }
  return false;
}

bool Http::is_supported_method(std::string method) {
  for(const char *current : Http::SUPPORTED_METHODS) {
    if(method.compare(current) == 0) {
      return true;
    }
  }
  return false;
}

bool Http::is_supported_version(std::string version) {
  return (version.compare(Http::SUPPORTED_VERSION) == 0) ? true : false;
}

void Http::erase_worker(unsigned int fd) {
  for(std::vector<ConnectionHandler*>::iterator i = this->workers.begin(); 
    i!=this->workers.end(); i++) {
    if((*i)->get_fd() == fd) {
      delete *i;
      this->workers.erase(i);
      return;
    }
  }
}

bool Http::resource_exist(std::string path) {
  bool status = false;

  struct stat st;
  memset(&st, 0, sizeof(struct stat));
  stat(path.c_str(), &st);
  // check if file is directory, link or whatever (not regular file)
  if((st.st_mode & S_IFMT) == S_IFREG) {
    std::ifstream file(path, std::ifstream::binary);
    status = (file.good() == true) ? true : false;
    if(status) { 
      file.close(); 
    }
  }
  return status;
}

std::string Http::expand_path(std::string path) {
  char last_character = path[path.size()-1];
  if(last_character == '/') {
    path.append("index.html");
  }
  return path;
}

Response::Response() {
  this->status.version = Http::SUPPORTED_VERSION;
  Response::set_code(Response::Code::INTERNAL_SERVER_ERROR);
}

Response::Response(Status_Code code) {
  this->status.version = Http::SUPPORTED_VERSION;
  Response::set_code(code);
}

std::string Response::get_status() {
  return std::string(this->status.version)
    .append(" ")
    .append(this->status.message);
}

void Response::set_code(const Status_Code &code) {
  this->code = code;
  
  std::string message = std::to_string(std::get<0>(this->code))
    .append(" ")
    .append(std::get<Response::Code::VALUE>(this->code));
  this->status.message = message;
}

void Response::set_body(std::vector<std::byte> &data) {
  this->body = data;
}

void Response::set_body(std::vector<std::byte> &&data) {
  this->body = data;
}

std::vector<std::byte> Response::get_bytearray() {
  constexpr unsigned int CRLF_SIZE = std::strlen(Http::CRLF);
  
  unsigned int total_size = this->get_status().size() + CRLF_SIZE;
  for(HttpHeader *h : this->headers) {
    total_size += h->get_size() + CRLF_SIZE;
  }
  total_size += CRLF_SIZE + body.size();
  std::vector<std::byte> result(total_size);

  // copy status line
  std::byte *ptr = result.data();
  unsigned int length = this->get_status().size();
  memcpy(ptr, this->get_status().c_str(), length);
  memcpy(ptr+length, Http::CRLF, CRLF_SIZE);
  ptr += length + CRLF_SIZE;

  // copy headers
  for(HttpHeader *h : this->headers) {
    length = h->get_size();
    memcpy(ptr, h->get_all().c_str(), length);
    memcpy(ptr+length, Http::CRLF, CRLF_SIZE);
    ptr += length + CRLF_SIZE;
  }
  memcpy(ptr, Http::CRLF, CRLF_SIZE);
  ptr += CRLF_SIZE;
  
  //copy body
  length = this->body.size();
  memcpy(ptr, this->body.data(), length);
  return result;
}

Http::Http(std::string document_root) 
  : document_root(document_root) { 
}

std::vector<std::byte> Http::get_error_page(Status_Code code) {
  std::vector<std::byte> page;
  std::string error_message = std::get<Response::Code::VALUE>(code);
  unsigned int template_size = Http::error_template.size() - 2; // strlen("%s") == 2
  unsigned int error_message_size = error_message.size();
  unsigned int total_size = template_size + error_message_size;
  
  page.resize(total_size);
  snprintf((char*)page.data(), total_size + 1, // + 1 for null character
    Http::error_template.data(), error_message.c_str());
  return page;
}

void Http::send(const int fd, std::vector<std::byte> &bytes) {
  int status = ::send(fd, bytes.data(), bytes.size(), 0);
  if(status == -1) {
    throw HttpException(strerror(errno), HttpException::ErrorLevel::LOW);
  }
}

void Http::accept(Peer_info *pi) {
  ConnectionHandler *ch = new ConnectionHandler(pi, this);
  this->workers.push_back(ch);

  std::thread connection(&ConnectionHandler::main_wrapper, ch);
  connection.detach();
}
#ifndef HTTP_HPP
#define HTTP_HPP

#include <cstdint>
#include <cstddef>
#include <memory>
#include <vector>
#include <array>
#include <string_view>

#include "HttpHeader.hpp"

typedef std::pair<int, const char*> Status_Code;
class ConnectionHandler;
class Peer_info;

class Base_http_connection {
public:
  Base_http_connection() = default;

  void add_header(HttpHeader *hdr);
  void add_header(std::string key, std::string value);
  HttpHeader* get_header_by_key(std::string key);
  std::vector<HttpHeader*>& get_headers();
  std::vector<std::byte>& get_body();

protected:
  std::vector<HttpHeader*> headers;
  std::vector<std::byte> body;
};


class Request : public Base_http_connection {
public:
  Request(int pfd);

  std::string get_method();
  std::string get_path();
  std::string get_version();

private:
  class Entry {
  public:
    std::string method;
    std::string path;
    std::string version;
  } status;

  void parse(std::vector<std::byte> &raw);
};

class Response : public Base_http_connection {
public:
  Response();
  Response(Status_Code code);

  void set_code(const Status_Code &code);
  void set_body(std::vector<std::byte> &data);
  void set_body(std::vector<std::byte> &&data);
  
  std::vector<std::byte>& get_body();
  std::vector<std::byte> get_bytearray();
  std::string get_status();

  class Code {
  public:
    static constexpr unsigned int KEY = 0;
    static constexpr unsigned int VALUE = 1;

    static constexpr Status_Code OK = { 200, "OK" };
    static constexpr Status_Code BAD_REQUEST = { 400, "Bad Request" };
    static constexpr Status_Code NOT_FOUND = { 404, "Not Found" };
    static constexpr Status_Code METHOD_NOT_ALLOWED = { 405, "Method Not Allowed" };
    static constexpr Status_Code INTERNAL_SERVER_ERROR = { 500, "Internal Server Error" };
    static constexpr Status_Code VERSION_NOT_SUPPORTED = { 505, "HTTP Version Not Supported" };
  };

private:
  class Entry {
  public:
    std::string version;
    std::string message;
  } status;

  Status_Code code;
};

class Http { 
public:
  Http(std::string document_root);
  void accept(Peer_info *pi);

  static constexpr std::array SUPPORTED_METHODS = {
    "GET",
    "HEAD"
  };
  static constexpr std::array AVALIBLE_METHODS = {
    "GET",
    "HEAD",
    "POST",
    "PUT",
    "DELETE",
    "CONNECT",
    "OPTIONS",
    "TRACE"
  };
  static constexpr const char *CRLF = "\r\n";
  static constexpr const char *CRLF_LONG = "\r\n\r\n";
  static constexpr const char *SERVER_NAME = "shttps";
  static constexpr const char *SUPPORTED_VERSION = "HTTP/1.1";

  static bool is_end(std::vector<std::byte> &data);

private:
  static bool is_supported_method(std::string method);
  static bool is_supported_version(std::string version);
  static std::string expand_path(std::string path);
  static std::vector<std::byte> get_error_page(Status_Code code);
  static bool resource_exist(std::string path);
  static std::string sanitize_path(std::string path);

  static void send(const int fd, std::vector<std::byte> &bytes);
  
  static constexpr std::string_view error_template = ""
    "<!DOCTYPE html>\n"
    " <html>\n"
    "   <head>\n"
    "     <title>Server error</title>\n"
    "   </head>\n"
    "   <body>\n"
    "     <h1>%s</h1>\n"
    "   </body>\n"
    "</html>\n";

  void process_connection(const int fd);
  std::string add_root(std::string path);
  void respond(const int fd, std::string filename, std::string method, Status_Code code);
  void error(const int fd, Status_Code code, std::string method);

  void erase_worker(unsigned int fd);
  #warning TODO: use std::list instead
  std::vector<ConnectionHandler*> workers;
  std::string document_root;
  friend class ConnectionHandler;
};


#endif
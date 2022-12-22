#include "ConnectionHandler.hpp"
#include "HttpException.hpp"
#include "Mime.hpp"
#include "Utils.hpp"

#include <glog/logging.h>
#include <memory>
#include <utility>

ConnectionHandler::ConnectionHandler(Peer_info *pi, Http *srv)
    : pi(pi), srv(srv) {}

int ConnectionHandler::get_fd() const { return this->pi->get_fd(); }

void ConnectionHandler::main_wrapper() {
  try {
    this->main();
  } catch (const HttpException &e) {
    VLOG(1) << e.what();
  }
  int fd = this->get_fd();
  srv->erase_worker(fd);
}

void ConnectionHandler::main() {
  std::unique_ptr<Request> request = nullptr;
  try {
    request = std::make_unique<Request>(this->get_fd());
  } catch (const HttpException &e) {
    this->error(Response::Code::BAD_REQUEST, "GET");
    return;
  }
  VLOG(0) << request->get_method() << " " << request->get_path();

  Status_Code code = Response::Code::OK;
  if (Http::is_supported_version(request->get_version()) == false) {
    code = Response::Code::VERSION_NOT_SUPPORTED;
  } else if (Http::is_supported_method(request->get_method()) == false) {
    code = Response::Code::METHOD_NOT_ALLOWED;
  }
  if (code != Response::Code::OK) {
    this->error(code, "GET"); // 505
    return;
  }

  std::string file_path = Http::expand_path(srv->add_root(request->get_path()));
  std::string method = request->get_method();
  if (Http::resource_exist(file_path) == false) {
    this->error(Response::Code::NOT_FOUND, method); // 404
    return;
  } else {
    this->respond(file_path, method, Response::Code::OK); // 200
  }
}

void ConnectionHandler::error(Status_Code code, std::string method) {
  this->respond("error.html", method, code); // error.html for mime type
}

void ConnectionHandler::respond(std::string filename, std::string method,
                                Status_Code code) {
  std::unique_ptr<Response> response = std::make_unique<Response>();
  response->set_code(code);

  std::vector<std::byte> body;

  if (code == Response::Code::OK) {
    if (method.compare("GET") == 0) {
      try {
        body = Utils::read_file(filename);
      } catch (const HttpException &e) {
        body = Http::get_error_page(Response::Code::INTERNAL_SERVER_ERROR);
      }
    }
  } else {
    body = Http::get_error_page(code);
  }

  unsigned int body_size = body.size();
  response->set_body(std::move(body));

  response->add_header("Server", Http::SERVER_NAME);
  response->add_header("Content-Type", Mime::get_type(filename));
  response->add_header("Content-Length", std::to_string(body_size));
  response->add_header("Connection", "close");

  std::vector<std::byte> raw_response = response->get_bytearray();
  Http::send(this->get_fd(), raw_response);
}
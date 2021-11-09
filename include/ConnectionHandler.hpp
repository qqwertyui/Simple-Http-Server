#ifndef CONNECTIONHANDLER_HPP
#define CONNECTIONHANDLER_HPP

#include "Http.hpp"
#include "Socket.hpp"
#include <string>
#include <memory>

class ConnectionHandler {
public:
  ConnectionHandler(Peer_info *pi, Http *srv);
  void main_wrapper();
  int get_fd() const;

private:
  void main();
  void terminate();
  void error(Status_Code code, std::string method);
  void respond(std::string filename, std::string method,
               Status_Code code = Response::Code::OK);

  Http *srv = nullptr;
  std::unique_ptr<Peer_info> pi = nullptr;
};

#endif
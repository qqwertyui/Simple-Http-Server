#ifndef CONNECTIONHANDLER_HPP
#define CONNECTIONHANDLER_HPP

#include "Http.hpp"
#include "Socket.hpp"
#include <string>

class ConnectionHandler {
public:
  ConnectionHandler(Peer_info *pi, Http *srv);
  void main_wrapper();
  unsigned int get_fd() const;

private:
  void main();
  void terminate();
  void error(Status_Code code, std::string method);
  void respond(std::string filename, std::string method, Status_Code code = Response::Code::OK);

  Http *srv;
  Peer_info *pi;
};

#endif
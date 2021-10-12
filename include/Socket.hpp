#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <netinet/in.h>

#include <cstdint>
#include <memory>
#include <string>

class Peer_info;

class Socket {
public:
  Socket(std::string interface, const int port, const int timeout);
  void listen();
  Peer_info* accept();

private:
  static in_addr get_interface(std::string name);
  static constexpr int MAX_QUEUE = 5;

  int fd;
  sockaddr_in socket_info;
};

class Peer_info {
public:
  Peer_info(uint32_t fd, uint16_t port, std::string ip);
  ~Peer_info();
  uint32_t get_fd();
  uint16_t get_port();
  std::string get_ip();

private:
  uint32_t fd;
  uint16_t port;
  std::string ip;
};

#endif
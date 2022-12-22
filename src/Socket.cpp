#include "Socket.hpp"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <cstddef>

#include <errno.h>
#include <poll.h>
#include <stdexcept>

#include <glog/logging.h>

Socket::Socket(std::string interface, const int port, const int timeout) {
  this->fd = socket(AF_INET, SOCK_STREAM, 0);
  if (this->fd == -1) {
    throw std::runtime_error(strerror(errno));
  }

  timeval to = {timeout, 0};
  if (setsockopt(this->fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&to, sizeof(to)) ==
      -1) {
    throw std::runtime_error(strerror(errno));
  }
  int reuseaddr = 1;
  if (setsockopt(this->fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr,
                 sizeof(reuseaddr)) == -1) {
    throw std::runtime_error(strerror(errno));
  }

  in_addr iface_addr = {INADDR_ANY};
  if (interface.compare("all") != 0) {
    iface_addr = Socket::get_interface(interface);
  }

  this->socket_info.sin_family = AF_INET;
  this->socket_info.sin_port = htons(port);
  this->socket_info.sin_addr = iface_addr;
  if (bind(this->fd, (const sockaddr *)&this->socket_info, sizeof(sockaddr)) ==
      -1) {
    throw std::runtime_error(strerror(errno));
  }
}

in_addr Socket::get_interface(std::string name) {
  in_addr result;
  ifaddrs *ifap, *tmp;
  int status = getifaddrs(&ifap);
  if (status != 0) {
    throw std::runtime_error(strerror(errno));
  }
  tmp = ifap;
  while (tmp) {
    sockaddr_in *p = (sockaddr_in *)tmp->ifa_addr;
    if (p->sin_family == AF_INET) {
      if (name.compare(tmp->ifa_name) == 0) {
        result = p->sin_addr;
        break;
      }
    }
    tmp = tmp->ifa_next;
  }
  freeifaddrs(ifap);
  return result;
}

void Socket::listen() {
  if (::listen(this->fd, Socket::MAX_QUEUE) == -1) {
    throw std::runtime_error(strerror(errno));
  }
}

Peer_info *Socket::accept() {
  pollfd fds;
  memset(&fds, 0, sizeof(pollfd));
  fds.fd = this->fd;
  fds.events = POLLIN;
  fds.revents = POLLIN;
  if (poll(&fds, (nfds_t)1, -1) < 0) {
    throw std::runtime_error(strerror(errno));
  }

  sockaddr_in tmp;
  memset(&tmp, 0, sizeof(sockaddr_in));
  socklen_t socklen = sizeof(sockaddr);
  int pfd = ::accept(this->fd, (sockaddr *)&tmp, &socklen);
  if (pfd == -1) {
    throw std::runtime_error(strerror(errno));
  }

  return new Peer_info(pfd, htons(tmp.sin_port), inet_ntoa(tmp.sin_addr));
}

Peer_info::Peer_info(uint32_t fd, uint16_t port, const std::string &ip) {
  this->fd = fd;
  this->port = port;
  this->ip = ip;
  VLOG(1) << "New connection from " << this->ip << ":" << this->port;
}

Peer_info::~Peer_info() {
  VLOG(1) << "Closed connection at " << this->ip << ":" << this->port;
  shutdown(this->fd, SHUT_WR);
  std::byte buffer[1024] = {};
  while (recv(this->fd, buffer, 1024, 0) > 0) {
  };
  close(this->fd);
}

uint32_t Peer_info::get_fd() { return this->fd; }

uint16_t Peer_info::get_port() { return this->port; }

std::string Peer_info::get_ip() { return this->ip; }

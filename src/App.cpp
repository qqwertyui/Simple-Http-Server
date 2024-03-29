#include <gflags/gflags.h>
#include <glog/logging.h>
#include <iomanip>
#include <iostream>
#include <pthread.h>
#include <sys/syscall.h>
#include <thread>

#include "Http.hpp"
#include "HttpException.hpp"
#include "Socket.hpp"
#include "Status.hpp"
#include "Version.hpp"

DEFINE_string(document_root, "./", "Document root");
DEFINE_string(interface, "all", "Specify interface to listen on");
DEFINE_int32(port, 7070, "Port on which server is working");
DEFINE_int32(timeout, 5,
             "Time which server can wait if client doesn't respond");

int main(int argc, char **argv) {
  google::SetVersionString(Version::STRING);
  google::SetUsageMessage("./shttps [options]");
  google::ParseCommandLineFlags(&argc, &argv, true);

  google::SetLogDestination(google::INFO, "./logs");

  Socket *s = nullptr;
  try {
    s = new Socket(FLAGS_interface, FLAGS_port, FLAGS_timeout);
  } catch (const std::runtime_error &e) {
    VLOG(0) << "Couldn't create socket: " << e.what();
    return Status::SOCKET_ERROR;
  }

  std::unique_ptr<Http> http = std::make_unique<Http>(FLAGS_document_root);

  s->listen();
  VLOG(1) << "Waiting for incoming connections...";
  while (1) {
    try {
      http->accept(s->accept());
    } catch (const HttpException &e) {
      if (e.get_level() == HttpException::ErrorLevel::HIGH) {
        VLOG(0) << "Terminating due to: " << e.what();
        break;
      } else {
        VLOG(0) << "Error: " << e.what();
      }
    }
  }
  return Status::OK;
}

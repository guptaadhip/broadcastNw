#include "include/HostInterface.h"
#include "include/Logger.h"
#include "include/Host.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <unordered_map>

char HOST_PATH[] = "/tmp/hostSocket";

HostInterface::HostInterface(Host *myHost) {
  host_ = myHost;
}

void HostInterface::readSocket() {
  char command[BUFLEN];
  struct sockaddr_un local, remote;
  socklen_t remoteLen;
  bzero(&local, sizeof(local));
  local.sun_family = AF_UNIX;
  strncpy(local.sun_path, HOST_PATH, 104);
  unlink(HOST_PATH);
  if ((socket_ = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    Logger::log(Log::WARN, __FILE__, __FUNCTION__, __LINE__,
                "Unable to create the HostInterface Socket");
    close(socket_);
    close(cliSocket_);
    return;
  }
  Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__,
                "Listening at path: " + std::string(HOST_PATH));
  if (bind(socket_, (struct sockaddr *) &local, sizeof(local)) == -1) {
    Logger::log(Log::WARN, __FILE__, __FUNCTION__, __LINE__,
                      "HostInterface Socket bind failed: " 
                      + std::string(local.sun_path));
    close(socket_);
    close(cliSocket_);
    return;
  }
  /* only one external interface can connect */
  if (listen(socket_, 5) == -1) {
    Logger::log(Log::WARN, __FILE__, __FUNCTION__, __LINE__,
                "HostInterface Socket listen failed");
    close(socket_);
    close(cliSocket_);
    return;
  }
  while (true) {
    PacketEntry pkt;
    bool done = false;
    int rc;
    remoteLen = sizeof(remote);
    if ((cliSocket_ = accept(socket_, (struct sockaddr *)&remote, 
                                                          &remoteLen)) == -1) {
      Logger::log(Log::WARN, __FILE__, __FUNCTION__, __LINE__,
                  "HostInterface Socket accept failed");
      close(socket_);
      close(cliSocket_);
      return;
    }
    while (!done) {
      bzero(command, sizeof(command));
      rc = recv(cliSocket_, command, 1440, 0);
      if (rc <= 0) {
        continue;
      }
      if (strcmp(command, "quit") == 0) {
        done = true;
        Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__,
                    "quiting");
      } else if (strncmp(command, "g", 1) == 0) {
        bzero(pkt.packet, BUFLEN);
        unsigned int numberOfPackets = 0;
        bcopy(command + sizeof(char), numberOfPackets, sizeof(unsigned int));
        bcopy(command + sizeof(char) + sizeof(unsigned int), pkt.packet, BUFLEN);
        pkt.len = BUFLEN;
        Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__,
                    "Sending packet " + numberOfPackets + "times");
        for(int i = 0; i < numberOfPackets; i++) {
        	host_->send(pkt.packet, BUFLEN);
        }
        Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__,
                    "Full Data sent");
      }
    }
    close(cliSocket_);
  }
}

HostInterface::~HostInterface() {
  close(socket_);
  close(cliSocket_);
}

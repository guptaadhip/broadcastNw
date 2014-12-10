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
    char data[BUFLEN];
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
        bzero(data, BUFLEN);
        unsigned int numberOfPackets = 0, lastPacketSize = 0;;
        bcopy(command + sizeof(char), &numberOfPackets, sizeof(unsigned int));
        bcopy(command + sizeof(char) + sizeof(unsigned int), &lastPacketSize, sizeof(unsigned int));
        bcopy(command + sizeof(char) + (2 * sizeof(unsigned int)), data, BUFLEN);
        for(unsigned int i = 0; i < numberOfPackets; i++) {
        	host_->send(data, BUFLEN);
        }
        /* Sending last packet now */
        host_->send(data, lastPacketSize);
        Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__,
                    "data sent");
      } else if (strcmp(command, "show packet received counter") == 0) {
        bzero(data, sizeof(data));
        sprintf(data, "%u", host_->packetsReceived_); 
        sendData(data, strlen(data)); 
      } else if (strcmp(command, "clear packet received counter") == 0) {
        host_->packetsReceived_ = 0;
      }
    }
    close(cliSocket_);
  }
}

void HostInterface::sendData(char *data, int len) {
  auto rc = send(cliSocket_, data, len, 0);
  if (rc < 0) {
    Logger::log(Log::WARN, __FILE__, __FUNCTION__, __LINE__,
                "Unable to send data over the socket");
  }
}

HostInterface::~HostInterface() {
  close(socket_);
  close(cliSocket_);
}

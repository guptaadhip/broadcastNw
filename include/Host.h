#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include "PacketEngine.h"
#include <string>
#include "PacketHandler.h"

/* Build Forward IP Packet */
class Host {
 public:
  Host(unsigned int myId,std::string interface);
  void send(const char *data);

 private:
  unsigned int myId_;
  PacketHandler packetHandler_;
  PacketEngine *packetEngine_;
  std::string interface_;
};

#pragma once
#include "include/net.h"
#include "include/PacketHandler.h"

/* Build Forward IP Packet */
class PacketEngine {
 public:
  PacketEngine();
  void forward(const char *packet);
  void receive();
  
 private:
  void initializeEngine();
  unsigned short checksum(unsigned short *addr, int len);
  PacketHandler *packetHandler_;
  int socketFd_;
  unsigned int interfaceIdx_;
};

#pragma once
#include "include/net.h"
#include "include/PacketHandler.h"
#include <string>

/* Build Forward IP Packet */
class PacketEngine {
 public:
  PacketEngine(std::string interface, PacketHandler *packetHandler);
  void forward(char *packet, int size);
  void receive();
  
 private:
  void initializeEngine();
  unsigned short checksum(unsigned short *addr, int len);
  PacketHandler *packetHandler_;
  int socketFd_;
  unsigned int interfaceIdx_;
  std::string interface_;
};

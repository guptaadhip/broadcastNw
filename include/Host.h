#pragma once
#include <arpa/inet.h>
#include <netinet/in.h>
#include "PacketEngine.h"
#include "PacketHandler.h"
#include "Queue.h"
#include <string>

/* Build Forward IP Packet */
class Host {
 public:
  Host(unsigned int myId,std::string interface);
  void send(const char *data);
  void startSniffing(PacketEngine *packetEngine);
  void handlePacket();

 private:
  unsigned int myId_;
  PacketHandler packetHandler_;
  PacketEngine *packetEngine_;
  std::string interface_;
  Queue hostQueue_;
};

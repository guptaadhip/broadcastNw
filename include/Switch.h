#pragma once
#include "include/PacketHandler.h"
#include "include/PacketEngine.h"
#include "include/MyInterface.h"
#include "include/Queue.h"
#include <unordered_map>

class Switch {
 public:
  Switch(unsigned int myId);
  /*
   * Start receiving from interfaces
   */
  void startSniffing(PacketEngine *packetEngine);
  void handlePacket();

 private:
  PacketHandler packetHandler_;
  MyInterface myInterface_;
  unsigned int myId_;
  std::unordered_map<std::string, PacketEngine> ifToPacketEngine_;
  Queue switchQueue_;
};

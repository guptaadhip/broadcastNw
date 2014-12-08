#include "include/PacketHandler.h"
#include "include/MyInterface.h"

class Switch {
 public:
  Switch(unsigned int myId);
 private:
  PacketHandler packetHandler_;
  MyInterface myInterface_;
  unsigned int myId_;
};

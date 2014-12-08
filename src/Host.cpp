#include <iostream>
#include <sys/types.h>
#include <ifaddrs.h>
#include <thread>
#include <string.h>
#include "include/Host.h"
#include "include/Logger.h"

#define BUFLEN 1500
 
Host::Host(unsigned int myId, std::string interface) {
  myId_ = myId;
  interface_= interface;
  packetEngine_ = new PacketEngine(interface, &packetHandler_);
}

void Host::send(const char *data) {
  char packet[BUFLEN];
  bzero(packet, BUFLEN);
  char str[50]="Send the stupid packet";
  bcopy(str, packet + HEADER_LEN, BUFLEN - HEADER_LEN);
  packetEngine_->forward(packet, BUFLEN);
  Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__,
              "Packet sent");
}

#include <iostream>
#include <sys/types.h>
#include <ifaddrs.h>
#include <thread>
#include <string.h>
#include "include/Host.h"
#include "include/HostInterface.h"
#include "include/Logger.h"

#define BUFLEN 1500
 
Host::Host(unsigned int myId, std::string interface) {
  myId_ = myId;
  interface_= interface;
  packetEngine_ = new PacketEngine(interface, &packetHandler_);
  /*  
   * Create the host exteral interface object
   */
  HostInterface hostInterface(this);
  std::thread hostInterfaceThread = std::thread(
                        &HostInterface::readSocket, hostInterface);
  auto packetThread = std::thread(&Host::handlePacket, this);
  auto sniffThread = std::thread(&Host::startSniffing, this, packetEngine_);
  packetHandler_.processQueue(&hostQueue_);
}

void Host::handlePacket() {
  /* first one needs to be removed */
  (void) hostQueue_.packet_in_queue_.exchange(0, std::memory_order_consume);
  while(true) {
    auto pending = hostQueue_.packet_in_queue_.exchange(0, 
                                                    std::memory_order_consume);
    if( !pending ) { 
      std::unique_lock<std::mutex> lock (hostQueue_.packet_ready_mutex_); 
      if( !hostQueue_.packet_in_queue_) {
        hostQueue_.packet_ready_.wait(lock);
      }
      continue;
    }
    Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__, 
                  "received a packet");
  }
}

void Host::send(const char *data, unsigned int len) {
  char packet[BUFLEN];
  bzero(packet, BUFLEN);
  char str[50]="Send the stupid packet";
  bcopy(str, packet + HEADER_LEN, BUFLEN - HEADER_LEN);
  packetEngine_->forward(packet, len);
  Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__,
              "Packet sent");
}

/* Effectively the packet engine thread */
void Host::startSniffing(PacketEngine *packetEngine) {
    packetEngine->receive();
}

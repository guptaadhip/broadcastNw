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
  int packetCounter = 0;
  while(true) {
    std::unique_lock<std::mutex> lock(hostQueue_.packet_ready_mutex_);    
    hostQueue_.packet_ready_.wait(lock);

    while (!hostQueue_.packet_in_queue_.empty()) {
      packetCounter++;
      //auto pending = hostQueue_.packet_in_queue_.front();
      hostQueue_.packet_in_queue_.pop();
      /*char packet[BUFLEN];
      bzero(packet, BUFLEN);
      bcopy(pending->packet + HEADER_LEN, packet, 1440);*/
      Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__, 
                  "received a packet: " + std::to_string(packetCounter));
    }
  }
}

void Host::send(const char *data, unsigned int len) {
  char packet[BUFLEN];
  bzero(packet, BUFLEN);
  bcopy(data, packet + HEADER_LEN, strlen(data));
  packetEngine_->forward(packet, strlen(data) + HEADER_LEN);
  /*Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__,
              "Packet sent");*/
}

/* Effectively the packet engine thread */
void Host::startSniffing(PacketEngine *packetEngine) {
    packetEngine->receive();
}

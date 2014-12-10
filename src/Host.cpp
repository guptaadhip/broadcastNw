#include <iostream>
#include <sys/types.h>
#include <ifaddrs.h>
#include <thread>
#include <string.h>
#include "include/Host.h"
#include "include/HostInterface.h"
#include "boost/date_time/posix_time/posix_time.hpp"
#include "include/Logger.h"


#define BUFLEN 1500
namespace pt = boost::posix_time;

Host::Host(unsigned int myId, std::string interface) {
  myId_ = myId;
  interface_= interface;
  dataSize_ = 0;
  packetsReceived_ = 0;
  packetEngine_ = new PacketEngine(interface, &packetHandler_);
  /*  
   * Create the host exteral interface object
   */
  HostInterface hostInterface(this);
  std::thread hostInterfaceThread = std::thread(
                        &HostInterface::readSocket, hostInterface);
  auto packetThread = std::thread(&Host::handlePacket, this);
  auto sniffThread = std::thread(&Host::startSniffing, this, packetEngine_);
  auto throughputThread = std::thread(&Host::throughput, this);
  packetHandler_.processQueue(&hostQueue_);
}

void Host::handlePacket() {
  while(true) {
    std::unique_lock<std::mutex> lock(hostQueue_.packet_ready_mutex_);    
    hostQueue_.packet_ready_.wait(lock);

    while (!hostQueue_.packet_in_queue_.empty()) {
      packetsReceived_++;
      auto pending = hostQueue_.packet_in_queue_.front();
      hostQueue_.packet_in_queue_.pop();
      dataSize_ += pending->len;
      /*char packet[BUFLEN];
      bzero(packet, BUFLEN);
      bcopy(pending->packet + HEADER_LEN, packet, 1440);
      Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__, 
                  "received a packet: " + std::to_string(packetCounter));*/
    }
  }
}

void Host::throughput() {
  while (true) {
    pt::ptime time_start(pt::microsec_clock::local_time());
    dataSize_ = 0;
    sleep(1);
    unsigned int data = dataSize_;
    pt::ptime time_end(pt::microsec_clock::local_time());
    pt::time_duration duration(time_end - time_start);
    auto throughput = (data / duration.total_microseconds()) * 8;
    if (dataSize_ != 0) {
      Logger::log(Log::INFO, __FILE__, __FUNCTION__, __LINE__,
              "Throughput (mbps): " + std::to_string(throughput));
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

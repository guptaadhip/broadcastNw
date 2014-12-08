#include "include/Switch.h"
#include "include/Logger.h"
#include <thread>
#include <algorithm>

Switch::Switch(unsigned int myId) {
  myId_ = myId;
  std::vector<std::thread> packetEngineThreads;
  for (auto interface : myInterface_.getInterfaces()) {
    PacketEngine packetEngine(interface, &packetHandler_);
    std::pair<std::string, PacketEngine> ifPePair (interface, packetEngine); 
    ifToPacketEngine_.insert(ifPePair);
    Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__, interface);
  }

  /* making packetEngine threads for all interfaces */
  for(auto it = ifToPacketEngine_.begin(); it != ifToPacketEngine_.end(); it++) {
    packetEngineThreads.push_back(std::thread(&Switch::startSniffing, this,
                                  &it->second));
  }
  auto packetThread = std::thread(&Switch::handlePacket, this);
  packetHandler_.processQueue(&switchQueue_);
}

void Switch::handlePacket() {
  /* first one needs to be removed */
  (void) switchQueue_.packet_in_queue_.exchange(0, std::memory_order_consume);
  while(true) {
    auto pending = switchQueue_.packet_in_queue_.exchange(0, 
                                                    std::memory_order_consume);
    if( !pending ) { 
      std::unique_lock<std::mutex> lock (switchQueue_.packet_ready_mutex_); 
      if( !switchQueue_.packet_in_queue_) {
        switchQueue_.packet_ready_.wait(lock);
      }
      continue;
    }
    Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__, 
                  "got a packet to forward from:" + pending->interface);
    for (auto entry : ifToPacketEngine_) {
      if (entry.first.compare(pending->interface) == 0) {
        continue;
      }
      Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__, 
                  "forwarding to interface: " + entry.first);
      entry.second.forward(pending->packet, pending->len);
    }
  }
}

/* Effectively the packet engine thread */
void Switch::startSniffing(PacketEngine *packetEngine) {
  packetEngine->receive();
}

#include "include/PacketHandler.h"
#include "include/Logger.h"
#include "include/Queue.h"
#include <cstring>

void PacketHandler::queuePacket(PacketEntry *t) {
  packet_in_queue_.push(t);
  packet_ready_.notify_one();
}

void PacketHandler::processQueue(Queue *switchQueue) {
  /* first one needs to be removed */
  while(true) {
    std::unique_lock<std::mutex> lock(packet_ready_mutex_);                
    packet_ready_.wait(lock);

    while (!packet_in_queue_.empty()) {
      auto pending = packet_in_queue_.front();
      packet_in_queue_.pop();
      switchQueue->queuePacket(pending);
    }
  }
}

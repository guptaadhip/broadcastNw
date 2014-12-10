#include "include/PacketEngine.h"
#include "include/Logger.h"
#include "include/net.h"
#include <net/ethernet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <cstring>
#include <iostream>

using namespace std;

unsigned int bufSize = 33554432;

PacketEngine::PacketEngine(std::string interface, PacketHandler *packetHandler){
  const int on = 1;
  interface_ = interface;
  packetHandler_ = packetHandler;

  /* creating sending socket */
  socketFd_ = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (socketFd_ < 0) {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__, 
                "PacketEngine : Error in opening sending socket");
  }

  if (setsockopt(socketFd_, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__, 
                "PacketEngine : Error setting socket option");
  }

  if (setsockopt(socketFd_, SOL_SOCKET, SO_RCVBUF, &bufSize, sizeof(int)) == -1) {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__, 
                "PacketEngine : Error setting socket option");

  }

  initializeEngine(); 
}

void PacketEngine::initializeEngine() {
  /* Finding the index of the interface */
  char *if_name = (char *) interface_.c_str();
  struct ifreq ifr;
  size_t if_name_len = strlen(if_name);
  if (if_name_len < sizeof(ifr.ifr_name)) {
    memcpy(ifr.ifr_name,if_name,if_name_len);
    ifr.ifr_name[if_name_len]=0;
  } else {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__, 
                "PacketEngine : interface name is too long");
  }
  if (ioctl(socketFd_, SIOCGIFINDEX, &ifr)==-1) {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__,
                "Packet Engine: Error getting Interface Index");
  }
  interfaceIdx_ = ifr.ifr_ifindex;

  /* attempt to receive */
  struct sockaddr_ll raddrll;
  
  /* setting raddrll values */
  raddrll.sll_family = PF_PACKET;
  raddrll.sll_ifindex = interfaceIdx_;
  raddrll.sll_protocol = htons(ETH_P_ALL);
  char dest[8];
  bzero((char *) dest, sizeof(dest));
  memcpy((void*)(raddrll.sll_addr), (void*)dest, 8);
  
  /* Binding the receiving socket to the interafce if_name */
  if (bind(socketFd_, (struct sockaddr *) &raddrll, sizeof(raddrll)) < 0) {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__,
                "PacketEngine: Error binding to socket");
  }
  
  /* Get the current flags that the device might have */
  if (ioctl (socketFd_, SIOCGIFFLAGS, &ifr) == -1) {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__,
          "Error: Could not retrive the flags from the device.");
  }
  
  /* Set the old flags plus the IFF_PROMISC flag */
  ifr.ifr_flags |= IFF_PROMISC;
  if (ioctl (socketFd_, SIOCSIFFLAGS, &ifr) == -1) {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__,
                "Error: Could not set flag IFF_PROMISC");
  }
}

void PacketEngine::forward(char *packet, int size) {
  struct sockaddr_ll saddrll;
  memset((void*)&saddrll, 0, sizeof(saddrll));
  saddrll.sll_family = AF_PACKET;

  /* filling the interface index */
  saddrll.sll_ifindex = interfaceIdx_;
  saddrll.sll_halen = 2;
  
  char dest[8];
  bzero((char *) dest, sizeof(dest));
  memcpy((void*)(saddrll.sll_addr), (void*)dest, 8);
 
  if (sendto(socketFd_, packet, size, 0,
             (struct sockaddr*)&saddrll, sizeof(saddrll)) < 0) {
    Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__,
                "Packet Engine: Error sending Packet");
  }
}

void PacketEngine::receive() {
  char packet[BUFLEN];
  struct sockaddr_ll saddrll;
  socklen_t senderAddrLen;
  int rc;

  /* 
   * zeroing the sender's address struct.
   * It will be filled by the recvfrom function.
   */
  bzero(packet, BUFLEN);
  memset((void*)&saddrll, 0, sizeof(saddrll));
  senderAddrLen = (socklen_t) sizeof(saddrll);
  
  while (true) {
    
    PacketEntry *packetEntry = new PacketEntry;
    packetEntry->interface = interface_;
    /* Start receiving the data */
    rc = recvfrom(socketFd_, packetEntry->packet, BUFLEN, 0,
                          (struct sockaddr *) &saddrll, &senderAddrLen);
    
    if (rc < 0 || saddrll.sll_pkttype == PACKET_OUTGOING) {
      continue;
    }
    packetEntry->len = rc;
    packetHandler_->queuePacket(packetEntry);
  }
}

unsigned short PacketEngine::checksum(unsigned short *addr, int len){ 
  int nleft = len;
  int sum = 0;
  unsigned short *w = addr;
  unsigned short answer = 0;
  
  while (nleft > 1) {
    sum += *w++;
    nleft -= 2;
  }
  
  if (nleft == 1) {
    *(unsigned char *) (&answer) = *(unsigned char *) w;
    sum += answer;
  }
  
  sum = (sum >> 16) + (sum & 0xFFFF);
  sum += (sum >> 16);
  answer = ~sum;
  return (answer);
}

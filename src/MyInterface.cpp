#include "include/MyInterface.h"
#include "include/net.h"
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <algorithm>

/* to be removed */
#include <iostream>

using namespace std;

MyInterface::MyInterface() {
  /* Find all of my interfaces */
  init();
}

void MyInterface::init() {
  struct ifaddrs * ifa = NULL;
  getifaddrs(&ifAddrStruct_);

  for (ifa = ifAddrStruct_; ifa != NULL; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
      uint32_t ipAddr = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr.s_addr;
      if (ipAddr != LOOPBACK_IP && ((ipAddr & 0x0000ffff) != CONTROL_NW_IP)) {
      //if ((ipAddr & 0x0000ffff) != CONTROL_NW_IP) {
      //if (ipAddr != LOOPBACK_IP) { // This is for the local systems only
        std::string interface = string(ifa->ifa_name);
        if (std::find(interfaceList_.begin(), interfaceList_.end(), 
                                 interface) == interfaceList_.end()) {
          interfaceList_.push_back(interface);
        }
      }
    }
  }
}

std::vector<std::string> MyInterface::getInterfaces() {
  return interfaceList_; 
}

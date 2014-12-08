#pragma once
#include <vector>
#include <string>
#include <ifaddrs.h>

class MyInterface {
 public:
  MyInterface();
  std::vector<std::string> getInterfaces();

 private:
  void init();
  std::vector<std::string> interfaceList_;
  struct ifaddrs *ifAddrStruct_= NULL;
};

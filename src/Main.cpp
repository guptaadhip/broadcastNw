#include "include/MyInterface.h"
#include <iostream>

int main() {
  MyInterface myInterface;
  for (auto intf : myInterface.getInterfaces()) {
    std::cout << intf << std::endl;
  }
  return 0;
}

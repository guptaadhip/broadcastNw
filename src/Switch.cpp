#include "include/Switch.h"
#include "include/Logger.h"

Switch::Switch(unsigned int myId) {
  myId_ = myId;
  for (auto intf : myInterface_.getInterfaces()) {
    Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__, intf);
  }
}

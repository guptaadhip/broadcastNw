#include "include/Switch.h"
#include "include/Host.h"
#include "include/Logger.h"
#include <string.h>
#include <iostream>

int main(int argc, char *argv[]) {
  unsigned int myId;
  std::string role;
  if (argc < 3) {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__, 
                "too few arguments");
  } else if (argc == 5) {
    if (strncmp(argv[4], "dbg", strlen(argv[4])) == 0) {
      DEBUG_MODE = true;
    }
  } else if (argc > 7) {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__, 
                "too many arguments");
  }
  myId = atoi(argv[1]);
  role = argv[2];
  if (role.compare("Switch") == 0) {
    Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__, 
                "Role: Switch");
    Switch mySwitch(myId);
  } else if (role.compare("Host") == 0) {
    Logger::log(Log::DEBUG, __FILE__, __FUNCTION__, __LINE__, "Role: Host");
    Host host(myId, std::string(argv[3]));
    if (strncmp(argv[6], "send", strlen(argv[6])) == 0) {
      char data[10];
      host.send(data);
    }
  } else {
    Logger::log(Log::CRITICAL, __FILE__, __FUNCTION__, __LINE__, 
                "incorrect role specified");
  }
  return 0;
}

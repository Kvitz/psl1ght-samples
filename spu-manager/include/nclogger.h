#pragma once

#include <sstream>
#include <string>

#include <net/net.h>
#include <netinet/in.h>

class NCLogger {
public:
  NCLogger(const char* ip, const unsigned short port) {
    netInitialize();
    struct sockaddr_in stSockAddr;
    SocketFD = netSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    memset(&stSockAddr, 0, sizeof stSockAddr);

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &stSockAddr.sin_addr);

    netConnect(SocketFD, (struct sockaddr *)&stSockAddr, sizeof stSockAddr);
  }

  ~NCLogger() {
    netDeinitialize();
  }

  template<typename T>
  std::ostream& operator<< (T val) {
    return ss << val;
  }

  void send() {
      netSend(SocketFD, ss.str().c_str(), ss.str().size(), 0);
      ss.str("");
  }

  inline void send(const std::string &s) {
    netSend(SocketFD, s.c_str(), s.size(), 0);
  }

private:
  int SocketFD;
  std::stringstream ss;
};

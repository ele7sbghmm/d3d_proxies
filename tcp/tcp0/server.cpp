#include "pch.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")

namespace server {
  void RunTCPServer() {
    WSADATA wsa;
    (void)WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9000);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (sockaddr*)&addr, sizeof(addr));
    listen(server, 1);

    while (true) {
      SOCKET client = accept(server, nullptr, nullptr);

      char buf[1024];
      int len = recv(client, buf, sizeof(buf), 0);
      send(client, buf, len, 0);

      closesocket(client);
    }
  }
}
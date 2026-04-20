#include "pch.h"

#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#include <d3d8.h>

#include "server.h"

namespace server {
  SOCKET client = INVALID_SOCKET;

  void Run() {
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
      client = accept(server, nullptr, nullptr);

      char buf[1024];
      while (recv(client, buf, sizeof(buf), 0) > 0) { }

      closesocket(client);
      client = INVALID_SOCKET;
    }
  }

  void Send_DoPostDynaLoad() {
    if (client == INVALID_SOCKET) return;
    send(client, "DoPostDynaLoad: ", 17, 0);
  }
}
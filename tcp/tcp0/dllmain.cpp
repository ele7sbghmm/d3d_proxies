// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "proxy.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

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

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    std::thread(RunTCPServer).detach();
    proxy::init();
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    if (proxy::hModule)
      FreeLibrary(proxy::hModule);
    break;
  }
  return TRUE;
}


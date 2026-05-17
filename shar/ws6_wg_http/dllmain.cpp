// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <WinSock2.h>
#include <string>
#include <thread>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#pragma comment(linker, "/export:Direct3DCreate8=_Direct3DCreate8@4")

#include "index_js.h"
#include <d3d8.h>

namespace srv {
void handle_client(SOCKET cs) {
  char buf[4096]{};
  recv(cs, buf, sizeof(buf), 0);

  if (strstr(buf, "GET /index.js")) {

    std::string res = "HTTP/1.1 200 OK"
                      "Content-Type: application/javascript\r\n"
                      "Content-Length: " +
                      std::to_string(INDEX_JS_LEN) +
                      "\r\n"
                      "Connection: close\r\n\r\n" +
                      std::string((char *)INDEX_JS, INDEX_JS_LEN);
    send(cs, res.c_str(), res.size(), 0);
  } else {
    std::string html = "<!DOCTYPE "
                       "html><html><head><title>foobarbaz</title></"
                       "head><body><h1>foobarbaz</h1><script "
                       "src=\"./index.js\"></script></body></html>";
    std::string http = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: " +
                       std::to_string(html.size()) +
                       "\r\n"
                       "Connection: close\r\n\r\n" +
                       html;
    send(cs, http.c_str(), (int)http.size(), 0);
  }
}
void loop() {
  WSADATA wsa{};
  (void)WSAStartup(MAKEWORD(2, 2), &wsa);

  SOCKET ss = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(9001);
  addr.sin_addr.s_addr = INADDR_ANY;

  bind(ss, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  listen(ss, 100);

  while (true) {
    SOCKET cs = accept(ss, nullptr, nullptr);
    handle_client(cs);
  }

  WSACleanup();
}
} // namespace srv

IDirect3D8 *__stdcall Direct3DCreate8(UINT SDKVersion) {
  using Direct3DCreate8_t = IDirect3D8 *(__stdcall *)(UINT);
  static Direct3DCreate8_t oDirect3DCreate8 = nullptr;

  if (!oDirect3DCreate8) {
    char path[MAX_PATH]{};
    GetSystemDirectoryA(path, MAX_PATH);
    strcat_s(path, "\\d3d8.dll");
    HMODULE hModule = LoadLibraryA(path);
    if (!hModule)
      throw;
    oDirect3DCreate8 =
        (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");

    std::thread(srv::loop).detach();
  }

  return oDirect3DCreate8(SDKVersion);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call,
                      LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

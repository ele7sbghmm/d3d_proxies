// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <thread>

#include "proxy.h"
#include "server.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    std::thread(server::RunTCPServer).detach();
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


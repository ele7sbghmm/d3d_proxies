// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <thread>

#include "proxy.h"
#include "server.h"
#include "hooks.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    std::thread(server::Run).detach();

    MH_Initialize();
    proxy::init();
    hooks::install_game_hooks();

    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}


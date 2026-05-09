// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <iostream>
#include <d3d9.h>

IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {
  using Direct3DCreate9_t = IDirect3D9*(WINAPI*)(UINT);
  static Direct3DCreate9_t oDirect3DCreate9 = nullptr;

  if (!oDirect3DCreate9) {
    char path[MAX_PATH];
    GetSystemDirectoryA(path, MAX_PATH);
    strcat_s(path, "\\d3d9.dll");
    HMODULE hModule = LoadLibraryA(path);
    if (!hModule) return nullptr;

    oDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(hModule, "Direct3DCreate9");
  }

  IDirect3D9* d3d = oDirect3DCreate9(SDKVersion);
  if (!d3d) return nullptr;

  return d3d;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
      break;
  }
  return TRUE;
}


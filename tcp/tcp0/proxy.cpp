#include "pch.h"

#include <iostream>

#include "proxy.h"
#include "hooks.h"

#include <d3d8.h>

namespace proxy {
  using Direct3DCreate8_t = IDirect3D8*(WINAPI*)(UINT);
  Direct3DCreate8_t oDirect3DCreate8 = nullptr;

  void init() {
    char path[MAX_PATH];
    GetSystemDirectoryA(path, MAX_PATH);
    strcat_s(path, "\\d3d8.dll");
    HMODULE hModule = LoadLibraryA(path);

    oDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");
  }
}

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
  proxy::init();
  IDirect3D8* d3d = proxy::oDirect3DCreate8(SDKVersion);
  hooks::install_d3d_hooks(d3d);
  return d3d;
}
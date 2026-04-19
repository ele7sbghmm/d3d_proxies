#include "pch.h"

#include "proxy.h"

namespace proxy {
  HMODULE hModule = nullptr;

  using Direct3DCreate8_t = IDirect3D8*(WINAPI*)(UINT);
  Direct3DCreate8_t oDirect3DCreate8 = nullptr;

  void init() {
    TCHAR path[MAX_PATH];
    GetSystemDirectory(path, MAX_PATH);
    strcat_s(path, "\\d3d8.dll");
    hModule = LoadLibrary(path);
    oDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");
    //assert(oDirect3DCreate8 && "failed to find Direct3DCreate8");b 
  }
}

IDirect3D8* WINAPI hDirect3DCreate8(UINT SDKVersion) {
  return proxy::oDirect3DCreate8(SDKVersion);
}

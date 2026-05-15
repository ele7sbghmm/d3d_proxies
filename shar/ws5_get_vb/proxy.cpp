#include "pch.h"

#include <thread>
#include "hooks.h"
#include "server.cpp"

void init_hooks(IDirect3D8 *) { std::thread(sv::run).detach(); }

IDirect3D8 *__stdcall Direct3DCreate8(UINT SDKVersion) {
  using t = decltype(&Direct3DCreate8);
  static t o = nullptr;

  if (!o) {
    char p[MAX_PATH];
    GetSystemDirectory(p, MAX_PATH);
    strcat_s(p, "\\d3d8.dll");
    HMODULE h = LoadLibraryA(p);
    if (!h)
      return nullptr;
    o = (t)GetProcAddress(h, "Direct3DCreate8");
  }

  IDirect3D8 *d = o(SDKVersion);

  init_hooks(d);

  return d;
}

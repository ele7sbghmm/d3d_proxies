#include "pch.h"

#include "hooks.hpp"
#include "proxy.hpp"
#include "context.hpp"

extern Context g_context;

namespace hooks::d3d {
  using CreateDevice_t = HRESULT(WINAPI*)(IDirect3D8*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,IDirect3DDevice8**);
  CreateDevice_t oCreateDevice = nullptr;

  HRESULT WINAPI hCreateDevice(IDirect3D8* d3d, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
    HRESULT hr = oCreateDevice(d3d, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    g_context = { *ppReturnedDeviceInterface };

    return hr;
  }

  void install(IDirect3D8* d3d) {
    void** vftable = *(void***)d3d;

    MH_CreateHook(vftable[15], hCreateDevice, (void**)&oCreateDevice);
    MH_EnableHook(vftable[15]);
  }
}

namespace proxy {
  using Direct3DCreate8_t = IDirect3D8*(WINAPI*)(UINT);
  Direct3DCreate8_t oDirect3DCreate8 = nullptr;

  void init() {
    char path[MAX_PATH];
    GetSystemDirectory(path, MAX_PATH);
    strcat_s(path, "\\d3d8.dll");
    HMODULE hModule = LoadLibraryA(path);
    if (!hModule) throw;
    
    oDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");
  }
}

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
  proxy::init();
  IDirect3D8* d3d = proxy::oDirect3DCreate8(SDKVersion);

  MH_Initialize();
  hooks::d3d::install(d3d);
  hooks::install_all();
  return d3d;
}

#include "pch.h"

#include "hooks.h"
#include "server.h"

#include <MinHook.h>
#pragma comment(lib, "libMinHook.x86.lib")


#define DECLARE_HOOK(target,name)                                        \
  namespace name {                                                       \
    void* trampoline = nullptr;                                          \
    void detour();                                                       \
    void install() {                                                     \
      MH_CreateHook((void*)target, detour, (void**)&trampoline);  \
      MH_EnableHook((void*)target);                                      \
    }                                                                    \
  }

namespace hooks {
  namespace d3d {
    using CreateDevice_t = HRESULT(WINAPI*)(IDirect3D8*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice8**);
    CreateDevice_t oCreateDevice = nullptr;

    HRESULT WINAPI hCreateDevice(IDirect3D8* d3d, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
      HRESULT hr = oCreateDevice(d3d, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

      install_device_hooks(*ppReturnedDeviceInterface);

      return hr;
    }
  }
  namespace d3ddevice {
    using EndScene_t = HRESULT(WINAPI*)(IDirect3DDevice8*);
    using DrawPrimitive_t = HRESULT(WINAPI*)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, UINT);
    using DrawIndexedPrimitive_t = HRESULT(WINAPI*)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, UINT, UINT, UINT);
    EndScene_t oEndScene = nullptr;
    DrawPrimitive_t oDrawPrimitive = nullptr;
    DrawIndexedPrimitive_t oDrawIndexedPrimitive = nullptr;

    HRESULT WINAPI hEndScene(IDirect3DDevice8* device) {
      return oEndScene(device);
    }
    HRESULT WINAPI hDrawPrimitive(IDirect3DDevice8* device, D3DPRIMITIVETYPE PrimitiveType, UINT StartIndex, UINT PrimitiveCount) {
      return oDrawPrimitive(device, PrimitiveType, StartIndex, PrimitiveCount);
    }
    HRESULT WINAPI hDrawIndexedPrimitive(IDirect3DDevice8* device, D3DPRIMITIVETYPE PrimitiveType, UINT minIndex, UINT NumVertices, UINT startIndex, UINT primCount) {
      return oDrawIndexedPrimitive(device, PrimitiveType, minIndex, NumVertices, startIndex, primCount);
    }
  }

  void install_d3d_hooks(IDirect3D8* d3d) {
    void** vftable = *(void***)d3d;

    MH_Initialize();
    MH_CreateHook(vftable[15],
      hooks::d3d::hCreateDevice,
      (void**)&hooks::d3d::oCreateDevice);
    MH_EnableHook(vftable[15]);
  }

  void install_device_hooks(IDirect3DDevice8* device) {
    void** vftable = *(void***)device;

    MH_CreateHook(vftable[35],
      hooks::d3ddevice::hEndScene,
      (void**)&hooks::d3ddevice::oEndScene);
    MH_CreateHook(vftable[70],
      hooks::d3ddevice::hDrawPrimitive,
      (void**)&hooks::d3ddevice::oDrawPrimitive);
    MH_CreateHook(vftable[71],
      hooks::d3ddevice::hDrawIndexedPrimitive,
      (void**)&hooks::d3ddevice::oDrawIndexedPrimitive);
    MH_EnableHook(MH_ALL_HOOKS);
  }

  DECLARE_HOOK(0x4ac4d0,DoPostDynaLoad);
  using server::Send_DoPostDynaLoad;
  void __declspec(naked) DoPostDynaLoad::detour() {
    __asm pushad
    __asm call Send_DoPostDynaLoad
    __asm popad
    __asm jmp trampoline
  }

  void install_game_hooks() {
    MH_Initialize();
    DoPostDynaLoad::install();
  }
}

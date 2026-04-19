// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "hooks.h"

WNDPROC oWndProc;
LRESULT WINAPI hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;
  return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

decltype(&IDirect3D8::CreateDevice) oCreateDevice = nullptr;
HRESULT WINAPI hkCreateDevice(IDirect3D8* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
  HRESULT hr = (_this->*oCreateDevice)(Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

  if (!ppReturnedDeviceInterface || !*ppReturnedDeviceInterface)
    return hr;

  IDirect3DDevice8* device = *ppReturnedDeviceInterface;
  ImGui::CreateContext();
  ImGui_ImplWin32_Init(hWnd);
  ImGui_ImplDX8_Init(device);
  oWndProc = reinterpret_cast<WNDPROC>
    (SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(hkWndProc)));

  return hr;
}

__declspec(dllexport) IDirect3D8* WINAPI _Direct3DCreate8(UINT SDKVersion) {
  typedef IDirect3D8* (WINAPI* tDirect3DCreate8)(UINT);
  static tDirect3DCreate8 s_oDirect3DCreate8 = nullptr;

  if (!s_oDirect3DCreate8) {
    char path[MAX_PATH];
    if (!GetSystemDirectory(path, MAX_PATH)) return nullptr;
    strcat_s(path, "\\d3d8.dll");
    HMODULE hD3DModule = LoadLibrary(path);
    if (!hD3DModule) return nullptr;

    s_oDirect3DCreate8 = (tDirect3DCreate8)GetProcAddress(hD3DModule, "Direct3DCreate8");
    if (!s_oDirect3DCreate8) return nullptr;
  }

  IDirect3D8* pD3D = s_oDirect3DCreate8(SDKVersion);
  if (!pD3D) return nullptr;

  void** vftable = *(void***)pD3D;

  static bool s_is_hooked = false;
  if (!s_is_hooked) {
    MH_Initialize();
    MH_CreateHook(vftable[15],
      reinterpret_cast<void*>(hkCreateDevice),
      reinterpret_cast<void**>(&oCreateDevice));
    MH_EnableHook(vftable[15]);
    Hooks::inject_hooks();

    s_is_hooked = true;
  }

  return pD3D;
}

BOOL APIENTRY DllMain(HMODULE hModule,
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


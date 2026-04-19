#include "pch.h"

#include <windows.h>
#include <stdio.h>
#include <d3dx8.h>
#include <MinHook.h>

#pragma comment(lib, "d3d8.lib")
#pragma comment(lib, "libMinHook.x86.lib")


namespace Hooks {
    namespace WorldScene {
        namespace RenderOpaque {
            static LPVOID Fn = (LPVOID)0x49cb40;
            using T = void(__stdcall*)(void);
            inline T O = nullptr;

            inline void __declspec(naked) Hk(void) {
                __asm {
                    ret
                }
            }
        }
    }

    inline void Initialize() {
        if (MH_CreateHook(WorldScene::RenderOpaque::Fn, &WorldScene::RenderOpaque::Hk, reinterpret_cast<LPVOID*>(&WorldScene::RenderOpaque::O)) == MH_OK)
            MH_EnableHook(WorldScene::RenderOpaque::Fn);
    }
}


typedef HRESULT(STDMETHODCALLTYPE* tCreateDevice)(IDirect3D8* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface);

IDirect3DDevice8* pDevice = nullptr;
static tCreateDevice oCreateDevice = nullptr;

HRESULT STDMETHODCALLTYPE hkCreateDevice(IDirect3D8* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
    HRESULT hr = oCreateDevice(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    if (hr != D3D_OK) return hr;
    if (!ppReturnedDeviceInterface) return hr;
    if (!(*ppReturnedDeviceInterface)) return hr;

    pDevice = *ppReturnedDeviceInterface;
    void** vtable = *(void***)pDevice;

    return hr;
}

extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    static HMODULE hRealDirect3D = []() {
        char path[MAX_PATH];
        GetSystemDirectory(path, MAX_PATH);
        strcat_s(path, "\\d3d8.dll");
        return LoadLibrary(path);
    }();
    if (!hRealDirect3D) return nullptr;


    typedef IDirect3D8* (WINAPI* tDirect3DCreate8)(UINT SDKVersion);
    static tDirect3DCreate8 oDirect3DCreate8 = (tDirect3DCreate8)GetProcAddress(hRealDirect3D, "Direct3DCreate8");
    IDirect3D8* pDirect3D = oDirect3DCreate8(SDKVersion);
    if (!pDirect3D) return nullptr;

    void** vtable = *(void***)pDirect3D;

    if (oCreateDevice == nullptr) {
        if (MH_CreateHook(vtable[15], (LPVOID)hkCreateDevice, reinterpret_cast<LPVOID*>(&oCreateDevice)) == MH_OK) {
            MH_EnableHook(vtable[15]);
        }
    }

    return pDirect3D;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call != DLL_PROCESS_ATTACH) return TRUE;

    if(MH_Initialize() == MH_OK) {
		Hooks::Initialize();
    }

    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    return TRUE;
}

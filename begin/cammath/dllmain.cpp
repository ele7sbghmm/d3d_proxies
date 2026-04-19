#include "pch.h"

#include <windows.h>
#include <stdio.h>
#include <d3dx8.h>

#include <MinHook.h>

#pragma comment(lib, "d3dx8.lib")
#pragma comment(lib, "libMinHook.x86.lib")

void Clear();
void ClearBuffer(DWORD);
void ClearTarget(DWORD);
void ClearBufferTarget(DWORD);
void BeginScene();
void EndScene();

void TestZBuffer();


typedef HRESULT(STDMETHODCALLTYPE* tDrawPrimitive)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, UINT);
typedef HRESULT(STDMETHODCALLTYPE* tDrawIndexPrimitive)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, UINT, UINT, UINT);
typedef HRESULT(STDMETHODCALLTYPE* tDrawPrimitiveUP)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, const void*, UINT);
typedef HRESULT(STDMETHODCALLTYPE* tDrawIndexedPrimitiveUP)(IDirect3DDevice8*, D3DPRIMITIVETYPE, UINT, UINT, UINT, const void*, D3DFORMAT, const void*, UINT);
typedef HRESULT(STDMETHODCALLTYPE* tSetRenderState)(IDirect3DDevice8*, D3DRENDERSTATETYPE, DWORD);
typedef HRESULT(STDMETHODCALLTYPE* tClear)(IDirect3DDevice8*, DWORD, const D3DRECT*, DWORD, D3DCOLOR, float, DWORD);
typedef HRESULT(STDMETHODCALLTYPE* tBeginScene)(IDirect3DDevice8*);
typedef HRESULT(STDMETHODCALLTYPE* tEndScene)(IDirect3DDevice8*);
typedef HRESULT(STDMETHODCALLTYPE* tCreateDevice)(void*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice8**);
typedef IDirect3D8* (WINAPI* tDirect3DCreate8)(UINT);

tSetRenderState oSetRenderState = nullptr;
tClear oClear = nullptr;
tBeginScene oBeginScene = nullptr;
tEndScene oEndScene = nullptr;
tCreateDevice oCreateDevice = nullptr;
IDirect3DDevice8* pDevice = nullptr;

HRESULT STDMETHODCALLTYPE hkSetRenderState(IDirect3DDevice8* pDevicePtr, D3DRENDERSTATETYPE State, DWORD Value) {
    if (!pDevicePtr) return oSetRenderState(pDevicePtr, State, Value);
    pDevice = pDevicePtr;

    switch (State) {
    case D3DRS_ZWRITEENABLE: {
        if (Value == FALSE) { break; }
        if (Value == TRUE) {
            //TestZBuffer();
            break;
        }
    };
    default: break;
    };

    return oSetRenderState(pDevice, State, Value);
}

HRESULT STDMETHODCALLTYPE hkClear(IDirect3DDevice8* pDevicePtr, DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
    if (!pDevicePtr) return oClear(pDevicePtr, Count, pRects, Flags, Color, Z, Stencil);
    pDevice = pDevicePtr;

    return oClear(pDevice, Count, pRects, Flags, Color, Z, Stencil);
}

HRESULT STDMETHODCALLTYPE hkBeginScene(IDirect3DDevice8* pDevicePtr) {
    if (!pDevicePtr) return oBeginScene(pDevicePtr);
    pDevice = pDevicePtr;

    static bool done = false;
    if (!done) {
        printf("oBeginScene: %p\n", (void*)oBeginScene);
        done = true;
    }

    return oBeginScene(pDevice);
}

HRESULT STDMETHODCALLTYPE hkEndScene(IDirect3DDevice8* pDevicePtr) {
    if (!pDevicePtr) return oEndScene(pDevice);
    pDevice = pDevicePtr;

    return oEndScene(pDevice);
}

HRESULT STDMETHODCALLTYPE hkCreateDevice(void* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
    HRESULT hr = oCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    if (hr == D3D_OK && ppReturnedDeviceInterface && *ppReturnedDeviceInterface) {
        void** vtable = *(void***)*ppReturnedDeviceInterface;

        //printf("[DEBUG] pDevice: 0x%p\n", pDevice);
        printf("[DEBUG] VTable:  0x%p\n", vtable);

        static bool hookedSetRenderState = false;
        if (!hookedSetRenderState) {
            if (MH_CreateHook(vtable[50], (LPVOID)&hkSetRenderState, reinterpret_cast<LPVOID*>(&oSetRenderState)) == MH_OK) {
                MH_EnableHook(vtable[50]);
                hookedSetRenderState = true;
            }
        }

        static bool hookedClear = false;
        if (!hookedClear) {
            if (MH_CreateHook(vtable[36], (LPVOID)&hkClear, reinterpret_cast<LPVOID*>(&oClear)) == MH_OK) {
                MH_EnableHook(vtable[36]);
                hookedClear = true;
            }
        }

        static bool hookedBeginScene = false;
        if (!hookedBeginScene) {
            if (MH_CreateHook(vtable[34], (LPVOID)&hkBeginScene, reinterpret_cast<LPVOID*>(&oBeginScene)) == MH_OK) {
                MH_EnableHook(vtable[34]);
                hookedBeginScene = true;
            }
        }

        static bool hookedEndScene = false;
        if (!hookedEndScene) {
            if (MH_CreateHook(vtable[35], (LPVOID)&hkEndScene, reinterpret_cast<LPVOID*>(&oEndScene)) == MH_OK) {
                MH_EnableHook(vtable[35]);
                hookedEndScene = true;
            }
        }
    }
    return hr;
}

extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    char sysPath[MAX_PATH];
    GetSystemDirectoryA(sysPath, MAX_PATH);
    strcat_s(sysPath, "\\d3d8.dll");

    HMODULE hRealD3D8 = LoadLibraryA(sysPath);
    if (!hRealD3D8) return nullptr;

    IDirect3D8* pD3D = ((tDirect3DCreate8)GetProcAddress(hRealD3D8, "Direct3DCreate8"))(SDKVersion);
    if (!pD3D) return nullptr;

    void** vtable = *(void***)pD3D;
    if (oCreateDevice == nullptr) {
        if (MH_CreateHook(vtable[15], (LPVOID)&hkCreateDevice, reinterpret_cast<LPVOID*>(&oCreateDevice)) == MH_OK) {
            MH_EnableHook(vtable[15]);
        }
    }

    return pD3D;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        if (MH_Initialize() != MH_OK) return false;

        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);

        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


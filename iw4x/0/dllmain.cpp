// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>
#include <iostream>
#include <d3d9.h>

using SetRenderState_t = HRESULT(__stdcall*)(IDirect3DDevice9*,D3DRENDERSTATETYPE,DWORD);
using EndScene_t = HRESULT(__stdcall*)(IDirect3DDevice9*);
using CreateDevice_t = HRESULT(__stdcall*)(IDirect3D9*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,IDirect3DDevice9**);

SetRenderState_t oSetRenderState = nullptr;
EndScene_t oEndScene = nullptr;
CreateDevice_t oCreateDevice = nullptr;

HRESULT __stdcall hSetRenderState(IDirect3DDevice9* device, D3DRENDERSTATETYPE State, DWORD Value) {
    return oSetRenderState(device, State, Value);
}

HRESULT __stdcall hEndScene(IDirect3DDevice9* device) {
    return oEndScene(device);
}

HRESULT __stdcall hCreateDevice(IDirect3D9* d3d, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
    HRESULT hr = oCreateDevice(d3d, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    void** vftable = *(void***)*ppReturnedDeviceInterface;

    DWORD old;
    VirtualProtect(&vftable[42], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    oEndScene = (EndScene_t)vftable[42];
    vftable[42] = &hEndScene;
    VirtualProtect(&vftable[42], sizeof(void*), old, &old);

    VirtualProtect(&vftable[57], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    oSetRenderState = (SetRenderState_t)vftable[57];
    vftable[57] = &hSetRenderState;
    VirtualProtect(&vftable[57], sizeof(void*), old, &old);

    return hr;
}

IDirect3D9* __stdcall Direct3DCreate9(UINT SDKVersion) {
    using Direct3DCreate9_t = IDirect3D9*(WINAPI*)(UINT);
    static Direct3DCreate9_t oDirect3DCreate9 = nullptr;

    if (!oDirect3DCreate9) {
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, "\\d3d9.dll");
        HMODULE hModule = LoadLibraryA(path);
        if (!hModule) throw;

        oDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(hModule, "Direct3DCreate9");
    }

    IDirect3D9* d3d = oDirect3DCreate9(SDKVersion);
    void** vftable = *(void***)d3d;

    if (!oCreateDevice) {
        DWORD old;
        VirtualProtect(&vftable[16], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
        oCreateDevice = (CreateDevice_t)vftable[16];
        vftable[16] = &hCreateDevice;
        VirtualProtect(&vftable[16], sizeof(void*), old, &old);
    }

    return d3d;
}

HRESULT __stdcall Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ppD3D9Ex) { return {}; }

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


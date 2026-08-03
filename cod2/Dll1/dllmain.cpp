// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "d3d9.h"
#pragma comment(linker, "/export:Direct3DCreate9=_Direct3DCreate9@4")

#include "draw.hpp"
#include "hooks.hpp"

extern Drawer g_draw;

using CreateDevice_t = HRESULT(__stdcall*)(IDirect3D9*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,IDirect3DDevice9**);
CreateDevice_t oCreateDevice = nullptr;

HRESULT __stdcall hCreateDevice(IDirect3D9* d3d9, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    HRESULT hr = oCreateDevice(d3d9, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    void** vftable = *(void***)*ppReturnedDeviceInterface;

    g_draw = { *ppReturnedDeviceInterface };

    MH_Initialize();
    MH_CreateHook(I::addr, (void*)I::hook, (void**)&I::orig);
    MH_EnableHook(I::addr);
    
    return hr;
}

extern "C" IDirect3D9* __stdcall Direct3DCreate9(UINT SDKVersion)
{
    using Direct3DCreate9_t = IDirect3D9*(__stdcall*)(UINT);
    static Direct3DCreate9_t oDirect3DCreate9 = nullptr;

    if (!oDirect3DCreate9)
    {
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, "\\d3d9.dll");
        HMODULE hModule = LoadLibraryA(path);
        if (!hModule)
            return nullptr;

        oDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(hModule, "Direct3DCreate9");
    }

    IDirect3D9* d3d9 = oDirect3DCreate9(SDKVersion);
    void** vftable = *(void***)d3d9;

    if (!oCreateDevice)
    {
        DWORD old;
        VirtualProtect(&vftable[16], 4, PAGE_EXECUTE_READWRITE, &old);
        oCreateDevice = (CreateDevice_t)vftable[16];
        vftable[16] = &hCreateDevice;
        VirtualProtect(&vftable[16], 4, old, &old);
    }

    return d3d9;
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


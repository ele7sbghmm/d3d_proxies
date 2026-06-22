// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <d3d9.h>
#include <iostream>

#pragma comment(linker, "/export:Direct3DCreate9=_Direct3DCreate9@4")
#pragma comment(linker, "/export:Direct3DCreate9Ex=_Direct3DCreate9Ex@8"

extern Drawer g_drawer;

HRESULT __stdcall hCreateDevice(IDirect3D9* d3d9, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
    HRESULT hr = oCreateDevice(d3d9, Adapter, DeviceType, hWnd, BehaviorFlags,
        pPresentationParameters, ppReturnedDeviceInterface);

    g_Drawer = { *ppReturnedDeviceInterface };

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
        if (!hModule) return nullptr;

        oDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(hModule, "Direct3DCreate9");
    }

    IDirect3D9* d3d9 = oDirect3DCreate9(SDKVersion);

    return d3d9;
}

extern "C" HRESULT __stdcall Direct3DCreate9Ex(UINT, IDirect3D9Ex**)
{
    printf("Direct3DCreate9Ex\n");
    return S_OK;
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


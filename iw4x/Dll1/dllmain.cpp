// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <d3d9.h>
#include <iostream>
#pragma comment(linker, "/export:Direct3DCreate9=_Direct3DCreate9@4")
#pragma comment(linker, "/export:Direct3DCreate9Ex=_Direct3DCreate9Ex@8")

#include "draw.hpp"
#include "hooker.hpp"

extern Drawer g_Drawer;

using CreateDevice_t = HRESULT(__stdcall*)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
using EndScene_t = HRESULT(__stdcall*)(IDirect3DDevice9*);
using DrawPrimitive_t = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, UINT, UINT);
using DrawIndexedPrimitive_t = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
CreateDevice_t oCreateDevice = nullptr;
EndScene_t oEndScene = nullptr;
DrawPrimitive_t oDrawPrimitive = nullptr;
DrawIndexedPrimitive_t oDrawIndexedPrimitive = nullptr;

size_t dp_n = 0;
size_t dip_n = 0;
size_t es_n = 0;

HRESULT __stdcall hDrawPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE pt, 
    UINT si, UINT pc)
{
    dp_n++;
    return oDrawPrimitive(device, pt, si, pc);
}

HRESULT __stdcall hDrawIndexedPrimitive(IDirect3DDevice9* device, 
    D3DPRIMITIVETYPE pt, INT bvi, UINT mvi, UINT nv, UINT si, UINT pc)
{
    dip_n++;
    HRESULT hr = oDrawIndexedPrimitive(device, pt, bvi, mvi, nv, si, pc);

    return hr;
}

HRESULT __stdcall hEndScene(IDirect3DDevice9* device)
{
    printf("ES: %d\n", es_n);
    printf("DP: %d\n", dp_n);
    printf("DIP: %d\n", dip_n);
    dp_n = 0;
    dip_n = 0;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), {0, 0});

    es_n++;
    return oEndScene(device);
}

HRESULT __stdcall hCreateDevice(IDirect3D9* d3d9, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
    HRESULT hr = oCreateDevice(d3d9, Adapter, DeviceType, hWnd, BehaviorFlags,
        pPresentationParameters, ppReturnedDeviceInterface);

    g_Drawer = { *ppReturnedDeviceInterface };

    void** vftable = *(void***)*ppReturnedDeviceInterface;

    DWORD old;
    if (!oEndScene)
    {
        VirtualProtect(&vftable[42], 4, PAGE_EXECUTE_READWRITE, &old);
        oEndScene = (EndScene_t)vftable[42];
        vftable[42] = &hEndScene;
        VirtualProtect(&vftable[42], 4, old, &old);
    }
    if (!oDrawPrimitive)
    {
        VirtualProtect(&vftable[81], 4, PAGE_EXECUTE_READWRITE, &old);
        oDrawPrimitive = (DrawPrimitive_t)vftable[81];
        vftable[81] = &hDrawPrimitive;
        VirtualProtect(&vftable[81], 4, old, &old);
    }
    if (!oDrawIndexedPrimitive)
    {
        VirtualProtect(&vftable[82], 4, PAGE_EXECUTE_READWRITE, &old);
        oDrawIndexedPrimitive = (DrawIndexedPrimitive_t)vftable[82];
        vftable[82] = &hDrawIndexedPrimitive;
        VirtualProtect(&vftable[82], 4, old, &old);
    }

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

        FILE* f;
        AllocConsole();
        freopen_s(&f, "CONOUT$", "w", stdout);
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

    Hooker::Init();

    return d3d9;
}

extern "C" HRESULT __stdcall Direct3DCreate9Ex(UINT, IDirect3D9Ex**)
{
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

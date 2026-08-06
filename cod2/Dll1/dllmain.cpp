// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "d3dx9.h"
#pragma comment(linker, "/export:Direct3DCreate9=_Direct3DCreate9@4")

#include "draw.hpp"
#include "hooks.hpp"

extern Drawer g_draw;

using DrawIndexedPrimitive_t = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
using EndScene_t = HRESULT(__stdcall*)(IDirect3DDevice9*);
DrawIndexedPrimitive_t oDrawIndexedPrimitive = nullptr;
EndScene_t oEndScene = nullptr;

int n = 0;
HRESULT __stdcall hDrawIndexedPrimitive(IDirect3DDevice9* device, D3DPRIMITIVETYPE pt, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount) {

    if (n == 1)
    {
        //g_draw.Flush();
        //n++;
        //return S_OK;
    }

    HRESULT hr = oDrawIndexedPrimitive(device, pt, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);

    n++;

    return hr;
}

HRESULT __stdcall hEndScene(IDirect3DDevice9* device)
{
    printf("%d\n", n);
    n = 0;
    return oEndScene(device);
}

using CreateDevice_t = HRESULT(__stdcall*)(IDirect3D9*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,IDirect3DDevice9**);
CreateDevice_t oCreateDevice = nullptr;

HRESULT __stdcall hCreateDevice(IDirect3D9* d3d9, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    HRESULT hr = oCreateDevice(d3d9, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    void** vftable = *(void***)*ppReturnedDeviceInterface;

    DWORD old;
    if (!oDrawIndexedPrimitive)
    {
        VirtualProtect(&vftable[82], 4, PAGE_EXECUTE_READWRITE, &old);
        oDrawIndexedPrimitive = (DrawIndexedPrimitive_t)vftable[82];
        vftable[82] = &hDrawIndexedPrimitive;
        VirtualProtect(&vftable[82], 4, old, &old);
    }
    if (!oEndScene)
    {
        VirtualProtect(&vftable[42], 4, PAGE_EXECUTE_READWRITE, &old);
        oEndScene = (EndScene_t)vftable[42];
        vftable[42] = &hEndScene;
        VirtualProtect(&vftable[42], 4, old, &old);
    }

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

        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
    }

    IDirect3D9* d3d9 = oDirect3DCreate9(SDKVersion);
    void** vftable = *(void***)d3d9;

    DWORD old;
    if (!oCreateDevice)
    {
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


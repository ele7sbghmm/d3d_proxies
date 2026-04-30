// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <iostream>
#include <d3d9.h>

HRESULT WINAPI Direct3DCreate9Ex(UINT SDKVersion, IDirect3D9Ex** ppD3D9Ex) {
    using Direct3DCreate9Ex_t = HRESULT(WINAPI*)(UINT, IDirect3D9Ex**);
    static Direct3DCreate9Ex_t oDirect3DCreate9Ex = nullptr;

    if (!oDirect3DCreate9Ex) {
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, "\\d3d9.dll");
        HMODULE hModule = LoadLibraryA(path);
        if (!hModule) throw;

        oDirect3DCreate9Ex = (Direct3DCreate9Ex_t)GetProcAddress(hModule, "Direct3DCreate9Ex");
    }

    if (!oDirect3DCreate9Ex) throw;

    return oDirect3DCreate9Ex(SDKVersion, ppD3D9Ex);
}

IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {
    using Direct3DCreate9_t = IDirect3D9 * (WINAPI*)(UINT);
    static Direct3DCreate9_t oDirect3DCreate9 = nullptr;

    if (!oDirect3DCreate9) {
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, "\\d3d9.dll");
        HMODULE hModule = LoadLibraryA(path);
        if (!hModule) throw;

        oDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(hModule, "Direct3DCreate9");

        AllocConsole();
        FILE* f;
        freopen_s(&f, "CONOUT$", "w", stdout);
        printf("workin'");
    }

    IDirect3D9* d3d = oDirect3DCreate9(SDKVersion);

    return d3d;
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


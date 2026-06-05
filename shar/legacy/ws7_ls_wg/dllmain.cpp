// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <thread>
#include <d3dx8.h>
#pragma comment(linker, "/export:Direct3DCreate8=_Direct3DCreate8@4")

#include "server.hpp"

extern "C" IDirect3D8* __stdcall Direct3DCreate8(UINT SDKVersion) {
    using Direct3DCreate8_t = IDirect3D8* (__stdcall*)(UINT);
    static Direct3DCreate8_t oDirect3DCreate8 = nullptr;

    if (!oDirect3DCreate8) {
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, "\\d3d8.dll");
        HMODULE hModule = LoadLibraryA(path);
        oDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");
    }

    IDirect3D8* d3d = oDirect3DCreate8(SDKVersion);

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
        std::thread(run).detach();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


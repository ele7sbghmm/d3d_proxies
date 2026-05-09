#include "pch.h"

#include <d3d8.h>
#include <Minhook.h>
#pragma comment(lib, "libMinhook.x86.lib")
#include "hooks.h"

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    using Direct3DCreate8_t = IDirect3D8*(WINAPI*)(UINT);
    static Direct3DCreate8_t oDirect3DCreate8 = nullptr;

    if (!oDirect3DCreate8) {
        char path[MAX_PATH] = {};
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, "\\d3d8.dll");
        HMODULE hModule = LoadLibraryA(path);
        if (!hModule) return nullptr;
        oDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");
    }

    IDirect3D8* d3d = oDirect3DCreate8(SDKVersion);

    MH_Initialize();
    hooks::is_cheat_entered::inject();

    return d3d;
}
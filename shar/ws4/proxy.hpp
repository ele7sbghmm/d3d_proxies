#include <thread>
#include <d3d8.h>
#include "server.hpp"

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion)
{
    using Direct3DCreate8_t = IDirect3D8*(WINAPI*)(UINT);
    static Direct3DCreate8_t oDirect3DCreate8 = {};

    if (!oDirect3DCreate8) {
        char path[MAX_PATH];
        GetSystemDirectory(path, MAX_PATH);
        strcat_s(path, "\\d3d8.dll");
        HMODULE hModule = LoadLibrary(path);
        if (!hModule) return {};
        oDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");
    }

    IDirect3D8* d3d = oDirect3DCreate8(SDKVersion);

    std::thread(Server::run).detach();

    return d3d;
}
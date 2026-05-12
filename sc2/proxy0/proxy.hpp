#include <iostream>
#include <d3d9.h>

inline IDirect3DDevice9* g_device;

using CreateDevice_t = HRESULT(WINAPI*)(IDirect3D9*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,IDirect3DDevice9**);
CreateDevice_t oCreateDevice = nullptr;

HRESULT WINAPI hCreateDevice(IDirect3D9* d3d, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    HRESULT hr = oCreateDevice(d3d, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    g_device = *ppReturnedDeviceInterface;
    void** vftable = *(void***)*ppReturnedDeviceInterface;
}

IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion)
{
    using Direct3DCreate9_t = IDirect3D9*(WINAPI*)(UINT);
    static Direct3DCreate9_t oDirect3DCreate9 = nullptr;

    if (!oDirect3DCreate9) {
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, "\\d3d9.dll");
        HMODULE hModule = LoadLibraryA(path);
        if (!hModule) return nullptr;
        oDirect3DCreate9 = (Direct3DCreate9_t)GetProcAddress(hModule, "Direct3DCreate9");
    }

    IDirect3D9* d3d = oDirect3DCreate9(SDKVersion);

    AllocConsole();
    FILE* f{};
    freopen_s(&f, "CONOUT$", "w", stdout);
    printf("workin'");

    return d3d;
}
#include <iostream>
#include <d3d9.h>

inline IDirect3DDevice9* g_device;

using EndScene_t = HRESULT(WINAPI*)(IDirect3DDevice9*);
using CreateDevice_t = HRESULT(WINAPI*)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);

EndScene_t oEndScene = nullptr;
CreateDevice_t oCreateDevice = nullptr;


HRESULT WINAPI hEndScene(IDirect3DDevice9 * this_)
{
    return oEndScene(this_);
}

HRESULT WINAPI hCreateDevice(IDirect3D9* this_, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    HRESULT hr = oCreateDevice(this_, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    g_device = *ppReturnedDeviceInterface;
    void** vftable = *(void***)*ppReturnedDeviceInterface;

    return hr;
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

    void** vftable = *(void***)d3d;

    if (!oCreateDevice) {
        DWORD old{};
        VirtualProtect(&vftable[16], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
        oCreateDevice = (CreateDevice_t)vftable[16];
        vftable[16] = &hCreateDevice;
        VirtualProtect(&vftable[16], sizeof(void*), old, &old);
    }

    AllocConsole();
    FILE* f{};
    freopen_s(&f, "CONOUT$", "w", stdout);

    return d3d;
}
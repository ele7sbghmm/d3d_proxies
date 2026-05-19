#include "pch.h"

#include <d3dx8.h>
#include <Minhook.h>
#pragma comment(lib, "d3dx8.lib")
#pragma comment(lib, "libMinhook.x86.lib")

#include "hooks.h"

float factor = .9f;

using SetTransform_t = HRESULT(__stdcall*)(IDirect3DDevice8*,D3DTRANSFORMSTATETYPE,D3DMATRIX*);
using CreateDevice_t = HRESULT(__stdcall*)(IDirect3D8*,UINT,D3DDEVTYPE,HWND,DWORD,D3DPRESENT_PARAMETERS*,IDirect3DDevice8**);
SetTransform_t oSetTransform = nullptr;
CreateDevice_t oCreateDevice = nullptr;

HRESULT __stdcall hSetTransform(IDirect3DDevice8* device, D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix) {
    if (State == D3DTS_PROJECTION) {
    }
    return oSetTransform(device, State, pMatrix);
}

HRESULT __stdcall hCreateDevice(IDirect3D8* d3d, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
    HRESULT hr = oCreateDevice(d3d, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    void** vftable = *(void***)*ppReturnedDeviceInterface;

    MH_CreateHook(vftable[37], hSetTransform, (void**)&oSetTransform);
    MH_EnableHook(vftable[37]);

    return hr;
}

IDirect3D8* __stdcall Direct3DCreate8(UINT SDKVersion) {
    using Direct3DCreate8_t = IDirect3D8*(__stdcall*)(UINT);
    static Direct3DCreate8_t oDirect3DCreate8 = nullptr;

    if (!oDirect3DCreate8) {
        char path[MAX_PATH];
        GetSystemDirectoryA(path, MAX_PATH);
        strcat_s(path, "\\d3d8.dll");
        HMODULE hModule = LoadLibraryA(path);
        if (!hModule) return nullptr;
        oDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");
    }

    IDirect3D8* d3d = oDirect3DCreate8(SDKVersion);
    void** vftable = *(void***)d3d;

    MH_Initialize();
    MH_CreateHook(vftable[15], hCreateDevice, (void**)&oCreateDevice);
    MH_EnableHook(vftable[15]);

    //hooks::pddi_set_projection_mode::inject();

    return d3d;
}
// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <d3d9.h>
#include <iostream>

#pragma comment(linker, "/export:Direct3DCreate9=_Direct3DCreate9@4")
#pragma comment(linker, "/export:Direct3DCreate9Ex=_Direct3DCreate9Ex@8")

struct Vtx
{
    D3DVECTOR xyz;
    D3DCOLOR color;
    static constexpr DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

using EndScene_t = HRESULT(__stdcall*)(IDirect3DDevice9*);
EndScene_t oEndScene = nullptr;


HRESULT __stdcall hEndScene(IDirect3DDevice9* device)
{
    D3DCOLOR RED = 0xfff0000;
    Vtx vertices[6] =
    {
        {  1000, 0,  1000, RED },
        { -1000, 0,  -1000, RED },
        { -1000, 0,  1000, RED },
        {  1000, 0, -1000, RED }
    };

    static IDirect3DStateBlock9* sb = nullptr;
    if (sb == nullptr) device->CreateStateBlock(D3DSBT_ALL, &sb);
    sb->Capture();

    device->SetTexture(0, NULL);
    device->SetRenderState(D3DRS_LIGHTING, FALSE);
    device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
    device->SetRenderState(D3DRS_ZENABLE, TRUE);
    device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
    device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);

    device->SetVertexShader(nullptr);
    device->SetPixelShader(nullptr);
    device->SetFVF(Vtx::FVF);
    device->DrawPrimitiveUP(D3DPT_LINELIST, 2, vertices, sizeof(Vtx));

    sb->Apply();

    return oEndScene(device);
}

using CreateDevice_t = HRESULT(__stdcall*)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
CreateDevice_t oCreateDevice = nullptr;

HRESULT __stdcall hCreateDevice(IDirect3D9* d3d9, UINT Adapter,
    D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags,
    D3DPRESENT_PARAMETERS* pPresentationParameters,
    IDirect3DDevice9** ppReturnedDeviceInterface)
{
    HRESULT hr = oCreateDevice(d3d9, Adapter, DeviceType, hWnd,
        BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    void** vftable = *(void***)*ppReturnedDeviceInterface;

    DWORD old;
    VirtualProtect(&vftable[42], 4, PAGE_EXECUTE_READWRITE, &old);
    oEndScene = (EndScene_t)vftable[42];
    vftable[42] = &hEndScene;
    VirtualProtect(&vftable[42], 4, old, &old);

    printf("CreateDevice\n");
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
    void** vftable = *(void***)d3d9;

    if (!oCreateDevice)
    {
        DWORD old;
        VirtualProtect(&vftable[16], 4, PAGE_EXECUTE_READWRITE, &old);
        oCreateDevice = (CreateDevice_t)vftable[16];
        vftable[16] = &hCreateDevice;
        VirtualProtect(&vftable[16], 4, old, &old);
    }

    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    printf("Direct3DCreate9\n");
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


#include <iostream>
#include <vector>
#include <d3d9.h>

inline IDirect3DDevice9* g_device;

using DrawPrimitive_t = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, UINT, UINT);
using DrawIndexedPrimitive_t = HRESULT(__stdcall*)(IDirect3DDevice9*, D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT);
using UpdateSurface_t = HRESULT(__stdcall*)(IDirect3DDevice9*, IDirect3DSurface9*, RECT*, IDirect3DSurface9*, POINT*);
using SetRenderTarget_t = HRESULT(__stdcall*)(IDirect3DDevice9*, DWORD, IDirect3DSurface9*);
using EndScene_t = HRESULT(__stdcall*)(IDirect3DDevice9*);
using CreateDevice_t = HRESULT(__stdcall*)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);

DrawPrimitive_t oDrawPrimitive = nullptr;
DrawIndexedPrimitive_t oDrawIndexedPrimitive = nullptr;
UpdateSurface_t oUpdateSurface = nullptr;
SetRenderTarget_t oSetRenderTarget = nullptr;
EndScene_t oEndScene = nullptr;
CreateDevice_t oCreateDevice = nullptr;

HRESULT __stdcall hDrawPrimitive(IDirect3DDevice9* this_, D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
    return oDrawPrimitive(this_, PrimitiveType, StartVertex, PrimitiveCount);
}

HRESULT __stdcall hDrawIndexedPrimitive(IDirect3DDevice9* this_, D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
    return oDrawIndexedPrimitive(this_, PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
}

HRESULT __stdcall hUpdateSurface(IDirect3DDevice9* device, IDirect3DSurface9* pSourceSurface, RECT* pSourceRect, IDirect3DSurface9* pDestSurface, POINT* pDestPoint)
{
    return oUpdateSurface(device, pSourceSurface, pSourceRect, pDestSurface, pDestPoint);
}

HRESULT __stdcall hSetRenderTarget(IDirect3DDevice9* this_, DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
    if (pRenderTarget)
    {
        D3DSURFACE_DESC desc{};
        pRenderTarget->GetDesc(&desc);
        if (desc.Width == 512)
            printf("512: %p\n", pRenderTarget);
    }

    return oSetRenderTarget(this_, RenderTargetIndex, pRenderTarget);
}

HRESULT __stdcall hEndScene(IDirect3DDevice9* this_)
{
    //SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, 0 });

    return oEndScene(this_);
}

HRESULT __stdcall hCreateDevice(IDirect3D9* this_, UINT Adapter, D3DDEVTYPE DeviceType, HWND hWnd, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    HRESULT hr = oCreateDevice(this_, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    g_device = *ppReturnedDeviceInterface;
    void** vftable = *(void***)*ppReturnedDeviceInterface;

    DWORD old;
    VirtualProtect(&vftable[30], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    oUpdateSurface = (UpdateSurface_t)vftable[30];
    vftable[30] = &hUpdateSurface;
    VirtualProtect(&vftable[30], sizeof(void*), old, &old);

    VirtualProtect(&vftable[37], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    oSetRenderTarget = (SetRenderTarget_t)vftable[37];
    vftable[37] = &hSetRenderTarget;
    VirtualProtect(&vftable[37], sizeof(void*), old, &old);

    VirtualProtect(&vftable[42], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    oEndScene = (EndScene_t)vftable[42];
    vftable[42] = &hEndScene;
    VirtualProtect(&vftable[42], sizeof(void*), old, &old);

    VirtualProtect(&vftable[81], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    oDrawPrimitive = (DrawPrimitive_t)vftable[81];
    vftable[81] = &hDrawPrimitive;
    VirtualProtect(&vftable[81], sizeof(void*), old, &old);

    VirtualProtect(&vftable[82], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
    oDrawIndexedPrimitive = (DrawIndexedPrimitive_t)vftable[82];
    vftable[82] = &hDrawIndexedPrimitive;
    VirtualProtect(&vftable[82], sizeof(void*), old, &old);
    
    return hr;
}

IDirect3D9* __stdcall Direct3DCreate9(UINT SDKVersion)
{
    using Direct3DCreate9_t = IDirect3D9*(__stdcall*)(UINT);
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
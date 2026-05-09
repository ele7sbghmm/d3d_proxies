#include "pch.h"

#include "hooks_device.h"

namespace {
    typedef HRESULT(WINAPI* tSetTextureStageState)(IDirect3DDevice8* _this, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
    typedef HRESULT(WINAPI* tSetRenderState)(IDirect3DDevice8* _this, D3DRENDERSTATETYPE State, DWORD Value);
    typedef HRESULT(WINAPI* tEndScene)(IDirect3DDevice8* _this);
    typedef HRESULT(WINAPI* tCreateDevice)(IDirect3D8* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface);

    tSetTextureStageState oSetTextureStageState = nullptr;
    tSetRenderState oSetRenderState = nullptr;
    tEndScene oEndScene = nullptr;
    tCreateDevice oCreateDevice = nullptr;

    HRESULT WINAPI hkSetTextureStageState(IDirect3DDevice8* _this, DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value) {
        if (Stage == 0) {
            //if (Type == D3DTSS_ALPHAOP) Value = D3DTOP_SELECTARG1;
            //if (Type == D3DTSS_ALPHAARG1) Value = D3DTA_TFACTOR;
            if (Type == D3DTSS_ALPHAOP) Value = D3DTOP_MODULATE; // Multiply instead of Replace
            //if (Type == D3DTSS_ALPHAOP)
                //if (Value != D3DTOP_DISABLE)
                    //Value = D3DTOP_MODULATE;
            if (Type == D3DTSS_ALPHAARG1) Value = D3DTA_TEXTURE; // Use the texture...
            if (Type == D3DTSS_ALPHAARG2) Value = D3DTA_TFACTOR; // ...multiplied by our factor
        }
        return oSetTextureStageState(_this, Stage, Type, Value);
    }
    HRESULT WINAPI hkSetRenderState(IDirect3DDevice8* _this, D3DRENDERSTATETYPE State, DWORD Value) {
        if (State == D3DRS_ALPHABLENDENABLE) Value = TRUE;
        //if (State == D3DRS_SRCBLEND) Value = D3DBLEND_SRCALPHA;
        //if (State == D3DRS_DESTBLEND) Value = D3DBLEND_INVSRCALPHA;
        if (State == D3DRS_TEXTUREFACTOR) {
            BYTE oldAlpha = (Value >> 24) & 0xff;
            BYTE newAlpha = oldAlpha / 3;
            DWORD color = (Value & 0x00ffffff);
            DWORD newFactor = (newAlpha << 24) | color;
            Value = newFactor;
            //Value = 0x80ffffff;
        }

        return oSetRenderState(_this, State, Value);
    }
    HRESULT WINAPI hkEndScene(IDirect3DDevice8* _this) {
        return oEndScene(_this);
    }
    HRESULT WINAPI hkCreateDevice(IDirect3D8* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
        HRESULT hr = oCreateDevice(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

        if (SUCCEEDED(hr) && ppReturnedDeviceInterface && *ppReturnedDeviceInterface) {
            Globals::GameDevice = *ppReturnedDeviceInterface;

            void** vtable = *(void***)Globals::GameDevice;
            //MH_CreateHook(vtable[35], (LPVOID)hkEndScene, reinterpret_cast<LPVOID*>(&oEndScene));
            //MH_CreateHook(vtable[50], (LPVOID)hkSetRenderState, reinterpret_cast<LPVOID*>(&oSetRenderState));
            //MH_CreateHook(vtable[63], (LPVOID)hkSetTextureStageState, reinterpret_cast<LPVOID*>(&oSetTextureStageState));
            //MH_EnableHook(MH_ALL_HOOKS);

            Hooks::Create();
        }
        return hr;
    }
}

static HMODULE GetDirect3D8Module() {
    static HMODULE hModule = []() {
        char path[MAX_PATH];
        GetSystemDirectory(path, MAX_PATH);
        strcat_s(path, "\\d3d8.dll");
        return LoadLibrary(path);
        }();

    return hModule;
}

extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    HMODULE hRealDirect3D = GetDirect3D8Module();
    if (!hRealDirect3D) return nullptr;

    typedef IDirect3D8* (WINAPI* tDirect3DCreate8)(UINT SDKVersion);
    static tDirect3DCreate8 oDirect3DCreate8 = nullptr;
    if (!oDirect3DCreate8)
        oDirect3DCreate8 = (tDirect3DCreate8)GetProcAddress(hRealDirect3D, "Direct3DCreate8");

    IDirect3D8* pDirect3D = oDirect3DCreate8(SDKVersion);
    if (!pDirect3D) return nullptr;

    static bool hooked = false;
    if (!hooked) {
        void** vtable = *(void***)pDirect3D;

        MH_Initialize();
        if (MH_CreateHook(vtable[15], (LPVOID)hkCreateDevice, reinterpret_cast<LPVOID*>(&oCreateDevice)) == MH_OK)
            MH_EnableHook(vtable[15]);
        hooked = true;
    }

    return pDirect3D;
}

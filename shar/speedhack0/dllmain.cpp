// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include "Minhook.h"
#pragma comment(lib, "libMinhook.x86.lib")

using SetRenderState_t = HRESULT(WINAPI*)(IDirect3DDevice8*,D3DRENDERSTATETYPE,DWORD);
SetRenderState_t oSetRenderState = nullptr;

HRESULT WINAPI hSetRenderState(IDirect3DDevice8* _this, D3DRENDERSTATETYPE State, DWORD Value) {
    if (State == D3DRS_ZFUNC)
        Value = D3DCMP_ALWAYS;

    return oSetRenderState(_this, State, Value);
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MH_Initialize();
        MH_CreateHookApi(L"d3d8.dll", "SetRenderState", hSetRenderState, (void**)&oSetRenderState);
        MH_EnableHook(MH_ALL_HOOKS);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


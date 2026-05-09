// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
    using Direct3DCreate8_t = IDirect3D8*(WINAPI*)(UINT);
    static Direct3DCreate8_t oDirect3DCreate8 = nullptr;

    if (!oDirect3DCreate8) {
        char path[MAX_PATH];
        GetSystemDirectory(path, MAX_PATH);
        strcat_s(path, "\\d3d8.dll");
        HMODULE hModule = LoadLibraryA(path);
        if (!hModule) return nullptr;
        oDirect3DCreate8 = (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");
    }

    IDirect3D8* d3d = oDirect3DCreate8(SDKVersion);
    if (!d3d) return nullptr;

    return d3d;
}

using timeGetTime_t = DWORD(WINAPI*)();
using GetTickCount_t = DWORD(WINAPI*)();
using QueryPerformanceCounter_t = BOOL(WINAPI*)(LARGE_INTEGER*);
using Sleep_t = VOID(WINAPI*)(DWORD);
timeGetTime_t oTimeGetTime = nullptr;
GetTickCount_t oGetTickCount = nullptr;
QueryPerformanceCounter_t oQueryPerformanceCounter = nullptr;
Sleep_t oSleep = nullptr;

DWORD WINAPI hkTimeGetTime() { return (DWORD)(oTimeGetTime() * .5f); }
DWORD WINAPI hkGetTickCount() { return (DWORD)(oGetTickCount() * .5f); }
BOOL WINAPI hkQueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) {
    BOOL ret = oQueryPerformanceCounter(lpPerformanceCount);
    lpPerformanceCount->QuadPart = (LONGLONG)(lpPerformanceCount->QuadPart * .5f);
    return ret;
}
VOID WINAPI hkSleep(DWORD dwMilliseconds) { oSleep((DWORD)(0.f)); }

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        MH_Initialize();
        //MH_CreateHookApi(L"winmm.dll", "timeGetTime", hkTimeGetTime, (void**)&oTimeGetTime);
        //MH_CreateHookApi(L"kernel32.dll", "GetTickCount", hkGetTickCount, (void**)&oGetTickCount);
        MH_CreateHookApi(L"kernel32.dll", "QueryPerformanceCounter", hkQueryPerformanceCounter, (void**)&oQueryPerformanceCounter);
        //MH_CreateHookApi(L"kernel32.dll", "Sleep", hkSleep, (void**)&oSleep);
        MH_EnableHook(MH_ALL_HOOKS);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


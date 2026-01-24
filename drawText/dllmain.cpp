// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <windows.h>
#include <process.h>
#include <string>
#include <d3d9.h>
#include <d3dx9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9d.lib")
// #pragma comment(linker, "/export:Direct3DCreate9=C:\\Windows\\SysWOW64\\d3d9.Direct3DCreate9")
#pragma comment(linker, "/export:Direct3DCreate9Ex=C:\\Windows\\SysWOW64\\d3d9.Direct3DCreate9Ex")
#pragma comment(linker, "/export:D3DPERF_BeginEvent=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_BeginEvent")
#pragma comment(linker, "/export:D3DPERF_EndEvent=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_EndEvent")
#pragma comment(linker, "/export:D3DPERF_GetStatus=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_GetStatus")
#pragma comment(linker, "/export:D3DPERF_QueryRepeatFrame=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_QueryRepeatFrame")
#pragma comment(linker, "/export:D3DPERF_SetMarker=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_SetMarker")
#pragma comment(linker, "/export:D3DPERF_SetOptions=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_SetOptions")
#pragma comment(linker, "/export:D3DPERF_SetRegion=C:\\Windows\\SysWOW64\\d3d9.D3DPERF_SetRegion")
#pragma comment(linker, "/export:DebugSetLevel=C:\\Windows\\SysWOW64\\d3d9.DebugSetLevel")
#pragma comment(linker, "/export:DebugSetMute=C:\\Windows\\SysWOW64\\d3d9.DebugSetMute")
#pragma comment(linker, "/export:Direct3D9EnableMaximizedWindowedModeShim=C:\\Windows\\SysWOW64\\d3d9.Direct3D9EnableMaximizedWindowedModeShim")
#pragma comment(linker, "/export:Direct3DShaderValidatorCreate9=C:\\Windows\\SysWOW64\\d3d9.Direct3DShaderValidatorCreate9")
#pragma comment(linker, "/export:PSGPError=C:\\Windows\\SysWOW64\\d3d9.PSGPError")
#pragma comment(linker, "/export:PSGPSampleTexture=C:\\Windows\\SysWOW64\\d3d9.PSGPSampleTexture")

typedef HRESULT(STDMETHODCALLTYPE* tEndScene)(IDirect3DDevice9*);
typedef HRESULT(STDMETHODCALLTYPE* tCreateDevice)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);

tEndScene oEndScene = nullptr;
tCreateDevice oCreateDevice = nullptr;

HRESULT STDMETHODCALLTYPE hkCreateDevice(IDirect3D9* pD3D, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface) {
	MessageBoxA(NULL, "", "hkCreateDevice", MB_OK);

	HRESULT hr = oCreateDevice(pD3D, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if (hr == D3D_OK && ppReturnedDeviceInterface != nullptr) {
		void** vTable = *(void***)(*ppReturnedDeviceInterface);

		int testIndex = 42;

		DWORD old;
		VirtualProtect(&vTable[testIndex], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
		vTable[testIndex] = NULL;
		VirtualProtect(&testIndex, sizeof(void*), old, &old);
	}
	return hr;
}

extern "C" IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion) {
	MessageBoxA(NULL, "", "Direct3DCreate9", MB_OK);

	HMODULE hRealD3D = LoadLibraryA("C:\\Windows\\SysWOW64\\d3d9.dll");
	typedef IDirect3D9* (WINAPI* tD3DCreate9)(UINT);
	tD3DCreate9 oDirect3DCreate9 = (tD3DCreate9)GetProcAddress(hRealD3D, "Direct3DCreate9");

	IDirect3D9* pD3D = oDirect3DCreate9(SDKVersion);

	if (pD3D) {
		void** vTable = *(void***)pD3D;
		DWORD old;
		VirtualProtect(&vTable[16], sizeof(void*), PAGE_EXECUTE_READWRITE, &old);
		oCreateDevice = (tCreateDevice)vTable[16];
		vTable[16] = (void*)hkCreateDevice;
		VirtualProtect(&vTable[16], sizeof(void*), old, &old);
	}

	return pD3D;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
	MessageBoxA(NULL, "", "DllMain", MB_OK);

	return TRUE;
}

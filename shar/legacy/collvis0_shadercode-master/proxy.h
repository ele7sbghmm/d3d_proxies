#pragma once

#include "mod.h"
#include "MinHook.h"

extern PDIRECT3DDEVICE8 g_device;
extern Fence g_fence;

using t_CreateDevice = HRESULT(WINAPI*)(PDIRECT3D8, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, PDIRECT3DDEVICE8*);

t_CreateDevice o_CreateDevice = nullptr;

HRESULT WINAPI hk_CreateDevice(PDIRECT3D8 _this, DWORD Adapter, D3DDEVTYPE DeviceType, HWND hWnd, UINT BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, PDIRECT3DDEVICE8* ppReturnedDeviceInterface) {
	HRESULT hr = o_CreateDevice(_this, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if (FAILED(hr)) return hr;
	g_device = *ppReturnedDeviceInterface;
	
	void** vftable = *(void***)g_device;

	return hr;
}

extern "C" PDIRECT3D8 WINAPI Direct3DCreate8(UINT SDKVersion) {
	static HMODULE hModule = [&]() {
		char path[MAX_PATH];
		GetSystemDirectory(path, MAX_PATH);
		strcat_s(path, "\\d3d8.dll");
		return LoadLibrary(path);
		}();

	using t_Create = PDIRECT3D8(WINAPI*)(DWORD SDKVersion);
	t_Create o_Create = (t_Create)GetProcAddress(hModule, "Direct3DCreate8");
	PDIRECT3D8 d3d = o_Create(SDKVersion);

	if (!d3d || o_CreateDevice) return d3d;
	void** vftable = *(void***)d3d;

	if (MH_CreateHook(vftable[15], (void*)hk_CreateDevice, reinterpret_cast<void**>(o_CreateDevice)) != MH_OK)
		throw;
	MH_EnableHook(vftable[15]);

	return d3d;
}
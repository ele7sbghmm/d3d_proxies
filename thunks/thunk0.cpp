#include "pch.h"
#include <iostream>
#include "d3d8.h"
#include "d3dx8.h"
#include "MinHook.h"

#pragma comment(lib, "libMinHook.lib")
#pragma comment(lib, "C:/Users/imw/Desktop/mi/vs/drawText/packages/minhook.1.3.3/lib/native/lib/libMinHook.lib")
//#pragma comment(linker, "/export:Direct3DCreate8=\"C:\\Windows\\SysWOW64\\d3d8.Direct3DCreate8\"")
//#pragma comment(linker, "/export:ValidatePixelShader=\"C:\\Windows\\SysWOW64\\d3d8.ValidatePixelShader\"")
//#pragma comment(linker, "/export:ValidateVertexShader=\"C:\\Windows\\SysWOW64\\d3d8.ValidateVertexShader\"")
//#pragma comment(linker, "/export:DebugSetMute=\"C:\\Windows\\SysWOW64\\d3d8.DebugSetMute\"")

extern "C" void __stdcall DebugSetMute(void) {}
extern "C" void __stdcall ValidatePixelShader(void* a, void* b, void* c, void* d) {}
extern "C" void __stdcall ValidateVertexShader(void* a, void* b, void* c, void* d) {}

HRESULT __stdcall hkQueryInterface(IDirect3D8* pHandle, REFIID riid, void** ppvObj);

typedef HRESULT(__stdcall* tCreateDevice)(IDirect3D8*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice8**);
typedef HRESULT(__stdcall* tEndScene)(IDirect3DDevice8* pDevice);
typedef HRESULT(__stdcall* tQueryInterface)(IDirect3D8*, REFIID, void**);
tCreateDevice oCreateDevice = NULL;
tEndScene oEndScene = NULL;
tQueryInterface oQueryInterface = NULL;


HRESULT __stdcall hkEndScene(IDirect3DDevice8* pDevice) {
	return oEndScene(pDevice);
}

HRESULT __stdcall hkCreateDevice(IDirect3D8* pD3D8, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
	//MessageBoxA(NULL, "", "hkCreateDevice", MB_OK);
	Beep(1000, 5000);

	return oCreateDevice(pD3D8, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
}

extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
	SetEnvironmentVariableA("__COMPAT_LAYER", "RunAsInvoker");

	// 1. Get the real DLL handle
	static HMODULE hRealD3D8 = LoadLibraryA("C:\\Windows\\SysWOW64\\d3d8.dll");
	typedef IDirect3D8* (WINAPI* pDirect3DCreate8)(UINT);
	auto realCreate = (pDirect3DCreate8)GetProcAddress(hRealD3D8, "Direct3DCreate8");

	IDirect3D8* pD3D8 = realCreate(SDKVersion);

	if (pD3D8) {
		uintptr_t* vtable = *(uintptr_t**)pD3D8;

		if (MH_CreateHook((LPVOID)vtable[0], &hkQueryInterface, (LPVOID*)&oQueryInterface) == MH_OK) {
			MH_EnableHook((LPVOID)vtable[0]);
			Beep(440, 200);
			Beep(440, 200);
			Beep(440, 200);
		}

		if (MH_CreateHook((LPVOID)vtable[14], &hkCreateDevice, (LPVOID*)&oCreateDevice) == MH_OK) {
			MH_EnableHook((LPVOID)vtable[14]);

			bool isShimmed = GetModuleHandleA("d3d8thk.dll") != nullptr;
			Beep(isShimmed ? 1500 : 500, 200);
			Beep(isShimmed ? 1500 : 500, 200);
		}
	}

	return realCreate(SDKVersion);
}

HRESULT __stdcall hkQueryInterface(IDirect3D8* pHandle, REFIID riid, void** ppvObj) {
	//MessageBoxA(NULL, "", "hkCreateDevice", MB_OK);
	Beep(600, 1000);

	HRESULT hr = oQueryInterface(pHandle, riid, ppvObj);

	if (hr == S_OK && ppvObj != NULL && *ppvObj != NULL) {
		uintptr_t* vtable = *(uintptr_t**)*ppvObj;
		void* targetCreateDevice = (void*)vtable[14];

		MH_STATUS status = MH_CreateHook(targetCreateDevice, &hkCreateDevice, (LPVOID*)&oCreateDevice);
		if (status == MH_OK || status == MH_ERROR_ALREADY_CREATED) {
			MH_EnableHook(targetCreateDevice);
		}
	}
	return hr;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH: {
			DisableThreadLibraryCalls(hModule);
			if (MH_Initialize() != MH_OK)
				MessageBoxA(NULL, "MinHook Failed!", "Error", MB_OK);

			Sleep(5000);
			HMODULE WINAPI thunk = GetModuleHandleA("d3d8thk.dll");

			char buffer[MAX_PATH];
			sprintf_s(buffer, "drawText loaded! d3d8thk.dll: 0x%p", thunk);
			MessageBoxA(NULL, "", buffer, MB_OK);
			break;
		}
		case DLL_PROCESS_DETACH:
			MH_Uninitialize();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}

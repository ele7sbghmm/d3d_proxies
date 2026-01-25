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
typedef ULONG(__stdcall* tAddRef)(IDirect3D8*);
typedef ULONG(__stdcall* tRelease)(IDirect3D8*);
tCreateDevice oCreateDevice = NULL;
tEndScene oEndScene = NULL;
tQueryInterface oQueryInterface = NULL;
tAddRef oAddRef = NULL;


HRESULT __stdcall hkEndScene(IDirect3DDevice8* pDevice) {
	return oEndScene(pDevice);
}

HRESULT __stdcall hkCreateDevice(IDirect3D8* pD3D8, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) {
	MessageBoxA(NULL, "", "hkCreateDevice", MB_OK);
	Beep(2000, 500);

	return oCreateDevice(pD3D8, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
}

extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
	// 1. Get the real DLL handle
	static HMODULE hRealD3D8 = LoadLibraryA("C:\\Windows\\SysWOW64\\d3d8.dll");
	typedef IDirect3D8* (WINAPI* pDirect3DCreate8)(UINT);
	auto realCreate = (pDirect3DCreate8)GetProcAddress(hRealD3D8, "Direct3DCreate8");

	// 2. IMPORTANT: We hook the ACTUAL code inside the system DLL, 
	// not just the object we are about to create.
	static bool hooked = false;
	if (!hooked) {
		// We find the internal "CreateDevice" by creating a temporary object 
		// just to find where the code lives in the system.
		IDirect3D8* pTemp = realCreate(SDKVersion);
		if (pTemp) {
			uintptr_t* vtable = *(uintptr_t**)pTemp;

			// Hook CreateDevice (Index 14) globally
			MH_CreateHook((LPVOID)vtable[14], &hkCreateDevice, reinterpret_cast<LPVOID*>(&oCreateDevice));
			MH_EnableHook((LPVOID)vtable[14]);

			// Hook QueryInterface (Index 0) globally
			MH_CreateHook((LPVOID)vtable[0], &hkQueryInterface, reinterpret_cast<LPVOID*>(&oQueryInterface));
			MH_EnableHook((LPVOID)vtable[0]);

			pTemp->Release(); // Destroy temp object
			hooked = true;
			//Beep(600, 200); // "Global Hooks Active"
		}
	}

	return realCreate(SDKVersion);
}

ULONG __stdcall hkAddRef(IDirect3D8* pHandle) {
	MessageBoxA(NULL, "", "hkAddRef", MB_OK);

	return oAddRef(pHandle);
}

HRESULT __stdcall hkQueryInterface(IDirect3D8* pHandle, REFIID riid, void** ppvObj) {
	MessageBoxA(NULL, "", "hkQueryInterface", MB_OK);

	HRESULT hr = oQueryInterface(pHandle, riid, ppvObj);

	if (hr == S_OK && ppvObj != NULL && *ppvObj != NULL) {
		// BEEP! We caught the game asking for a new interface
		Beep(800, 100);

		uintptr_t* vtable = *(uintptr_t**)*ppvObj;

		// 2. Hook CreateDevice on THIS specific new object
		// Use MH_CreateHook again. MinHook is smart enough to handle 
		// if the address is the same or different.
		if (vtable[14] != (uintptr_t)hkCreateDevice) {
			MH_CreateHook((LPVOID)vtable[14], &hkCreateDevice, reinterpret_cast<LPVOID*>(&oCreateDevice));
			MH_EnableHook((LPVOID)vtable[14]);
		}
	}
	return hr;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	switch (ul_reason_for_call) {
		case DLL_PROCESS_ATTACH:
			DisableThreadLibraryCalls(hModule);
			if (MH_Initialize() != MH_OK)
				MessageBoxA(NULL, "MinHook Failed!", "Error", MB_OK);
			break;
		case DLL_PROCESS_DETACH:
			MH_Uninitialize();
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		break;
	}

	return TRUE;
}

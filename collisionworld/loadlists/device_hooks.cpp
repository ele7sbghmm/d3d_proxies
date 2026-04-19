#include "pch.h"

#include "device_hooks.h"
#include "globals.h"
#include "hooks.h"
#include "srr2_types.h"
#include "drawing.h"

using tSetRenderState = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice8* _this, D3DRENDERSTATETYPE State, DWORD Value);
using tEndScene = HRESULT(STDMETHODCALLTYPE*)(IDirect3DDevice8* _this);
using tCreateDevice = HRESULT(STDMETHODCALLTYPE*)(IDirect3D8* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface);

tSetRenderState oSetRenderState{};
tEndScene oEndScene{};
tCreateDevice oCreateDevice{};

auto STDMETHODCALLTYPE hkSetRenderState(IDirect3DDevice8* _this, D3DRENDERSTATETYPE State, DWORD Value) {
	return oSetRenderState(_this, State, Value);
}

auto STDMETHODCALLTYPE hkEndScene(IDirect3DDevice8* _this) {
	return oEndScene(_this);
}

auto STDMETHODCALLTYPE hkCreateDevice(IDirect3D8* _this, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface) -> HRESULT {
	HRESULT hr = oCreateDevice(_this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

	if (SUCCEEDED(hr) && ppReturnedDeviceInterface && *ppReturnedDeviceInterface) {
		Device = *ppReturnedDeviceInterface;
		void** vtable = *(void***)Device;

		MH_CreateHook(vtable[35], (LPVOID)hkEndScene, reinterpret_cast<LPVOID*>(&oEndScene));
		MH_CreateHook(vtable[50], (LPVOID)hkSetRenderState, reinterpret_cast<LPVOID*>(&oSetRenderState));
		MH_EnableHook(MH_ALL_HOOKS);
	}
	return hr;
}

static auto GetD3D8Module() -> HMODULE {
	static HMODULE hModule = []() {
		char path[MAX_PATH];
		GetSystemDirectory(path, MAX_PATH);
		strcat_s(path, "\\d3d8.dll");
		return LoadLibrary(path);
		}();
	return hModule;
}

extern "C" auto STDMETHODCALLTYPE Direct3DCreate8(UINT SDKVersion) -> IDirect3D8* {
	HMODULE hModule = GetD3D8Module();
	if (!hModule) return nullptr;

	typedef IDirect3D8* (STDMETHODCALLTYPE* tDirect3DCreate8)(UINT SDKVersion);
	static tDirect3DCreate8 oDirect3DDCreate8{};
	if (!oDirect3DDCreate8)
		oDirect3DDCreate8 = (tDirect3DCreate8)GetProcAddress(hModule, "Direct3DCreate8");


	IDirect3D8* pDirect3D8 = oDirect3DDCreate8(SDKVersion);
	if (!pDirect3D8) return nullptr;

	void** vtable = *(void***)pDirect3D8;

	MH_Initialize();
	CreateHooks();
	//if (MH_CreateHook(vtable[15], (LPVOID)hkCreateDevice, reinterpret_cast<LPVOID*>(&oCreateDevice)) == MH_OK)
		//MH_EnableHook(vtable[15]);

	return pDirect3D8;
}

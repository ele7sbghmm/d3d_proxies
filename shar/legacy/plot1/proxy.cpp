#include "pch.h"

#include "globals.h"
#include "hooks.h"
#include "gui.h"

auto CreateHooks() -> void;

WNDPROC oWndProc;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;
	return CallWindowProc(oWndProc, hWnd, msg, wParam, lParam);
}

using tEndScene = HRESULT(STDMETHODCALLTYPE*)(PDIRECT3DDEVICE8);
using tCreateDevice = HRESULT(STDMETHODCALLTYPE*)(
	PDIRECT3D8, UINT, D3DDEVTYPE, HWND, DWORD,
	D3DPRESENT_PARAMETERS*, PDIRECT3DDEVICE8*);

tEndScene oEndScene = nullptr;
tCreateDevice oCreateDevice = nullptr;

HRESULT STDMETHODCALLTYPE hkEndScene(PDIRECT3DDEVICE8 _this) {
	Render3DGui();

	return oEndScene(_this);
}

auto CreateDeviceHooks(void** vftable) -> void {
	if (MH_CreateHook(vftable[35], (void*)hkEndScene, reinterpret_cast<void**>(&oEndScene)) != MH_OK) throw;
	MH_EnableHook(vftable[35]); 
}

HRESULT STDMETHODCALLTYPE hkCreateDevice(
		PDIRECT3D8 _this,
		UINT Adapter, D3DDEVTYPE DeviceType,
		HWND hWnd, DWORD BehaviorFlags,
		D3DPRESENT_PARAMETERS* pPresentationParameters,
		PDIRECT3DDEVICE8* ppReturnedDeviceInterface) {
	HRESULT hr = oCreateDevice(_this, Adapter, DeviceType, hWnd, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	if (FAILED(hr)) return hr;

	g_device = *ppReturnedDeviceInterface;
	void** vftable = *(void***)g_device;
	CreateDeviceHooks(vftable);
	CreateHooks();

	ImGui::CreateContext();
	ImPlot::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX8_Init(g_device);

	oWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)hkWndProc);

	return hr;
}

extern "C" LPDIRECT3D8 STDMETHODCALLTYPE Direct3DCreate8(UINT SDKVersion) {
	static HMODULE s_hD3DModule = []() {
		char path[MAX_PATH];
		GetSystemDirectory(path, MAX_PATH);
		strcat_s(path, "\\d3d8.dll");
		return LoadLibrary(path);
		}();

	using tDirect3DCreate8 = PDIRECT3D8(STDMETHODCALLTYPE*)(UINT);
	tDirect3DCreate8 oDirect3DCreate8 = (tDirect3DCreate8)GetProcAddress(s_hD3DModule, "Direct3DCreate8");

	PDIRECT3D8 pDirect3D8 = oDirect3DCreate8(SDKVersion);
	if (!pDirect3D8 || oCreateDevice) return pDirect3D8;

	void** vftable = *(void***)pDirect3D8;

	if (MH_Initialize() != MH_OK) throw;
	if (MH_CreateHook(vftable[15], (void*)hkCreateDevice, reinterpret_cast<void**>(&oCreateDevice)) != MH_OK) throw;
	MH_EnableHook(vftable[15]);

	return pDirect3D8;
}

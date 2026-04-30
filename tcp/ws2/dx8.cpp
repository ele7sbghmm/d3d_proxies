#include "pch.h"

#include <d3d8.h>

IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion) {
	char path[MAX_PATH];
	GetSystemDirectory(path, MAX_PATH);
	strcat_s(path, "\\d3d8.dll");
	HMODULE hModule = LoadLibraryA(path);
	if (!hModule) return nullptr;

	using Direct3DCreate8_t = IDirect3D8*(WINAPI*)(UINT);
	Direct3DCreate8_t oDirect3DCreate8
		= (Direct3DCreate8_t)GetProcAddress(hModule, "Direct3DCreate8");
	
	if (!oDirect3DCreate8) return nullptr;

	IDirect3D8* d3d = oDirect3DCreate8(SDKVersion);

	return d3d;
}

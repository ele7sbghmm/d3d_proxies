#pragma once

#include "shar.h"
#include "context.h"
//#include "mixins.h"

namespace mod {
	class RenderManager;
	class Character;
}

inline IDirect3DDevice8* g_device{};
inline RenderContext g_context{};

namespace singletons {
	/*
	namespace p3d {
		inline mod::d3dDevice** device = reinterpret_cast<mod::d3dDevice**>(0x65ef54);
		inline mod::d3dDisplay** display = reinterpret_cast<mod::d3dDisplay**>(0x65ef58);
		inline mod::d3dContext** pddi = reinterpret_cast<mod::d3dContext**>(0x65ef5c);
		// ...
		inline mod::tPlatform** platform = reinterpret_cast<mod::tPlatform**>(0x65ef6c);
		inline mod::tContext** context = reinterpret_cast<mod::tContext**>(0x65ef70);
	}
	inline mod::Game** game = reinterpret_cast<mod::Game**>(0x6c8fc8);
	inline mod::Win32Platform** win32platform = reinterpret_cast<mod::Win32Platform**>(0x6c8fc8);
	*/
	inline mod::RenderManager** rendermanager = reinterpret_cast<mod::RenderManager**>(0x6c87b4);
	inline mod::Character** character = reinterpret_cast<mod::Character**>(0x6c922c);
}

namespace config {
	 /* FENCE */
	inline bool			draw_FenceLoadLists				= true;
	inline bool			draw_FenceSpatialTree			= true;
	inline D3DXCOLOR	color_FenceOpaqueInactive		= (D3DXCOLOR)0xffffffff;
	inline D3DXCOLOR	color_FenceOpaqueActive			= (D3DXCOLOR)0xffff0000;
	inline D3DXCOLOR	color_FenceOpaqueOverflow		= (D3DXCOLOR)0xff00ff00;
	inline D3DXCOLOR	color_FenceTranslucentInactive	= (D3DXCOLOR)0x40ffffff;
	inline D3DXCOLOR	color_FenceTranslucentActive	= (D3DXCOLOR)0x40ff0000;
	inline D3DXCOLOR	color_FenceTranslucentOverflow	= (D3DXCOLOR)0x4000ff00;
}

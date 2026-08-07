#pragma once

#include <vector>
#include <string>

#include "d3dx9.h"
#pragma comment(lib, "x86/d3dx9.lib")

#include "types.hpp"

struct Vtx
{
	float x, y, z;
	D3DCOLOR c;
	float u, v;
};

inline bool g_slow = false;
inline GfxViewParms g_parms = {};

class Drawer
{
public:
	IDirect3DDevice9* m_device = nullptr;
	IDirect3DVertexBuffer9* m_vb = nullptr;
	IDirect3DStateBlock9* m_sb = nullptr;
	IDirect3DVertexShader9* m_vs = nullptr;
	Vtx* m_v = nullptr;
	size_t m_n = 0;

	bool m_inited = false;

	Drawer() = default;
	Drawer(IDirect3DDevice9* device) : m_device(device) {}
	void Init()
	{
		m_device->CreateStateBlock(D3DSBT_ALL, &m_sb);

		std::string a = 
			"vs_1_1\n"
			"def c4, 1.44269502, 1, 0, 0\n"
			"dcl_position v0\n"
			"dcl_color v1\n"
			"dcl_texcoord v2\n"
			"dp4 r0.x, v0, c8\n"
			"dp4 r0.y, v0, c9\n"
			"dp4 r0.z, v0, c10\n"
			"dp3 r0.x, r0, r0\n"
			"rsq r0.w, r0.x\n"
			"rcp r0.w, r0.w\n"
			"mul r0.w, r0.w, c11.z\n"
			"mul r0.w, r0.w, c4.x\n"
			"exp r0.w, r0.w\n"
			"dp4 oPos.x, v0, c0\n"
			"add r0.w, -r0.w, c4.y\n"
			"dp4 oPos.y, v0, c1\n"
			"max r0.w, r0.w, c4.z\n"
			"dp4 oPos.z, v0, c2\n"
			"min r0.w, r0.w, c4.y\n"
			"dp4 oPos.w, v0, c3\n"
			"mul oD1.xyz, r0.w, c21\n"
			"mov oD0, v1\n"
			"mov oT0.xy, v2\n";

		LPD3DXBUFFER c = nullptr;
		HRESULT hr = D3DXAssembleShader(a.c_str(), a.length(), nullptr, nullptr, 0, &c, nullptr);

		m_device->CreateVertexShader((const DWORD*)c, &m_vs);

		m_inited = true;
	}
	void Flush()
	{
		if (!m_inited)
			Init();

		D3DCOLOR blue = 0xff0000ff;
		D3DCOLOR green = 0xff0ff00;
		D3DCOLOR red = 0xffff0000;
		Vtx v[6] = {
			{ 0.f, -100.f,  100000.f, red, 0.f, 0.f },
			{ 0.f, -100.f, -100000.f, red, 0.f, 0.f },
			{ 0.f,    0.f,  100000.f, green, 0.f, 0.f },
			{ 0.f,    0.f, -100000.f, green, 0.f, 0.f },
			{ 0.f,  100.f,  100000.f, blue, 0.f, 0.f },
			{ 0.f,  100.f, -100000.f, blue, 0.f, 0.f }
		};

		if (g_slow)
		{
			m_device->SetVertexShader((IDirect3DVertexShader9*)0x6d1c24c0);
			UINT size = 0;
			m_vs->GetFunction(nullptr, &size);
			std::vector<DWORD> code(size / 4);
			m_vs->GetFunction(code.data(), &size);
			LPD3DXBUFFER nasm = nullptr;
			std::string res = "nope";
			D3DXDisassembleShader(code.data(), FALSE, nullptr, &nasm);
			res = (const char*)nasm->GetBufferPointer();

		}

		m_device->SetVertexShader(m_vs);

		m_sb->Capture();
		m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
		m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX0);
		m_device->DrawPrimitiveUP(D3DPT_LINELIST, 3, v, sizeof(Vtx));
		m_sb->Apply();
		
	}
};

inline Drawer g_draw{};

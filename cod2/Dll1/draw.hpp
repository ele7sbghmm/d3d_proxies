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
};

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

		m_inited = true;
	}
	void Flush()
	{
		if (!m_inited)
			Init();

		D3DCOLOR c = 0xffffffff;
		Vtx v[6] = {
			//{  100000.f,       0.f,     666.f, c },
			//{ -100000.f,       0.f,     666.f, c },
			//{       0.f,  100000.f,     666.f, c },
			//{       0.f, -100000.f,     666.f, c },
			{       0.f,       0.f,  100000.f, c },
			{       0.f,       0.f, -100000.f, c }
		};

		//m_device->GetVertexShader(&m_vs);
		//int i = *(int*)m_vs;
		//if (i != 0x6d1c24c0)
		//	return;
		//UINT size = 0;
		//m_vs->GetFunction(nullptr, &size);
		//std::vector<DWORD> code(size / 4);
		//m_vs->GetFunction(code.data(), &size);
		//LPD3DXBUFFER nasm = nullptr;
		//std::string res = "nope";
		//D3DXDisassembleShader(code.data(), FALSE, nullptr, &nasm);
		//res = (const char*)nasm->GetBufferPointer();

		m_sb->Capture();
		m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
		m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		//m_device->SetVertexShader(nullptr);
		//m_device->SetPixelShader(nullptr);
		m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		m_device->DrawPrimitiveUP(D3DPT_LINELIST, 3, v, sizeof(Vtx));
		m_sb->Apply();
	}
};

inline Drawer g_draw{};

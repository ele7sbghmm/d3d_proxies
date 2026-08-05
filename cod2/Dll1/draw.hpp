#pragma once

#include "d3d9.h"

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

		Vtx v[6] = {
			{  100000.f,       0.f,     666.f, 0xffff0000 },
			{ -100000.f,       0.f,     666.f, 0xffff0000 },
			{       0.f,  100000.f,     666.f, 0xffff0000 },
			{       0.f, -100000.f,     666.f, 0xffff0000 },
			{       0.f,       0.f,  100000.f, 0xffff0000 },
			{       0.f,       0.f, -100000.f, 0xffff0000 }
		};
		
		m_sb->Capture();
		//m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
		//m_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		//m_device->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
		//m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		//m_device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
		//m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		//m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

		//m_device->SetVertexShader(nullptr);
		//m_device->SetPixelShader(nullptr);
		m_device->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
		m_device->DrawPrimitiveUP(D3DPT_LINELIST, 3, v, sizeof(Vtx));
		m_sb->Apply();
	}
};

inline Drawer g_draw{};

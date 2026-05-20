#pragma once

#include <d3dx8.h>

struct Vtx {
	D3DXVECTOR3 xyz;
	D3DCOLOR c;
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

class Data {
public:
	IDirect3DVertexBuffer8* m_vb{};
	Vtx* m_vtxData = nullptr;
	size_t m_vtxCount = 0;
	static constexpr size_t MAX_VTX = 10000;

	Data() = default;
	void Init() {
		IDirect3DDevice8* device = shar::d3dDisplay::get_()->d3dDevice;
		
		device->CreateVertexBuffer(10000 * sizeof(Vtx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, Vtx::FVF, D3DPOOL_DEFAULT, &m_vb);
	
		Begin();
	}
	void Begin() {
		m_vtxCount = 0;
		m_vtxData = nullptr;
		m_vb->Lock(0, 0, (BYTE**)&m_vtxData, D3DLOCK_NOOVERWRITE);
	}
	void End() { m_vb->Unlock(); }
	void AddLine(D3DXVECTOR3& a, D3DXVECTOR3& b, D3DCOLOR c)  {
		m_vtxData[m_vtxCount++] = { a, c };
		m_vtxData[m_vtxCount++] = { b, c };
	}
	void AddCircle(D3DXVECTOR3& center, float radius, D3DCOLOR c) {
		size_t steps = 36;

		for (size_t i = 0; i <= steps; ++i) {
			float theta = (360.f / steps) * (D3DX_PI / 180.f) * i;
			float x = sinf(theta) * radius + center.x;
			float z = cosf(theta) * radius + center.z;

			m_vtxData[m_vtxCount++] = { { x, center.y, z }, c };
			if (i == 0 || i == steps)
				continue;

			m_vtxData[m_vtxCount++] = { { x, center.y, z }, c };
		}
	}
};

class Draw {
public:
	DWORD m_sbt{};

	Data m_data{};

	bool inited = false;

	Draw() = default;
	void Init() {
		if (inited) return;

		IDirect3DDevice8* device = shar::d3dDisplay::get_()->d3dDevice;

		device->CreateStateBlock(D3DSBT_ALL, &m_sbt);
		m_data.Init();

		inited = true;
	}
	void DrawStuff() {
		Init();

		m_data.End();
		
		IDirect3DDevice8* device = shar::d3dDisplay::get_()->d3dDevice;
		device->CaptureStateBlock(m_sbt);

		device->SetStreamSource(0, m_data.m_vb, sizeof(Vtx));
		device->SetVertexShader(Vtx::FVF);

		device->SetRenderState(D3DRS_ZENABLE, FALSE);
		device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		device->SetRenderState(D3DRS_LIGHTING, FALSE);
		device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->DrawPrimitive(D3DPT_LINELIST, 0, m_data.m_vtxCount / 2);

		device->ApplyStateBlock(m_sbt);

		m_data.Begin();
	}
};

inline Draw g_draw;

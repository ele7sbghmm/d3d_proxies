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
	static constexpr size_t MAX_VTX = 1000000;

	Data() = default;
	void Init() {
		IDirect3DDevice8* device = shar::d3dDisplay::get_()->d3dDevice;
		
		device->CreateVertexBuffer(MAX_VTX * sizeof(Vtx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, Vtx::FVF, D3DPOOL_DEFAULT, &m_vb);
	
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
	void DrawCross(D3DXVECTOR3& center, float extent, D3DCOLOR color) {
		D3DXVECTOR3 x = center; D3DXVECTOR3 nx = center;
		D3DXVECTOR3 y = center; D3DXVECTOR3 ny = center;
		D3DXVECTOR3 z = center; D3DXVECTOR3 nz = center;
		x.x += extent; nx.x -= extent;
		y.y += extent; ny.y -= extent;
		z.z += extent; nz.z -= extent;
		g_draw.m_data.AddLine(x, nx, color);
		g_draw.m_data.AddLine(y, ny, color);
		g_draw.m_data.AddLine(z, nz, color);
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

		m_data.AddLine(*shar::TriggerVolumeTracker::p_sP1, *shar::TriggerVolumeTracker::p_sP2, 0xffff0000);
		m_data.AddLine(*shar::TriggerVolumeTracker::p_sP3, *shar::TriggerVolumeTracker::p_sP4, 0xffff0000);

		m_data.End();
		
		IDirect3DDevice8* device = shar::d3dDisplay::get_()->d3dDevice;
		device->CaptureStateBlock(m_sbt);

		device->SetStreamSource(0, m_data.m_vb, sizeof(Vtx));
		device->SetVertexShader(Vtx::FVF);

		device->SetTexture(0, NULL);
		device->SetRenderState(D3DRS_ZENABLE, FALSE);
		device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		device->SetRenderState(D3DRS_LIGHTING, FALSE);
		device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		device->DrawPrimitive(D3DPT_LINELIST, 0, m_data.m_vtxCount / 2);

		device->ApplyStateBlock(m_sbt);

		m_data.Begin();
	}
	void DrawTriggerVolume(shar::TriggerVolume* volume, float rangeSq, float distSq, float sumRadius) {
		Init();

		bool active = distSq < rangeSq;
		if (!active) return;

		shar::TriggerVolume::Type type = volume->GetType();
		if (type == shar::TriggerVolume::RECTANGLE)
			DrawRectTriggerVolume(volume, active);
	}
	void DrawRectTriggerVolume(shar::TriggerVolume* volume, bool active) {
		if (!active) return;

		D3DCOLOR color = active ? 0xff00ffff : 0xff0000ff;
		shar::RectTriggerVolume* rect = static_cast<shar::RectTriggerVolume*>(volume);
		
		D3DXVECTOR3& center = rect->mPosition;
		D3DXVECTOR3 aX = rect->mExtentX * rect->mAxisX;
		D3DXVECTOR3 aY = rect->mExtentY * rect->mAxisY;
		D3DXVECTOR3 aZ = rect->mExtentZ * rect->mAxisZ;
		D3DXVECTOR3 c[8] = {
			center + aX + aY + aZ,
			center - aX + aY + aZ,
			center + aX - aY + aZ,
			center - aX - aY + aZ,
			center + aX + aY - aZ,
			center - aX + aY - aZ,
			center + aX - aY - aZ,
			center - aX - aY - aZ,
		};

		m_data.AddLine(c[0], c[1], color);
		m_data.AddLine(c[1], c[3], color);
		m_data.AddLine(c[3], c[2], color);
		m_data.AddLine(c[2], c[0], color);
		m_data.AddLine(c[4], c[5], color);
		m_data.AddLine(c[5], c[7], color);
		m_data.AddLine(c[7], c[6], color);
		m_data.AddLine(c[6], c[4], color);
		m_data.AddLine(c[0], c[4], color);
		m_data.AddLine(c[1], c[5], color);
		m_data.AddLine(c[2], c[6], color);
		m_data.AddLine(c[3], c[7], color);
	}
};

inline Draw g_draw;

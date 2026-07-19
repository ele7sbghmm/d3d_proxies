#pragma once

#include <d3dx8.h>
#pragma comment(lib, "d3dx8.lib")

#include <wrl/client.h>
template <typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

struct Vtx {
	D3DXVECTOR3 xyz;
	D3DCOLOR c;
	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

class Line {
public:
	IDirect3DVertexBuffer8* m_vb{};
	Vtx* m_vtxData = nullptr;
	size_t m_vtxCount = 0;
	static constexpr size_t MAX_VTX = 1000000;

	Line() = default;
	void Init(IDirect3DDevice8* device) {
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
	void DrawTeethLine(D3DXVECTOR3& start, D3DXVECTOR3& end, float extent, D3DCOLOR color) {
		D3DXVECTOR3 delta = end - start;
		D3DXVECTOR3 dir;
		D3DXVec3Normalize(&dir, &delta);
		D3DXVECTOR3 startIn = start + dir * extent;
		D3DXVECTOR3 endIn = end - dir * extent;
		AddLine(start, startIn, color);
		AddLine(end, endIn, color);
	}
	void DrawCross(D3DXVECTOR3& center, float extent, D3DCOLOR color) {
		D3DXVECTOR3 x = center; D3DXVECTOR3 nx = center;
		D3DXVECTOR3 y = center; D3DXVECTOR3 ny = center;
		D3DXVECTOR3 z = center; D3DXVECTOR3 nz = center;
		x.x += extent; nx.x -= extent;
		y.y += extent; ny.y -= extent;
		z.z += extent; nz.z -= extent;
		AddLine(x, nx, color);
		AddLine(y, ny, color);
		AddLine(z, nz, color);
	}
};


class Quad {
public:
	IDirect3DVertexBuffer8* m_vb{};
	Vtx* m_vtxData = nullptr;
	size_t m_vtxCount = 0;
	static constexpr size_t MAX_VTX = 1000000;

	Quad() = default;
	void Init(IDirect3DDevice8* device) {
		device->CreateVertexBuffer(MAX_VTX * sizeof(Vtx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, Vtx::FVF, D3DPOOL_DEFAULT, &m_vb);

		Begin();
	}
	void Begin() {
		m_vtxCount = 0;
		m_vtxData = nullptr;
		m_vb->Lock(0, 0, (BYTE**)&m_vtxData, D3DLOCK_NOOVERWRITE);
	}
	void End() { m_vb->Unlock(); }
	void DrawQuad(D3DXVECTOR3& a, D3DXVECTOR3& b, D3DXVECTOR3& c, D3DXVECTOR3& d, D3DCOLOR color) {
		m_vtxData[m_vtxCount++] = { a, color };
		m_vtxData[m_vtxCount++] = { b, color };
		m_vtxData[m_vtxCount++] = { c, color };
		m_vtxData[m_vtxCount++] = { c, color };
		m_vtxData[m_vtxCount++] = { b, color };
		m_vtxData[m_vtxCount++] = { d, color };
	}
	void DrawQuadDoubleSided(D3DXVECTOR3& a, D3DXVECTOR3& b, D3DXVECTOR3& c, D3DXVECTOR3& d, D3DCOLOR colorOut, D3DCOLOR colorIn) {
		DrawQuad(a, b, c, d, colorOut);
		DrawQuad(b, a, d, c, colorIn);
	}
};


class Draw {
public:
	DWORD m_sbt{};

	Line m_line{};
	Quad m_quad{};

	ComPtr<IDirect3DDevice8> m_device;

	bool inited = false;

	Draw() = default;
	void Init() {
		if (inited) return;

		IDirect3DDevice8* device = shar::d3dDisplay::get_()->d3dDevice;

		device->CreateStateBlock(D3DSBT_ALL, &m_sbt);
		m_line.Init(device);
		m_quad.Init(device);

		m_device = device;

		inited = true;
	}
	void DrawStuff() {
		Init();

		m_line.End();
		m_quad.End();
		
		m_device->CaptureStateBlock(m_sbt);

		m_device->SetVertexShader(Vtx::FVF);
		m_device->SetTexture(0, NULL);
		m_device->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
		m_device->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);

		m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

		m_device->SetStreamSource(0, m_quad.m_vb, sizeof(Vtx));
		m_device->SetRenderState(D3DRS_ZENABLE, TRUE);
		m_device->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
		m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_quad.m_vtxCount / 3);

		m_device->SetStreamSource(0, m_line.m_vb, sizeof(Vtx));
		m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
		m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		m_device->SetRenderState(D3DRS_ZBIAS, 1);
		m_device->DrawPrimitive(D3DPT_LINELIST, 0, m_line.m_vtxCount / 2);

		m_device->ApplyStateBlock(m_sbt);

		m_line.Begin();
		m_quad.Begin();
	}
	void DrawTriggerVolume(shar::TriggerVolume* volume, int al) {
		Init();

		bool active = al > 0;
		//if (!active) return;

		auto type = volume->GetType();
		if (type == shar::TriggerVolume::RECTANGLE)
			DrawRectTriggerVolume(volume, active);
	}
	void DrawRectTriggerVolume(shar::TriggerVolume* volume, bool active) {
		D3DCOLOR color = active ? 0xff00ffff : 0x01000000;
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

		m_line.AddLine(c[0], c[1], color);
		m_line.AddLine(c[1], c[3], color);
		m_line.AddLine(c[3], c[2], color);
		m_line.AddLine(c[2], c[0], color);
		m_line.AddLine(c[4], c[5], color);
		m_line.AddLine(c[5], c[7], color);
		m_line.AddLine(c[7], c[6], color);
		m_line.AddLine(c[6], c[4], color);
		m_line.AddLine(c[0], c[4], color);
		m_line.AddLine(c[1], c[5], color);
		m_line.AddLine(c[2], c[6], color);
		m_line.AddLine(c[3], c[7], color);

		D3DCOLOR out = active ? 0x4000ffff : 0x10000000;
		D3DCOLOR in  = active ? 0x2000ffff : 0x08000000;
		m_quad.DrawQuad(c[0], c[1], c[2], c[3], out);
		m_quad.DrawQuad(c[1], c[0], c[5], c[4], out);
		m_quad.DrawQuad(c[0], c[2], c[4], c[6], out);
		m_quad.DrawQuad(c[3], c[1], c[7], c[5], out);
		m_quad.DrawQuad(c[4], c[6], c[5], c[7], out);
		m_quad.DrawQuad(c[2], c[3], c[6], c[7], out);
		m_quad.DrawQuad(c[1], c[0], c[3], c[2], in);
		m_quad.DrawQuad(c[0], c[1], c[4], c[5], in);
		m_quad.DrawQuad(c[2], c[0], c[6], c[4], in);
		m_quad.DrawQuad(c[1], c[3], c[5], c[7], in);
		m_quad.DrawQuad(c[6], c[4], c[7], c[5], in);
		m_quad.DrawQuad(c[3], c[2], c[7], c[6], in);
	}
};

inline Draw g_draw;

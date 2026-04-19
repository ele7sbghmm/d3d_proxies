#include "pch.h"

#include <d3dx8.h>

#include "srr2.h"

struct VertexRHW {
	float x, y, z, rhw;
	D3DCOLOR c;
};
struct Vertex {
	float x, y, z;
	D3DCOLOR c;
};
DWORD BLACK = D3DCOLOR_XRGB(0, 0, 0);
DWORD WHITE = D3DCOLOR_XRGB(255, 255, 255);
DWORD RED = D3DCOLOR_XRGB(255, 0, 0);
DWORD GREEN = D3DCOLOR_XRGB(0, 255, 0);
DWORD BLUE = D3DCOLOR_XRGB(0, 0, 255);
DWORD GREENA = D3DCOLOR_ARGB(128, 0, 255, 0);

namespace CameraGlobals {
	D3DXMATRIX view;
	D3DXMATRIX projection;
	D3DXMATRIX viewProjection;
	float nearZ{};
	float farZ{};
	int screenWidth = 800;
	int screenHeight = 600;
	void Update(SuperCam* sc) {
		if (!sc || !sc->mCamera) return;
		
		float aspect = (float)screenWidth / (float)screenHeight;
		float fovX = sc->mSCFOV;
		float fovY = 2.f * atanf(tanf(fovX / 2.f) / aspect);

		D3DXVECTOR3 eye = reinterpret_cast<D3DXVECTOR3&>(sc->mCamera->position);
		D3DXVECTOR3 at = reinterpret_cast<D3DXVECTOR3&>(sc->mCamera->target);
		D3DXVECTOR3 up(0.f, 1.f, 0.f);
		nearZ = sc->mSCNearPlane;
		farZ = sc->mSCFarPlane;
		
		D3DXVECTOR3 delta = eye - at;
		if (D3DXVec3LengthSq(&delta) < .001f) return;

		D3DXMatrixLookAtLH(&view, &eye, &at, &up);
		D3DXMatrixPerspectiveFovLH(&projection, fovY, aspect, nearZ, farZ);
		D3DXMatrixMultiply(&viewProjection, &view, &projection);
	}
}

float GetNormalizedZ(float worldDistance) {
	if (worldDistance <= CameraGlobals::nearZ) return 0.f;
	if (worldDistance >= CameraGlobals::farZ) return 1.f;

	// Standard D3D Depth formula
	return (CameraGlobals::farZ / (CameraGlobals::farZ - CameraGlobals::nearZ))
		* (1.f - (CameraGlobals::nearZ / worldDistance));
}

bool WorldToScreen2(const D3DXVECTOR3& worldPos, D3DXVECTOR2& screenOut) {
	D3DXVECTOR4 clipSpacePos;
	D3DXVec3Transform(&clipSpacePos, &worldPos, &CameraGlobals::viewProjection);
	if (clipSpacePos.w < .1f) return false;

	D3DXVECTOR3 ndc = D3DXVECTOR3(
		clipSpacePos.x / clipSpacePos.w,
		clipSpacePos.y / clipSpacePos.w,
		clipSpacePos.z / clipSpacePos.w
	);

	screenOut.x = (ndc.x + 1.f) * .5f * CameraGlobals::screenWidth;
	screenOut.y = (1.f - ndc.y) * .5f * CameraGlobals::screenHeight;

	return true;
}

bool WorldToScreen3(const D3DXVECTOR3& worldPos, D3DXVECTOR3& screenOut) {
	D3DXVECTOR4 clipSpacePos;
	D3DXVec3Transform(&clipSpacePos, &worldPos, &CameraGlobals::viewProjection);
	if (clipSpacePos.w < .1f) return false;

	float invW = 1.f / clipSpacePos.w;
	screenOut.x = (clipSpacePos.x * invW + 1.f) * .5f * CameraGlobals::screenWidth;
	screenOut.y = (1.f - clipSpacePos.y * invW) * .5f * CameraGlobals::screenHeight;

	float z = clipSpacePos.z * invW;

	if (z < 0.001f) z = 0.001f;

	screenOut.z = z;
	return true;
}

void DrawLine3(D3DXVECTOR3 start, D3DXVECTOR3 end, D3DCOLOR color) {
	// Subtract a tiny epsilon (like 0.0001) from Z to bring the line 
    // slightly "closer" to the camera than the building it's standing next to.
	VertexRHW line[2] = {
		{ start.x, start.y, start.z - 0.0001f, 1.0f, color },
		{ end.x, end.y, end.z - 0.0001f, 1.0f, color }
	};

	//pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_TRUE);
	//pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	//pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);

	pDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, line, sizeof(VertexRHW));
}

void DrawBox(float x, float y, float size, D3DCOLOR color) {
	float halfSize = size / 2.f;
	//float z = CameraGlobals::nearZ;
	float z = .8f;
	VertexRHW rect[4] = {
		{ x - halfSize, y - halfSize, z, 1.f, color }, { x + halfSize, y - halfSize, z, 1.f, color },
		{ x - halfSize, y + halfSize, z, 1.f, color }, { x + halfSize, y + halfSize, z, 1.f, color }
	};

	pDevice->SetTexture(0, NULL);
	pDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, rect, sizeof(VertexRHW));
}

void TestZBuffer() {
	IDirect3DBaseTexture8* oldTexture;
	pDevice->GetTexture(0, &oldTexture);
	//pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pDevice->SetTexture(0, NULL);

	DWORD oldSrcBlend, oldDestBlend;
	pDevice->GetRenderState(D3DRS_SRCBLEND, &oldSrcBlend);
	pDevice->GetRenderState(D3DRS_DESTBLEND, &oldDestBlend);
	//pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	//pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//pDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	//pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
	
	DWORD oldZEnable;
	pDevice->GetRenderState(D3DRS_ZENABLE, &oldZEnable);
	pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	pDevice->SetVertexShader(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);

	float n(.9f), f(1.f);
	float l(0), r(500), u(0), d(600);
	VertexRHW verts[4] = {
		{ l, u, n, 1.f, GREEN }, { r, u, f, 1.f, GREEN },
		{ l, d, n, 1.f, GREEN }, { r, d, f, 1.f, GREEN }
	};
	pDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, verts, sizeof(VertexRHW));

	pDevice->SetRenderState(D3DRS_SRCBLEND, oldSrcBlend);
	pDevice->SetRenderState(D3DRS_DESTBLEND, oldDestBlend);

	pDevice->SetTexture(0, oldTexture);

	pDevice->SetRenderState(D3DRS_ZENABLE, oldZEnable);

}

void Clear() {}

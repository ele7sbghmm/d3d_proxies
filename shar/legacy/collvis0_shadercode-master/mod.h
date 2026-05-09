#pragma once

#include <d3dx8.h>
#include "shar.h"

inline IDirect3DDevice8* g_device;

class Context {
public:
	Fence* fence;
};

class Fence {
public:
	static constexpr usize MAX_FENCES = 5000;

	DWORD dwFenceDecl[8] = {
		D3DVSD_STREAM(0),
		D3DVSD_REG(3, D3DVSDT_FLOAT2),
		D3DVSD_STREAM(1),
		D3DVSD_SKIP(14),
		D3DVSD_REG(0, D3DVSDT_FLOAT3),
		D3DVSD_REG(1, D3DVSDT_FLOAT3),
		D3DVSD_REG(2, D3DVSDT_FLOAT3),
		D3DVSD_END()
	};

	struct Selector { float isEnd; float isTop; };
	Selector templateData[6] = {
		{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f},
		{1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}
	};

	IDirect3DVertexBuffer8* pFenceVB = nullptr;
	IDirect3DVertexBuffer8* pStaticTemplateVB = nullptr;
	IDirect3DIndexBuffer8* pInstancingIB = nullptr;
	DWORD hFenceShader;

	Fence(IDirect3DDevice8* device) {
		//g_device->CreateVertexShader(dwFenceDecl, pShaderBinary, &hFenceShader, 0);
		device->CreateVertexBuffer(sizeof(FenceEntityDSG) * MAX_FENCES, 0, 0, D3DPOOL_MANAGED, &pFenceVB);
		device->CreateVertexBuffer(sizeof(templateData), 0, 0, D3DPOOL_MANAGED, &pStaticTemplateVB);
		device->CreateIndexBuffer(MAX_FENCES * 6 * sizeof(u16), D3DUSAGE_WRITEONLY, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pInstancingIB);

		void* pData;
		pStaticTemplateVB->Lock(0, 0, (BYTE**)&pData, 0);
		memcpy(pData, templateData, sizeof(templateData));
		pStaticTemplateVB->Unlock();

		u16* pIndices = nullptr;
		pInstancingIB->Lock(0, 0, (BYTE**)&pIndices, 0);
		for (u16 i = 0; i < MAX_FENCES; ++i) {
			pIndices[i * 6 + 0] = i;
			pIndices[i * 6 + 1] = i;
			pIndices[i * 6 + 2] = i;
			pIndices[i * 6 + 3] = i;
			pIndices[i * 6 + 4] = i;
			pIndices[i * 6 + 5] = i;
		}
		pInstancingIB->Unlock();

		const char* fenceShaderASM = {
			"vs.1.1\n"
			"mul r0, v0, (1 - v3.x)\n"
			"mad r0, v1, v3.x, r0\n"
			"mul r1.y, c4.x, v3.y\n"
			"mad r0.y, c4.y, (1 - v3.y), r1.y\n"
			//"m4x4 oPos, r0, c0\n"
			"mov oD0, c5\n"
		};
		ID3DXBuffer* pCode = nullptr;
		ID3DXBuffer* pErrors = nullptr;
		hFenceShader = 0;

		D3DXAssembleShader(fenceShaderASM, strlen(fenceShaderASM), 0, NULL, &pCode, &pErrors);
		if (pErrors)
			throw;

		g_device->CreateVertexShader(dwFenceDecl, (DWORD*)pCode->GetBufferPointer(), &hFenceShader, 0);
		pCode->Release();
	}

	auto Draw() -> void {
		if (!(RenderManager**)0x6c87b4) return;
		RenderManager* rm = *(RenderManager**)0x6c87b4;
		SwapArray<FenceEntityDSG*> fence_array = rm->_wrl->mStaticLoadLists.mpData[0].mFenceElems;

		UINT bufferSize = fence_array.mUseSize * sizeof(FenceEntityDSG);
		void* pGPUData;
		pFenceVB->Lock(0, bufferSize, (BYTE**)&pGPUData, D3DLOCK_DISCARD);
		memcpy(pGPUData, fence_array.mpData[0], bufferSize);
		pFenceVB->Unlock();

		float heights[4] = { 9.f, 3.f, 0.f, 0.f };
		g_device->SetVertexShaderConstant(4, &heights, 1);
		g_device->SetVertexShader(hFenceShader);
		g_device->SetStreamSource(0, pStaticTemplateVB, sizeof(Selector));
		g_device->SetStreamSource(1, pFenceVB, sizeof(FenceEntityDSG));
		g_device->SetIndices(pInstancingIB, 0);

		g_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, fence_array.mUseSize, 0, fence_array.mUseSize * 2);
	}
};

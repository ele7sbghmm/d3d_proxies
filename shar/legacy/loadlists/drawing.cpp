#include "pch.h"

#include <d3dx8.h>
#include "drawing.h"
#include "globals.h"
#include "types.h"

auto FenceCache::Init() {
	vertices.reserve(MAX_VERTICES);
	fenceLoadList.reserve(7);

	std::vector<unsigned short> trilist{};
	std::vector<unsigned short> linelist{};
	trilist.reserve(MAX_VERTICES * 6);
	linelist.reserve(MAX_VERTICES * 8);

	for (size_t i{}; i < MAX_VERTICES; i += 4) {
		trilist.insert(trilist.end(), {
			(unsigned short)(i + 0), (unsigned short)(i + 1),
			(unsigned short)(i + 2), (unsigned short)(i + 0),
			(unsigned short)(i + 2), (unsigned short)(i + 3),
			});
		linelist.insert(linelist.end(), {
			(unsigned short)(i + 0), (unsigned short)(i + 1),
			(unsigned short)(i + 1), (unsigned short)(i + 2),
			(unsigned short)(i + 2), (unsigned short)(i + 3),
			(unsigned short)(i + 3), (unsigned short)(i + 0),
			});
	}

	size_t verticesMaxSize = MAX_VERTICES * sizeof(DVertexNormalColor);
	size_t trilistSize = trilist.size() * sizeof(unsigned short);
	size_t linelistSize = linelist.size() * sizeof(unsigned short);

	Device->CreateVertexBuffer(verticesMaxSize,
							   D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
							   DVertexNormalColor::FVF,
							   D3DPOOL_DEFAULT,
							   &vertexBuffer);
	
	Device->CreateIndexBuffer(trilistSize,
							  D3DUSAGE_WRITEONLY,
							  D3DFMT_INDEX16,
							  D3DPOOL_DEFAULT,
							  &trilistIndexBuffer);

	Device->CreateIndexBuffer(linelistSize,
							  D3DUSAGE_WRITEONLY,
							  D3DFMT_INDEX16,
							  D3DPOOL_DEFAULT,
							  &linelistIndexBuffer);

	void* dummy1 = nullptr;
	void* dummy2 = nullptr;
	trilistIndexBuffer->Lock(0, trilistSize, (BYTE**)&dummy1, 0);
	linelistIndexBuffer->Lock(0, linelistSize, (BYTE**)&dummy2, 0);
	memcpy(dummy1, trilist.data(), trilistSize);
	memcpy(dummy2, linelist.data(), linelistSize);
	trilistIndexBuffer->Unlock();
	linelistIndexBuffer->Unlock();

	inited = true;
}
auto FenceCache::Fetch() {
	WorldRenderLayer* wrl = RenderManager::GetInstance()->pWorldRenderLayer();
	if (!wrl || !wrl->mpLoadLists.mUseSize) return;
	
	for (size_t i{}; i < wrl->mpLoadLists.mUseSize; ++i) {
		DynaLoadListDSG* ll = wrl->mpLoadLists.mpData[i];
		if (!ll->mFenceElems.mUseSize) continue;

		fenceLoadList.push_back(
			std::span(ll->mFenceElems.mpData, ll->mFenceElems.mUseSize)
		);
	}
}	
auto FenceCache::Clear() {
	vertices.clear();
	fenceLoadList.clear();
}
auto FenceCache::Populate() {
	if (!inited) Init();

	Fetch();
	if (fenceLoadList.empty()) return;

	DWORD RED = 0x40ff0000;

	rmt::Vector pos{};
	AvatarManager::GetInstance()->GetAvatarForPlayer(0)->GetPosition(&pos);
	auto top = pos.y + 3;
	auto bot = pos.y - 1;

	for (auto& span : fenceLoadList) {
		for (auto& fence : span) {
			if (!fence) continue;
			if (vertices.size() + 4 > MAX_VERTICES) break;

			auto s = fence->mStartPoint;
			auto e = fence->mEndPoint;
			auto n = fence->mNormal;
			vertices.insert(vertices.end(), {
				{{ s.x, top, s.z }, n, RED },
				{{ s.x, bot, s.z }, n, RED },
				{{ e.x, bot, e.z }, n, RED },
				{{ e.x, top, e.z }, n, RED }
			});
		}
	}

	UINT size = vertices.size() * sizeof(DVertexNormalColor);
	DVertexNormalColor* gpu = nullptr;
	HRESULT hr = vertexBuffer->Lock(0, size, (BYTE**)&gpu, D3DLOCK_DISCARD);
	if (SUCCEEDED(hr)) {
		memcpy(gpu, vertices.data(), size);
		vertexBuffer->Unlock();
	}
}
auto FenceCache::Draw() -> void {
	Populate();
	if (vertices.empty()) return;

	//Log::PrintRenderStates();

	IDirect3DBaseTexture8* originalTexture;
	DWORD originalSrc, originalDest, originalAlphaFunc, originalAlphaBlendEnable;
	Device->GetTexture(0, &originalTexture);
	Device->GetRenderState(D3DRS_SRCBLEND, &originalSrc);
	Device->GetRenderState(D3DRS_DESTBLEND, &originalDest);
	Device->GetRenderState(D3DRS_ALPHAFUNC, &originalAlphaFunc);
	Device->GetRenderState(D3DRS_ALPHABLENDENABLE, &originalAlphaBlendEnable);

	Device->SetTexture(0, NULL);
	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_LESSEQUAL);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);

	Device->SetVertexShader(DVertexNormalColor::FVF);
	Device->SetStreamSource(0, vertexBuffer, sizeof(DVertexNormalColor));
	Device->SetIndices(trilistIndexBuffer, 0);
	Device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
								 0, vertices.size(),
								 0, vertices.size() / 2);

	Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	Device->SetIndices(linelistIndexBuffer, 0);
	Device->DrawIndexedPrimitive(D3DPT_LINELIST,
								 0, vertices.size(),
								 0, vertices.size());

	Device->SetTexture(0, originalTexture);
	if (originalTexture) originalTexture->Release();
	Device->SetRenderState(D3DRS_SRCBLEND, originalSrc);
	Device->SetRenderState(D3DRS_DESTBLEND, originalDest);
	Device->SetRenderState(D3DRS_ALPHAFUNC, originalAlphaFunc);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, originalAlphaBlendEnable);

	Clear();
}

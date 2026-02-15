#include "pch.h"

#include "globals.h"
#include "render.h"
#include "types.h"

namespace GameCache {
	FenceCache fenceCache{};
}

auto SetFillmode(DWORD fill) -> DWORD {
	DWORD originalFill;
	Globals::GameDevice->GetRenderState(D3DRS_FILLMODE, &originalFill);
	Globals::GameDevice->SetRenderState(D3DRS_FILLMODE, fill);
	return originalFill;
}
auto SetFillmode_Solid() -> DWORD { return SetFillmode(D3DFILL_SOLID); }
auto SetFillmode_Wireframe() -> DWORD { return SetFillmode(D3DFILL_WIREFRAME); }
auto SetFillmode_Point() -> DWORD { return SetFillmode(D3DFILL_POINT); }

auto FenceCache::Init() -> void {
	vertices.reserve(MAX_FENCES);
	ptrs.reserve(MAX_FENCES);
	InitIndexBuffers();

	cais.resize(MAX_CAIS);
	for (auto& cai : cais)
		cai.Init();

	inited = true;

	return;
}
auto FenceCache::InitIndexBuffers() -> void {
	indicesTriList.reserve(MAX_FENCES * 6);
	indicesLineList.reserve(MAX_FENCES * 8);

	for (size_t i{}; i < MAX_FENCES; ++i) {
		unsigned short index = static_cast<unsigned short>(i * 4);

		indicesTriList.insert(indicesTriList.end(), {
			unsigned short(index + 0), unsigned short(index + 1),
			unsigned short(index + 2), unsigned short(index + 0),
			unsigned short(index + 2), unsigned short(index + 3)
			});
		indicesLineList.insert(indicesLineList.end(), {
			unsigned short(index + 0), unsigned short(index + 1),
			unsigned short(index + 1), unsigned short(index + 2),
			unsigned short(index + 2), unsigned short(index + 3),
			unsigned short(index + 3), unsigned short(index + 0)
			});
	}
}
auto FenceCache::Reset() -> void {
	for (auto& v : vertices) {
		v.position.y = 3.f;
		v.color = 0xffffffff;
	}
}
auto FenceCache::Update() -> void {
	DWORD GREEN = 0x4000ff00;
	DWORD RED = 0x40ff0000;
	DWORD WHITE = 0x40c0c0c0;

	if (vertices.empty()) return;

	for (auto cheat : { 0, 1, 2, 3, 15 }) {
		FenceCache::CaiStats cai = cais[cheat];
	//for (auto& cai : cais) {
		if (!cai.notUseless) continue;
		size_t processedCount = 0;

		for (auto& ptr : cai.thisFrameCacheInRange) {
			auto it = std::find(ptrs.begin(), ptrs.end(), ptr);

			if (it != ptrs.end()) {
				size_t fenceIndex = std::distance(ptrs.begin(), it);
				size_t vertexStart = fenceIndex * 4;

				float top = 2.5f;
				float bot = -2.5f;
				DWORD color = (processedCount < 8) ? RED : GREEN;

				DVertexNormalColor& v0 = vertices[vertexStart + 0];
				DVertexNormalColor& v1 = vertices[vertexStart + 1];
				DVertexNormalColor& v2 = vertices[vertexStart + 2];
				DVertexNormalColor& v3 = vertices[vertexStart + 3];
				v0.position.y = cai.position.y + top;
				v1.position.y = cai.position.y + bot;
				v2.position.y = cai.position.y + bot;
				v3.position.y = cai.position.y + top;
				//v0.position.y = (std::max)(v0.position.y, cai.position.y) + top;
				//v1.position.y = (std::min)(v1.position.y, cai.position.y) + bot;
				//v2.position.y = (std::min)(v2.position.y, cai.position.y) + bot;
				//v3.position.y = (std::max)(v3.position.y, cai.position.y) + top;
				v0.color = color;
				v1.color = color;
				v2.color = color;
				v3.color = color;

				processedCount++;
			}
		}
		for (auto& ptr : cai.thisFrameCache) {
			auto it = std::find(ptrs.begin(), ptrs.end(), ptr);
			if (it != ptrs.end()) {
				size_t fenceIndex = std::distance(ptrs.begin(), it);
				size_t vertexStart = fenceIndex * 4;

				float top = 2.5f;
				float bot = -2.5f;
				DWORD color = WHITE;

				DVertexNormalColor& v0 = vertices[vertexStart + 0];
				DVertexNormalColor& v1 = vertices[vertexStart + 1];
				DVertexNormalColor& v2 = vertices[vertexStart + 2];
				DVertexNormalColor& v3 = vertices[vertexStart + 3];
				v0.position.y = cai.position.y + top;
				v1.position.y = cai.position.y + bot;
				v2.position.y = cai.position.y + bot;
				v3.position.y = cai.position.y + top;
				//v0.position.y = (std::max)(v0.position.y, cai.position.y) + top;
				//v1.position.y = (std::min)(v1.position.y, cai.position.y) + bot;
				//v2.position.y = (std::min)(v2.position.y, cai.position.y) + bot;
				//v3.position.y = (std::max)(v3.position.y, cai.position.y) + top;
				v0.color = color;
				v1.color = color;
				v2.color = color;
				v3.color = color;
			}
		}
	}
}
auto FenceCache::Add(FenceEntityDSG* fence) -> void {
	if (!inited) Init();
	if (vertices.size() / 4 >= MAX_FENCES) return;
	//if (vertices.size() > 800) return;

	DWORD RED = 0x40ff0000;
	float t = 4.f;
	float b = 4.f;

	D3DXVECTOR3 s = fence->mStartPoint;
	D3DXVECTOR3 e = fence->mEndPoint;
	D3DXVECTOR3 n = fence->mNormal;
	DVertexNormalColor st = { { s.x, t, s.z }, n, RED };
	DVertexNormalColor sb = { { s.x, b, s.z }, n, RED };
	DVertexNormalColor eb = { { e.x, b, e.z }, n, RED };
	DVertexNormalColor et = { { e.x, t, e.z }, n, RED };

	vertices.insert(vertices.end(), { st, sb, eb, et });
	ptrs.push_back(fence);
}
auto FenceCache::Draw() -> void {
	if (!inited || vertices.empty() || indicesTriList.empty() || indicesLineList.empty()) return;
	Update();
	for (auto& cai : cais) cai.Clear();

	UINT numVertices = vertices.size();
	UINT numFences = numVertices / 4;
	UINT stride = sizeof(DVertexNormalColor);
	DWORD fvf = DVertexNormalColor::FVF;

	Globals::GameDevice->SetTexture(0, NULL);
	Globals::GameDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	Globals::GameDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	Globals::GameDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	Globals::GameDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		
	Globals::GameDevice->SetVertexShader(fvf);
	Globals::GameDevice->DrawIndexedPrimitiveUP(
		D3DPT_TRIANGLELIST, 0, numVertices, numFences * 2, indicesTriList.data(),
		D3DFMT_INDEX16, vertices.data(), stride);

	Globals::GameDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	Globals::GameDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	//Globals::GameDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	Globals::GameDevice->DrawIndexedPrimitiveUP(
		D3DPT_LINELIST, 0, numVertices, numFences * 4, indicesLineList.data(),
		D3DFMT_INDEX16, vertices.data(), stride);

	Globals::GameDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	//Globals::GameDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
	Reset();
}
auto FenceCache::CaiStats::Init() -> void {
	notUseless = false;

	thisFrameCacheInRange.reserve(1024);
	thisFrameCache.reserve(1024);
}
auto FenceCache::CaiStats::Clear() -> void {
	notUseless = false;

	thisFrameCacheInRange.clear();
	thisFrameCache.clear();
}
auto FenceCache::CaiStats::Update(D3DXVECTOR3* p, float r, FenceEntityDSG* f, bool inRange) -> void {
	notUseless = true;

	position = *p;
	radius = r;

	if (inRange) thisFrameCacheInRange.push_back(f);
	else thisFrameCache.push_back(f);
}

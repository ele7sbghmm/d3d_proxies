#pragma once

#include <d3dx8.h>
#include "shar.h"
#include "vertex.h"

class Buffer2d {
public:
	std::vector<float, AlignedAllocator<float>> m_x;
	std::vector<float, AlignedAllocator<float>> m_y;

	Buffer2d(size_t count) {
		m_x.reserve(count);
		m_y.reserve(count);
	}
	auto Clear() -> void {
		m_x.clear();
		m_y.clear();
	}
};

class FenceRenderer2d {
public:
	Buffer2d m_inactive{ 9000 };
	Buffer2d m_active{ 1000 };

	auto Clear() -> void;
	auto BakeArray(const Shar::SwapArray<Shar::FenceEntityDSG*>& array, const D3DXVECTOR3& position, float radius) -> void;
	auto BakeArray(const Shar::SwapArray<Shar::FenceEntityDSG*>& array) -> void;

};

struct FenceStats {
	D3DXVECTOR3 center, scratch;
	float rangeSq, distSq;
	bool inRange;
	FenceStats(const Shar::FenceEntityDSG* entity, const D3DXVECTOR3& position, float radius) {
		center = *(D3DXVECTOR3*)&entity->mStartPoint;
		scratch = *(D3DXVECTOR3*)&entity->mEndPoint;
		center.y = 0.f;
		scratch.y = 0.f;

		center += scratch;
		center *= .5f;

		scratch -= center;

		rangeSq = D3DXVec3LengthSq(&scratch) + (radius * radius);

		scratch = position;
		scratch.y = 0.f;
		scratch -= center;
		distSq = D3DXVec3LengthSq(&scratch);

		inRange = distSq < rangeSq;
	}
};
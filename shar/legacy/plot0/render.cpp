#include "pch.h"

#include "render.h"

auto FenceRenderer2d::BakeArray(const Shar::SwapArray<Shar::FenceEntityDSG*>& array, const D3DXVECTOR3& position, float radius) -> void {
	for (size_t i = 0; i < array.mUseSize; ++i) {
		size_t reverse = array.mUseSize - 1 - i;
		Shar::FenceEntityDSG* entity = array.mpData[reverse];
		if (!entity) continue;

		FenceStats stats = FenceStats(entity, position, radius);
		if (stats.inRange) {
			m_active.m_x.push_back(entity->mStartPoint.x);
			m_active.m_y.push_back(entity->mStartPoint.z);
			m_active.m_x.push_back(entity->mEndPoint.x);
			m_active.m_y.push_back(entity->mEndPoint.z);
		}
		m_inactive.m_x.push_back(entity->mStartPoint.x);
		m_inactive.m_y.push_back(entity->mStartPoint.z);
		m_inactive.m_x.push_back(entity->mEndPoint.x);
		m_inactive.m_y.push_back(entity->mEndPoint.z);
	}
}
auto FenceRenderer2d::Clear() -> void {
	m_inactive.Clear();
	m_active.Clear();
}
auto FenceRenderer2d::BakeArray(const Shar::SwapArray<Shar::FenceEntityDSG*>& array) -> void {
	for (size_t i = 0; i < array.mUseSize; ++i) {
		size_t reverse = array.mUseSize - 1 - i;
		Shar::FenceEntityDSG* entity = array.mpData[reverse];
		if (!entity) continue;

		m_inactive.m_x.push_back(entity->mStartPoint.x);
		m_inactive.m_y.push_back(entity->mStartPoint.z);
		m_inactive.m_x.push_back(entity->mEndPoint.x);
		m_inactive.m_y.push_back(entity->mEndPoint.z);
	}
}

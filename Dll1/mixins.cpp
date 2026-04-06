#include "pch.h"

#include "mixins.h"
#include "globals.h"

//auto mod::FenceArray::AddVertices(XYZNormalColor::ArrayType& vertices) -> void {
//	const size_t vertex_count = this->mUseSize * 6;
//	const size_t old_size = vertices.size();
//	vertices.resize(old_size + vertex_count);
//	XYZNormalColor* write_ptr = vertices.data() + old_size;
//
//	const D3DCOLOR active_color = (D3DCOLOR)config::color_FenceOpaqueActive;
//	//const D3DCOLOR active_color = static_cast<D3DCOLOR>(g_Config::color_FenceOpaqueActive);
//
//	const float top{ 10.f }, bot{ 0.f };
//
//	for (size_t i = 0; i < this->mUseSize; ++i) {
//		Shar::FenceEntityDSG& fence = *this->mpData[i];
//
//		const auto& s = fence.mStartPoint;
//		const auto& e = fence.mEndPoint;
//		const auto& n = fence.mNormal;
//
//		write_ptr[0] = { s.x, top, s.z, n.x, n.y, n.z, active_color };
//		write_ptr[1] = { s.x, bot, s.z, n.x, n.y, n.z, active_color };
//		write_ptr[2] = { e.x, bot, e.z, n.x, n.y, n.z, active_color };
//		write_ptr[3] = { e.x, bot, e.z, n.x, n.y, n.z, active_color };
//		write_ptr[4] = { e.x, top, e.z, n.x, n.y, n.z, active_color };
//		write_ptr[5] = { s.x, top, s.z, n.x, n.y, n.z, active_color };
//
//		write_ptr += 6;
//	}
//}

auto mod::WorldRenderLayer::GetWorldScene() -> mod::WorldScene& {
	return *static_cast<mod::WorldScene*>(mpWorldScene);
}

auto mod::RenderManager::Get() -> mod::RenderManager* {
	return *::singletons::rendermanager;
}
auto mod::RenderManager::GetWorldRenderLayer() -> mod::WorldRenderLayer* {
	auto level_slot = Shar::RenderEnums::LevelSlot;
	return static_cast<mod::WorldRenderLayer*>(mpRenderLayers[level_slot]);
}

auto mod::Character::GetInstance() -> mod::Character* {
	return *::singletons::character;
}
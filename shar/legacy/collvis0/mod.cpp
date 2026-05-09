#include "pch.h"

#include "mod.h"

auto get_spatialtreeiter() -> shar::SpatialTreeIter* {
	shar::RenderManager* rm = shar::RenderManager::GetInstance();
	if (!rm) return nullptr;
	
	return &rm->_wrl->mpWorldScene->mStaticTreeWalker;
}
auto get_avatar_position() -> D3DXVECTOR3 {
	D3DXVECTOR3 position{};

	shar::AvatarManager* avm = shar::AvatarManager::GetInstance();
	if (!avm) return position;

	shar::Avatar* av = avm->GetAvatarForPlayer(0);
	av->GetPosition(&position);
	return position;
}
auto get_collisionareaindex_radius() -> float {
	shar::AvatarManager* avm = shar::AvatarManager::GetInstance();
	if (!avm || !avm->GetAvatarForPlayer(0)) return 0.f;
	shar::Avatar* av = avm->GetAvatarForPlayer(0);
	if (!av->mpVehicle)
		return 5.f;
	return av->mpVehicle->GetWheelBase() * 2.f;
}

auto Fence::bake_passive(Buffer& buffer, shar::SwapArray<shar::FenceEntityDSG*> fence_array, float y, bool draw_nonpassive) {
	CustomVertex* v = buffer.m_gpu_data;
	std::size_t c = buffer.m_vertex_count;

	std::uint32_t color = draw_nonpassive ? m_color_passive : m_color_collidable;
	for (std::size_t i = 0; i < fence_array.mUseSize; ++i) {
		auto* fence = fence_array.mpData[i];

		auto& s = fence->mStartPoint;
		auto& e = fence->mEndPoint;
		auto& n = fence->mNormal;

		float top = y + m_height + m_y_offset;
		float bottom = y - m_height + m_y_offset;

		v[c++] = { s.x, top, s.z, n.x, n.y, n.z, color, NULL };
		v[c++] = { s.x, bottom, s.z, n.x, n.y, n.z, color, NULL };
		v[c++] = { e.x, bottom, e.z, n.x, n.y, n.z, color, NULL };
		v[c++] = { e.x, bottom, e.z, n.x, n.y, n.z, color, NULL };
		v[c++] = { e.x, top, e.z, n.x, n.y, n.z, color, NULL };
		v[c++] = { s.x, top, s.z, n.x, n.y, n.z, color, NULL };
	}

	buffer.m_vertex_count = c;
}

auto Fence::bake_active(Buffer& buffer, shar::SwapArray<shar::FenceEntityDSG*> fence_array, D3DXVECTOR3 position, float radiussq) {
	std::uint32_t pad = 0;
	D3DXCOLOR color = m_color_active;
	D3DXCOLOR line_color;

	D3DXVECTOR3 scratch, center;
	float top, bottom, angle_step, x, z, rad, line_alpha;

	CustomVertex* v_tri = buffer.m_gpu_data;
	//CustomVertex* v_line = buffer.m_gpu_data + Buffer::MAX_TRIANGLE_VERTICES;
	std::size_t c_tri = buffer.m_vertex_count;
	//std::size_t c_line = buffer.m_vertex_count;

	for (std::size_t i = 0; i < fence_array.mUseSize; ++i) {
		std::size_t reverse = fence_array.mUseSize - 1 - i;
		auto* fence = fence_array.mpData[reverse];
		auto& s = fence->mStartPoint;
		auto& e = fence->mEndPoint;
		auto& n = fence->mNormal;

		scratch = s;
		scratch.y = 0.f;

		center = e;
		center.y = 0.f;

		center += scratch;
		center *= .5f;

		scratch -= center;
		float half_length_sq = D3DXVec3LengthSq(&scratch);
		float activated_radius_sq = half_length_sq + radiussq;
		float activated_radius = sqrtf(activated_radius_sq);

		scratch = center - position;
		scratch.y = 0.f;
		float distance_to_center_sq = D3DXVec3LengthSq(&scratch);

		line_alpha = 1.f - (min(100.f, distance_to_center_sq) / 100);

		if (distance_to_center_sq < activated_radius_sq) {
			m_active_count++;

			color = (m_active_count > MAX_COLLIDABLE_COUNT) ? m_color_overflow : m_color_collidable;
		} else
			color = m_color_active;

		line_color = { color.r, color.g, color.b, line_alpha };

		top = position.y + m_height + m_y_offset;
		bottom = position.y - m_height + m_y_offset;

		v_tri[c_tri++] = { s.x, top, s.z, n.x, n.y, n.z, color, pad };
		v_tri[c_tri++] = { s.x, bottom, s.z, n.x, n.y, n.z, color, pad };
		v_tri[c_tri++] = { e.x, bottom, e.z, n.x, n.y, n.z, color, pad };
		v_tri[c_tri++] = { e.x, bottom, e.z, n.x, n.y, n.z, color, pad };
		v_tri[c_tri++] = { e.x, top, e.z, n.x, n.y, n.z, color, pad };
		v_tri[c_tri++] = { s.x, top, s.z, n.x, n.y, n.z, color, pad };

		//angle_step = D3DX_PI * 2.0f / SEGMENTS_PER_PERIMETER;
		//for (std::size_t j = 0; j <= SEGMENTS_PER_PERIMETER; ++j) {
		//	rad = j * angle_step;
		//	x = sinf(rad) * activated_radius;
		//	z = cosf(rad) * activated_radius;

		//	v_line[c_line++] = { center.x + x, position.y, center.z + z, n.x, n.y, n.z, line_color, pad };
		//	if (j > 0 && j < SEGMENTS_PER_PERIMETER)
		//		v_line[c_line++] = { center.x + x, position.y, center.z + z, n.x, n.y, n.z, line_color, pad };
		//}
	}

	buffer.m_vertex_count = c_tri;
}

CollVisContext::CollVisContext(IDirect3DDevice8* device) : m_device(device) {
	memset((void*)&m_material, NULL, sizeof(D3DMATERIAL8));
	
	m_material.Ambient = { .7f, .7f, .7f, .7f };
	m_material.Emissive = { 0.f, 0.f, 0.f, 0.f };
}

auto CollVisContext::batch_active(shar::SpatialNode& node, D3DXVECTOR3 position, float radiussq) {
	if (m_fence.m_draw && node.mFenceElems.mUseSize)
		m_fence.bake_active(m_buffer, node.mFenceElems, position, radiussq);
}

auto CollVisContext::batch_passive(shar::SpatialNode& node, float y) {
	if (m_fence.m_draw && node.mFenceElems.mUseSize)
		m_fence.bake_passive(m_buffer, node.mFenceElems, y, m_draw_active);
}

auto CollVisContext::iter() {
	shar::SpatialTreeIter& sti = *get_spatialtreeiter();

	m_inited = true;

	D3DXVECTOR3 center = m_volume.mCenter;

	if (!m_freeze_position) {
		center = get_avatar_position();

		if (m_freeze_y)
			center.y = m_volume.mCenter.y;
	}

	float radius = m_freeze_radius ? m_volume.mRadius : get_collisionareaindex_radius();
	float radiussq = radius * radius;

	m_volume.SetTo(center, radius);

	if (false) {
		sti.MarkAll(m_volume, shar::WorldScene::msStaticPhys);
		for (sti.MoveToFirst(); sti.NotDone(); sti.MoveToNext()) {
			batch_active(sti.rCurrent(), m_volume.mCenter, radiussq);
		}
	}

	for (std::int32_t i = 0; i < 1; ++i) {
		shar::SpatialNode& node = sti.rIthNode(i);

		if (m_draw_passive)
			batch_passive(node, m_volume.mCenter.y);

	}

	m_fence.m_active_count = 0;
}

auto CollVisContext::commit() -> void {
	m_buffer.lock(m_device, 100000);
	//m_buffer_lines.lock(m_device, 100000);

	iter();

	m_buffer.m_primitive_count = m_buffer.m_vertex_count / 3;
	//m_buffer_lines.m_primitive_count = m_buffer_lines.m_vertex_count / 2;
	m_buffer.unlock();
	//m_buffer_lines.unlock();
}

auto CollVisContext::draw() -> void {
	if (m_buffer.m_primitive_count == 0) return;

	DWORD state_block;
	if (FAILED(m_device->CreateStateBlock(D3DSBT_ALL, &state_block)))
		return;

	m_device->SetVertexShader(CustomVertex::FVF);
	m_state.SetStates(m_device, m_material);

	m_device->SetStreamSource(0, m_buffer.m_vertex_buffer, sizeof(CustomVertex));
	m_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_buffer.m_primitive_count);

	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_device->SetRenderState(D3DRS_LIGHTING, FALSE);

	//m_device->SetStreamSource(0, m_buffer_lines.m_vertex_buffer, sizeof(CustomVertex));
	//m_device->DrawPrimitive(D3DPT_LINELIST, 0, m_buffer_lines.m_primitive_count);

	m_device->ApplyStateBlock(state_block);
	m_device->DeleteStateBlock(state_block);
}

auto CollVisContext::end_scene() -> void {
	if (m_inited)
		m_gui.new_frame(*this);
}

auto Buffer::lock(IDirect3DDevice8* device, std::size_t max_vertices) -> HRESULT {
	if (!m_vertex_buffer)
		device->CreateVertexBuffer(max_vertices * sizeof(CustomVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, CustomVertex::FVF, D3DPOOL_DEFAULT, &m_vertex_buffer);

	m_gpu_data = nullptr;
	m_vertex_count = 0;

	return m_vertex_buffer->Lock(0, 0, (BYTE**)&m_gpu_data, D3DLOCK_DISCARD);
}

#pragma once

#include <wrl/client.h>
#include <MinHook.h>

#include "shar.h"


using Microsoft::WRL::ComPtr;

struct CustomVertex {
	float x, y, z, nx, ny, nz;
	D3DCOLOR color;
	std::uint32_t pad;
	static constexpr DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;
};

class Renderer {
public:
	Renderer() = default;

	void CommitDraw();
	void Iter(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx);
	void BakeFences(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::SpatialNode& node, Shar::SphereSP& sphere, bool node_is_active);
	void BakeIntersects(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::SpatialNode& node, Shar::SphereSP& sphere, bool node_is_active);
	void BakeStatics(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::SpatialNode& node, Shar::SphereSP& sphere, bool node_is_active);
	void BakeDynamics(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::SpatialNode& node, Shar::SphereSP& sphere, bool node_is_active);
	void BakeCollisionVolume(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::sim::CollisionVolume* vol, D3DCOLOR color);
	void BakeOBboxVolume(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::sim::OBBoxVolume* obbox, D3DCOLOR color);
	void BakeCylinderVolume(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::sim::CylinderVolume* obbox, D3DCOLOR color);
	void BakeSphereVolume(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::sim::SphereVolume* obbox, D3DCOLOR color);

private:
	ComPtr<IDirect3DDevice8> m_device = nullptr;
	ComPtr<IDirect3DVertexBuffer8> m_vb = nullptr;
	ComPtr<IDirect3DIndexBuffer8> m_ib = nullptr;
	std::size_t m_vtx_count = 0;
	std::size_t m_idx_count = 0;
	static constexpr std::size_t MAX_VTX_COUNT = 1000000;
	static constexpr std::size_t MAX_IDX_COUNT = 1000000;

public:
	bool m_inited = false;

	static constexpr std::size_t MAX_COLLIDABLE_STATICS = 30;
	static constexpr std::size_t MAX_COLLIDABLE_DYNAMICS = 20;
	static constexpr std::size_t MAX_COLLIDABLE_ANIMCOLLISIONS = 20;
	static constexpr std::size_t MAX_COLLIDABLE_FENCES = 8;

	D3DXVECTOR3 m_player_position{};
	float m_player_radius{};

	std::size_t m_collidable_fence_count = 0;
	std::size_t m_collidable_statics_count = 0;

	static constexpr D3DCOLOR WHITE = 0xffffffff;
	static constexpr D3DCOLOR RED = 0xffff0000;
	static constexpr D3DCOLOR GREEN = 0xff00ff00;
	static constexpr D3DCOLOR TEAL = 0xff00ffff;

	struct Config {
		struct TerrainPaletteItem { D3DXCOLOR color; char name[32]; };
		TerrainPaletteItem color_terrain_palette[8] = {
			{ 0xff202020, "road" },
			{ 0xff00ff00, "grass" },
			{ 0xffff8000, "sand" },
			{ 0xff404040, "gravel" },
			{ 0xff0000ff, "water" },
			{ 0xffc04040, "wood" },
			{ 0xff808080, "metal" },
			{ 0xffc04040, "dirt" },
		};

		D3DXCOLOR color_passive = WHITE;
		D3DXCOLOR color_active = TEAL;
		D3DXCOLOR color_collidable = RED;
		D3DXCOLOR color_overflow = GREEN;
		D3DXCOLOR color_statics_passive = 0xff808080;
		D3DXCOLOR color_statics_active = 0xffffffff;
		D3DXCOLOR color_statics_collidable = 0xff00ffff;

		bool draw_world_sphere = false;
		bool draw_opaque = false;
		bool draw_simple_shadows = false;
		bool draw_translucent = true;

		bool draw_passive_nodes = true;
		bool draw_active_nodes = true;

		bool draw_fences = true;
		bool draw_statics = true;
		bool draw_dynamics = true;
		bool draw_intersects = true;

		bool update_position = true;
		bool update_y = true;
		bool update_radius = true;
		bool use_custom_radius = false;
		float custom_radius = 5.f;

		float fence_height = 2.f;
		float fence_y_offset = 1.f;
	} m_config;
};

inline void Renderer::CommitDraw() {
	if (!m_device) {
		Shar::d3dDisplay* display = Shar::get_display();
		if (!display || !display->d3dDevice) return;
		m_device = display->d3dDevice;
	}

	if (!m_vb && FAILED(m_device->CreateVertexBuffer(MAX_VTX_COUNT * sizeof(CustomVertex), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, CustomVertex::FVF, D3DPOOL_DEFAULT, m_vb.GetAddressOf())))
		return;
	if (!m_ib && FAILED(m_device->CreateIndexBuffer(MAX_IDX_COUNT * sizeof(std::uint32_t), D3DUSAGE_WRITEONLY, D3DFMT_INDEX32, D3DPOOL_DEFAULT, m_ib.GetAddressOf())))
		return;

	if (!m_inited)
		m_inited = true;

	CustomVertex* gpu_vtx;
	std::uint32_t* gpu_idx;

	if (FAILED(m_vb->Lock(0, 0, (BYTE**)&gpu_vtx, D3DLOCK_DISCARD)))
		return;
	if (FAILED(m_ib->Lock(0, 0, (BYTE**)&gpu_idx, D3DLOCK_DISCARD))) {
		m_vb->Unlock();
		return;
	}

	Iter(gpu_vtx, gpu_idx);

	m_vb->Unlock();
	m_ib->Unlock();

	DWORD state_block_token = NULL;
	m_device->CreateStateBlock(D3DSBT_ALL, &state_block_token);

	m_device->SetVertexShader(CustomVertex::FVF);
	m_device->SetStreamSource(0, m_vb.Get(), sizeof(CustomVertex));
	m_device->SetIndices(m_ib.Get(), 0);

	//m_device->SetMaterial(&mat);
	m_device->SetTexture(0, NULL);

	m_device->SetRenderState(D3DRS_LIGHTING, TRUE);
	m_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	m_device->SetRenderState(D3DRS_ZBIAS, 1);
	m_device->SetRenderState(D3DRS_COLORVERTEX, TRUE);
	m_device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_COLOR1);
	//m_device->SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR1);
	//m_device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_COLOR1);
	//m_device->SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_COLOR1);
	//m_device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	//m_device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	//m_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	m_device->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, m_vtx_count, 0, m_idx_count / 3);

	m_device->ApplyStateBlock(state_block_token);
	m_device->DeleteStateBlock(state_block_token);

	m_vtx_count = 0;
	m_idx_count = 0;
}

inline void Renderer::Iter(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx) {
	Shar::SpatialTreeIter& sti = Shar::RenderManager::GetInstance()->_wrl->mpWorldScene->mStaticTreeWalker;
	Shar::Avatar* avatar = Shar::AvatarManager::GetInstance()->GetAvatarForPlayer(0);

	D3DXVECTOR3 new_position = m_player_position;
	float new_radius = m_player_radius;

	if (m_config.update_position) {
		avatar->GetPosition(&new_position);
		if (!m_config.update_y)
			new_position.y = m_player_position.y;
	}

	if (m_config.use_custom_radius)
		new_radius = m_config.custom_radius;
	else if (m_config.update_radius)
		if (!avatar->mpVehicle)
			new_radius = 5.f;
		else
			new_radius = avatar->mpVehicle->GetWheelBase() * 2.;

	Shar::SphereSP sphere;
	sphere._constructor();
	sphere.SetTo(new_position, new_radius);
	m_player_position = new_position;
	m_player_radius = new_radius;

	m_collidable_fence_count = 0;
	m_collidable_statics_count = 0;

	sti.MarkAll(sphere, Shar::WorldScene::msStaticPhys);
	for (std::size_t i = 0; i < sti.NumNodes(); ++i) {
		Shar::SpatialNode& node = sti.rIthNode(i);;
		Shar::tMark mark = sti.mNodeMarks.mpData[i];

		bool is_active_node = m_config.draw_active_nodes
			? (mark & 0b1111) == Shar::WorldScene::msStaticPhys
			: false;

		if (!is_active_node && !m_config.draw_passive_nodes)
			continue;

		BakeFences(gpu_vtx, gpu_idx, node, sphere, is_active_node);
		BakeStatics(gpu_vtx, gpu_idx, node, sphere, is_active_node);
		BakeDynamics(gpu_vtx, gpu_idx, node, sphere, is_active_node);
		BakeIntersects(gpu_vtx, gpu_idx, node, sphere, is_active_node);
	}
}

inline void Renderer::BakeFences(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::SpatialNode& node, Shar::SphereSP& sphere, bool node_is_active) {
	if (!m_config.draw_fences || !node.mFenceElems.mUseSize) return;

	float fence_center_y = m_player_position.y + m_config.fence_y_offset;
	float top = fence_center_y + m_config.fence_height;
	float bot = fence_center_y - m_config.fence_height;
	float radiussq = sphere.mRadius * sphere.mRadius;

	std::size_t n_vtx = m_vtx_count;
	std::size_t n_idx = m_idx_count;

	for (int i = node.mFenceElems.mUseSize - 1; i >= 0; --i) {
		Shar::FenceEntityDSG* fence = node.mFenceElems.mpData[i];
		D3DVECTOR& start = fence->mStartPoint;
		D3DVECTOR& end = fence->mEndPoint;
		D3DVECTOR& normal = fence->mNormal;

		D3DCOLOR color = m_config.draw_active_nodes
			? m_config.color_passive
			: m_config.color_collidable;
		if (m_config.draw_active_nodes && node_is_active) {
			color = m_config.color_active;

			D3DXVECTOR3 center = start;
			center += end;
			center *= .5f;
			center.y = 0.f;

			D3DXVECTOR3 scratch = start;
			scratch -= center;
			scratch.y = 0.f;

			float half_length_sq = D3DXVec3LengthSq(&scratch);
			float is_collidable_distance_sq = half_length_sq + radiussq;

			scratch = sphere.mCenter;
			scratch -= center;
			scratch.y = 0.;
			float player_distance_sq = D3DXVec3LengthSq(&scratch);
			if (player_distance_sq < is_collidable_distance_sq) {
				m_collidable_fence_count++;

				color = m_collidable_fence_count > MAX_COLLIDABLE_FENCES
					? m_config.color_overflow
					: m_config.color_collidable;
			}
		}

		gpu_idx[n_idx++] = n_vtx + 0;
		gpu_idx[n_idx++] = n_vtx + 1;
		gpu_idx[n_idx++] = n_vtx + 2;
		gpu_idx[n_idx++] = n_vtx + 2;
		gpu_idx[n_idx++] = n_vtx + 3;
		gpu_idx[n_idx++] = n_vtx + 0;

		gpu_vtx[n_vtx++] = { start.x, top, start.z, normal.x, normal.y, normal.z, color, NULL };
		gpu_vtx[n_vtx++] = { start.x, bot, start.z, normal.x, normal.y, normal.z, color, NULL };
		gpu_vtx[n_vtx++] = { end.x, bot, end.z, normal.x, normal.y, normal.z, color, NULL };
		gpu_vtx[n_vtx++] = { end.x, top, end.z, normal.x, normal.y, normal.z, color, NULL };
	}
	m_vtx_count = n_vtx;
	m_idx_count = n_idx;
}

inline void Renderer::BakeIntersects(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::SpatialNode& node, Shar::SphereSP& sphere, bool node_is_active) {
	if (!m_config.draw_intersects || !node.mIntersectElems.mUseSize) return;

	bool use_terrain_color = !m_config.draw_active_nodes || node_is_active;
	D3DCOLOR color;

	std::size_t n_vtx = m_vtx_count;
	std::size_t n_idx = m_idx_count;

	for (int i = (int)node.mIntersectElems.mUseSize - 1; i >= 0; --i) {
		auto* intersect = node.mIntersectElems.mpData[i];
		if (!intersect) return;

		if (m_vtx_count + intersect->mTriIndices.mUseSize > MAX_VTX_COUNT) return;

		for (std::size_t j = 0; j < intersect->mTriIndices.mUseSize; ++j) {
			gpu_idx[n_idx++] = n_vtx;

			std::size_t triangle_index = j / 3;
			std::size_t point_index = intersect->mTriIndices.mpData[j];

			D3DVECTOR& point = intersect->mTriPts.mpData[point_index];
			D3DVECTOR& normal = intersect->mTriNorms.mpData[triangle_index];

			std::size_t terrain_index = intersect->mTerrainType.mUseSize
				? intersect->mTerrainType.mpData[triangle_index]
				: 0;
			if (use_terrain_color)
				color = m_config.color_terrain_palette[terrain_index].color;
			else
				color = m_config.color_passive;

			gpu_vtx[n_vtx++] = {
				point.x,  point.y,  point.z,
				normal.x, normal.y, normal.z,
				color,
				0
			};
		}
	}

	m_vtx_count = n_vtx;
	m_idx_count = n_idx;
}
inline void Renderer::BakeStatics(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::SpatialNode& node, Shar::SphereSP& sphere, bool node_is_active) {
	if (!m_config.draw_statics || !node.mSPhysElems.mUseSize) return;

	D3DCOLOR node_color = node_is_active
		? m_config.color_statics_active
		: m_config.color_statics_passive;

	for (int i = node.mSPhysElems.mUseSize - 1; i >= 0; --i) {
		D3DCOLOR color = node_color;

		Shar::StaticPhysDSG* sphys = node.mSPhysElems.mpData[i];
		if (!sphys || !sphys->mpSimStateObj) continue;

		Shar::sim::SimState* ss = sphys->mpSimStateObj;
		if (ss->mScale != 1.f) throw;

		Shar::sim::CollisionObject* co = ss->mCollisionObject;
		if (!co) continue;

		if (node_is_active) {
			D3DXVECTOR3 scratch = sphere.mCenter;
			scratch -= sphys->mSphere.center;

			float dist_to_elem_sq = D3DXVec3LengthSq(&scratch);
			float max_dist_sq = (sphere.mRadius + sphys->mSphere.radius) * (sphere.mRadius + sphys->mSphere.radius);
			if (dist_to_elem_sq < max_dist_sq) {
				m_collidable_statics_count++;

				color = (m_collidable_statics_count > MAX_COLLIDABLE_STATICS)
					? m_config.color_overflow
					: m_config.color_statics_collidable;
			}
		}

		BakeCollisionVolume(gpu_vtx, gpu_idx, co->mCollisionVolume, color);
	}
}
inline void Renderer::BakeDynamics(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::SpatialNode& node, Shar::SphereSP& sphere, bool node_is_active) {
	if (!m_config.draw_dynamics || !node.mDPhysElems.mUseSize) return;

	D3DCOLOR node_color = node_is_active
		? m_config.color_statics_active
		: m_config.color_statics_passive;

	for (int i = node.mDPhysElems.mUseSize - 1; i >= 0; --i) {
		D3DCOLOR color = node_color;

		Shar::StaticPhysDSG* dphys = node.mDPhysElems.mpData[i];
		if (!dphys || !dphys->mpSimStateObj) continue;
		D3DXVECTOR3 position = dphys->mPosn;

		Shar::sim::SimState* ss = dphys->mpSimStateObj;
		if (ss->mScale != 1.f) throw;
		D3DXMATRIX transform = ss->mTransform;

		Shar::sim::CollisionObject* co = ss->mCollisionObject;
		if (!co || !co->mCollisionEnabled) continue;

		if (node_is_active) {
			D3DXVECTOR3 scratch = sphere.mCenter;
			scratch -= dphys->mSphere.center;

			float dist_to_elem_sq = D3DXVec3LengthSq(&scratch);
			float max_dist_sq = (sphere.mRadius + dphys->mSphere.radius) * (sphere.mRadius + dphys->mSphere.radius);
			if (dist_to_elem_sq < max_dist_sq) {
				m_collidable_statics_count++;

				color = (m_collidable_statics_count > MAX_COLLIDABLE_STATICS)
					? m_config.color_overflow
					: m_config.color_statics_collidable;	
			}
		}

		BakeCollisionVolume(gpu_vtx, gpu_idx, co->mCollisionVolume, color);
	}
}

inline void Renderer::BakeCollisionVolume(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::sim::CollisionVolume* vol, D3DCOLOR color) {
	if (!vol) return;

	if (vol->mType == Shar::sim::OBBoxVolumeType) {
		auto* cast = static_cast<Shar::sim::OBBoxVolume*>(vol);
		BakeOBboxVolume(gpu_vtx, gpu_idx, cast, color);
	}
	if (vol->mType == Shar::sim::CylinderVolumeType) {
		auto* cast = static_cast<Shar::sim::CylinderVolume*>(vol);
		BakeCylinderVolume(gpu_vtx, gpu_idx, cast, color);
	}
	if (vol->mType == Shar::sim::SphereVolumeType) {
		auto* cast = static_cast<Shar::sim::SphereVolume*>(vol);
		BakeSphereVolume(gpu_vtx, gpu_idx, cast, color);
	}

	if (vol->mSubVolumeList && vol->mSubVolumeList->mArray)
		for (std::size_t i = 0; i < vol->mSubVolumeList->mSize; ++i) {
			auto* child = static_cast<Shar::sim::CollisionVolume*>(vol->mSubVolumeList->mArray[i]);
			BakeCollisionVolume(gpu_vtx, gpu_idx, child, color);
		}
}

inline void Renderer::BakeOBboxVolume(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::sim::OBBoxVolume* vol, D3DCOLOR color) {
	if (!vol) return;

	float* l = vol->mLength;
	D3DXVECTOR3* axis = reinterpret_cast<D3DXVECTOR3*>(vol->mAxis);
	D3DXVECTOR3 pos = static_cast<D3DXVECTOR3>(vol->mPosition);

	std::size_t vtx_n = m_vtx_count;
	std::size_t idx_n = m_idx_count;

	for (std::size_t f = 0; f < 6; ++f) {
		std::size_t base = vtx_n;

		int axis_idx = f / 2;
		float side = (f % 2) ? 1. : -1.f;
		D3DXVECTOR3 n = axis[axis_idx] * side;

		D3DXVECTOR3 a1 = axis[(axis_idx + 1) % 3];
		D3DXVECTOR3 a2 = axis[(axis_idx + 2) % 3];
		float signs[4][2] = { { -1.f, -1.f }, { 1.f, -1.f }, { 1.f, 1.f }, { -1.f, 1.f } };
		for (int i = 0; i < 4; ++i) {
			D3DXVECTOR3 v = pos
				+ (n * l[axis_idx])
				+ (a1 * (signs[i][0] * l[(axis_idx + 1) % 3]))
				+ (a2 * (signs[i][1] * l[(axis_idx + 2) % 3]));
			gpu_vtx[vtx_n++] = { v.x, v.y, v.z, n.x, n.y, n.z, color, NULL };
		}

		if (side > 0) {
			gpu_idx[idx_n++] = base + 0; gpu_idx[idx_n++] = base + 1;
			gpu_idx[idx_n++] = base + 2; gpu_idx[idx_n++] = base + 0;
			gpu_idx[idx_n++] = base + 2; gpu_idx[idx_n++] = base + 3;
		}
		else {
			gpu_idx[idx_n++] = base + 0; gpu_idx[idx_n++] = base + 2;
			gpu_idx[idx_n++] = base + 1; gpu_idx[idx_n++] = base + 0;
			gpu_idx[idx_n++] = base + 3; gpu_idx[idx_n++] = base + 2;
		}
	}

	m_vtx_count = vtx_n;
	m_idx_count = idx_n;
}

inline void Renderer::BakeCylinderVolume(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::sim::CylinderVolume* vol, D3DCOLOR color) {
	if (!vol) return;

}

inline void Renderer::BakeSphereVolume(CustomVertex* gpu_vtx, std::uint32_t* gpu_idx, Shar::sim::SphereVolume* vol, D3DCOLOR color) {
	if (!vol) return;

}

class Context {
public:
	Renderer m_renderer{};
};

inline Renderer g_context{};

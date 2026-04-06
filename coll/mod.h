#pragma once

#include "shar.h"
#include "gui.h"

struct CustomVertex {
	float x, y, z, nx, ny, nz;
	std::uint32_t color;
	std::uint32_t pad_1c;

	static constexpr std::uint32_t FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;
};

class Buffer {
public:
	auto lock(IDirect3DDevice8* device, std::size_t max_vertices) -> HRESULT;
	auto unlock() { m_vertex_buffer->Unlock(); };

	static constexpr std::size_t MAX_TRIANGLE_VERTICES = 100000;

	IDirect3DVertexBuffer8* m_vertex_buffer = nullptr;
	CustomVertex* m_gpu_data = nullptr;

	std::size_t m_vertex_count = 0;
	std::size_t m_primitive_count = 0;
	std::size_t m_primitive_count_line = 0;
};

class Fence {
public:
	auto bake_active(Buffer& buffer, shar::SwapArray<shar::FenceEntityDSG*> fence_array, D3DXVECTOR3 position, float radiussq) ;
	auto bake_passive(Buffer& buffer, shar::SwapArray<shar::FenceEntityDSG*> fence_array, float y, bool draw_nonpassive);

	static constexpr std::size_t MAX_FENCES = 5000;
	static constexpr std::size_t MAX_COLLIDABLE_COUNT = 8;
	static constexpr std::int32_t SEGMENTS_PER_PERIMETER = 36;

	std::size_t m_active_count = 0;
		
	bool m_draw = true;

	float m_height = 2.f;
	float m_y_offset = 1.f;

	D3DXCOLOR m_color_passive = (D3DXCOLOR)0xfffffffff;
	D3DXCOLOR m_color_active = (D3DXCOLOR)0xffffff00;
	D3DXCOLOR m_color_collidable = (D3DXCOLOR)0xffff0000;
	D3DXCOLOR m_color_overflow = (D3DXCOLOR)0xff00ff00;
};

class RenderState {
public:
	bool lighting = true;
	bool normalize = true;
	bool colorVertex = true;
	bool alphaBlendEnable = true;
	bool zEnable = true;
	bool zWriteEnable = true;
	std::uint32_t diffuseMaterialSource = D3DMCS_COLOR1;
	std::uint32_t srcBlend = D3DBLEND_ONE;
	std::uint32_t destBlend = D3DBLEND_ZERO;
	std::uint32_t cullMode = D3DCULL_NONE;

	const char* blend[14] = {
		"##D3DBLEND",
		"ZERO##D3DBLEND",
		"ONE##D3DBLEND",
		"SRCCOLOR##D3DBLEND",
		"INVSRCCOLOR##D3DBLEND",
		"SRCALPHA##D3DBLEND",
		"INVSRCALPHA##D3DBLEND",
		"DESTALPHA##D3DBLEND",
		"INVDESTALPHA##D3DBLEND",
		"DESTCOLOR##D3DBLEND",
		"INVDESTCOLOR##D3DBLEND",
		"SRCALPHASAT##D3DBLEND",
		"BOTHSRCALPHA##D3DBLEND",
		"BOTHINVSRCALPHA##D3DBLEND"
	};
	const char* materialcolorsource[3] = {
		"MATERIAL##D3DMCS",
		"COLOR1##D3DMCS",
		"COLOR2##D3DMCS"
	};
	const char* cullmode[4] = {
		"##CULLMODE",
		"NONE##CULLMODE",
		"CW##CULLMODE",
		"CCW##CULLMODE"
	};

	auto SetStates(IDirect3DDevice8* device, D3DMATERIAL8 mat) {
		device->SetMaterial(&mat);
		device->SetTexture(0, NULL);

		device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, diffuseMaterialSource);\

		device->SetRenderState(D3DRS_LIGHTING, lighting);
		device->SetRenderState(D3DRS_NORMALIZENORMALS, normalize);
		device->SetRenderState(D3DRS_COLORVERTEX, colorVertex);
		device->SetRenderState(D3DRS_ALPHABLENDENABLE, alphaBlendEnable);
		device->SetRenderState(D3DRS_ZENABLE, zEnable);
		device->SetRenderState(D3DRS_ZWRITEENABLE, zWriteEnable);
		device->SetRenderState(D3DRS_SRCBLEND, srcBlend);
		device->SetRenderState(D3DRS_DESTBLEND, destBlend);
		device->SetRenderState(D3DRS_CULLMODE, cullMode);
	}
};

class CollVisContext {
public:
	CollVisContext() = default;
	CollVisContext(IDirect3DDevice8* device);
	auto batch_active(shar::SpatialNode& node, D3DXVECTOR3 position, float radiussq);
	auto batch_passive(shar::SpatialNode& node, float y);
	auto iter();
	auto commit() -> void;
	auto draw() -> void;
	auto end_scene() -> void;

	static constexpr std::size_t MAX_VERTICES = Fence::MAX_FENCES * 6;

	D3DMATERIAL8 m_material;
	IDirect3DDevice8* m_device;

	Buffer m_buffer{};
	Gui m_gui{};
	RenderState m_state{};
	Fence m_fence{};

	bool m_inited = false;

	bool m_draw_passive = true;
	bool m_draw_active = true;

	bool m_freeze_y = false;
	bool m_freeze_position = false;
	bool m_freeze_radius = false;

	shar::SphereSP m_volume{};
	float m_radius{};

	bool m_show_gui = true;
};

inline CollVisContext g_collviscontext{};

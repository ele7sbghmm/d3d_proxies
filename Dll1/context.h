#pragma once

#include "vertex.h"
#include "render.h"

extern IDirect3DDevice8* g_device;

class Buffer {
public:
	static constexpr size_t MAX_VERTICES = 10000;
	static constexpr size_t MAX_VERTICES_LENGTH = MAX_VERTICES * XYZNormalColor::STRIDE;

	size_t m_length = 0;
	size_t m_primitiveCount = 0;

	IDirect3DVertexBuffer8* m_vertexBuffer = nullptr;
	XYZNormalColor::ArrayType m_vertices{};

	bool m_initialized = false;

	auto Initialize() -> void;
	auto Flush() -> void;
	auto Clear() -> void;
	auto Draw() const -> void;
};

class RenderContext {
public:
	Buffer m_buffer_static{};
	Buffer m_buffer_dynamic{};
	Buffer m_buffer_transient{};

	FenceRenderer2d m_fences2d{};

	auto Initialize() -> void;
	auto PopulateTransient() -> void;
	auto Draw() const -> void;
};

#pragma once

#include <span>
#include <vector>
#include "srr2_types.h"
#include "types.h"

inline size_t MAX_VERTICES = (std::numeric_limits<unsigned short>::max)();
inline IDirect3DIndexBuffer8* trilistIndexBuffer{};
inline IDirect3DIndexBuffer8* linelistIndexBuffer{};

struct FenceCache {
	std::vector<DVertexNormalColor> vertices{};
	IDirect3DVertexBuffer8* vertexBuffer{};
	std::vector<std::span<FenceEntityDSG*>> fenceLoadList{};

	bool inited{};

	auto Init();
	auto Fetch();
	auto Populate();
	auto Clear();
	auto Draw() -> void;
};

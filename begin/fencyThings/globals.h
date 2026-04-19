#pragma once

#include <limits>
#include "render.h"
#include "dsg.h"
#include "types.h"

struct FenceCache {
	struct CaiStats {
		D3DXVECTOR3 position{};
		float radius{};

		std::vector<FenceEntityDSG*> thisFrameCache{};
		std::vector<FenceEntityDSG*> thisFrameCacheInRange{};

		bool notUseless = false;

		auto Update(D3DXVECTOR3* p, float r, FenceEntityDSG* f, bool inRange) -> void;
		auto Init() -> void;
		auto Clear() -> void;
	};

	static const size_t MAX_VERTICES = (std::numeric_limits<unsigned short>::max)();
	static const size_t MAX_FENCES = MAX_VERTICES / 4;
	static const size_t MAX_CAIS = 64;

	std::vector<DVertexNormalColor> vertices{};
	std::vector<unsigned short> indicesTriList{};
	std::vector<unsigned short> indicesLineList{};
	std::vector<FenceEntityDSG*> ptrs{};
	std::vector<CaiStats> cais{};

	bool inited = false;

	auto Init() -> void;
	auto InitIndexBuffers() -> void;
	auto Add(FenceEntityDSG* fence) -> void;
	auto Reset() -> void;
	auto Update() -> void;
	auto Draw() -> void;
};

namespace Globals {
	inline IDirect3DDevice8* GameDevice{};
	inline IDirect3DDevice8* ToolDevice{};
}

namespace GameCache {
	extern FenceCache fenceCache;
}

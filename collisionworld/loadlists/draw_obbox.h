#pragma once

#include <vector>
#include <d3dx8.h>
#include "types.h"

struct ObboxCache {
	std::vector<DVertexNormalColor> vertices{};


	auto Init();
	auto Populate();
	auto Clear();
	auto Draw();
};

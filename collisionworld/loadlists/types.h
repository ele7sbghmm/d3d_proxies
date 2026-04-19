#pragma once

#include <d3dx8.h>

struct DVertexNormalColor {
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	DWORD color;

	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;
};

#pragma once

//DWORD    BLACK = 0xff000000;
//DWORD    WHITE = 0xffffffff;
//DWORD DARKGRAY = 0xff202020;
//DWORD      RED = 0xffff0000;
//DWORD   ORANGE = 0xffffff00;
//DWORD    GREEN = 0xff00ff00;
//DWORD     TEAL = 0xff00ffff;
//DWORD     BLUE = 0xff0000ff;

struct DVertexColor {
	D3DXVECTOR3 position;
	DWORD color;

	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};
struct DVertexNormalColor {
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;
	DWORD color;

	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE;
};
struct VertexColor {
	float x, y, z;
	DWORD color;

	static const DWORD FVF = D3DFVF_XYZ | D3DFVF_DIFFUSE;
};

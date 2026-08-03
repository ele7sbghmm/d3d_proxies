#pragma once

#include "d3d9.h"

struct refdef_s
{
	int x, y, width, height;
	float fov_x, fov_y;
	D3DVECTOR vieworg, viewaxis[3];
	int time;
	float zNear, blurRadius;
	int viewIndex;
};

struct GfxViewParms
{
	D3DVECTOR origin, axis[3];
	D3DVIEWPORT9 viewport;
	D3DMATRIX viewMatrix, projectionMatrix, viewProjectionMatrix, inverseViewProjectionMatrix;
	float depthHackNearClip;
};

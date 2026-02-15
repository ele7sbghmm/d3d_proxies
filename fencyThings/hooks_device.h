#pragma once

#include "globals.h"
#include "hooks.h"

extern "C" IDirect3D8* WINAPI Direct3DCreate8(UINT SDKVersion);

HMODULE GetDirect3D8Module();

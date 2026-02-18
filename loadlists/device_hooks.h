#pragma once

#include "globals.h"
#include "hooks.h"

auto STDMETHODCALLTYPE Direct3DCreate8(UINT SDKVersion) ->IDirect3D8*;
auto GetD3D8Module() -> HMODULE;

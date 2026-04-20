#pragma once

#include <MinHook.h>
#include <d3d8.h>

namespace hooks {
  void install_d3d_hooks(IDirect3D8*);
  void install_device_hooks(IDirect3DDevice8*);
  void install_game_hooks();
}

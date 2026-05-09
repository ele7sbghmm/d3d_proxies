#pragma once

namespace hooks {
  namespace d3d {
    void install(IDirect3D8*);
  }
  namespace d3ddevice {
    void install(IDirect3DDevice8*);
  }
}

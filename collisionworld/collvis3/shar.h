#pragma once

#include <d3d8.h>
#include <cstdint>

namespace shar {
  namespace rmt {
    using Vector = D3DVECTOR;
  }
  struct FenceEntityDSG {
    std::uint8_t pad_0[0x3c];
    rmt::Vector mStartPoint;
    rmt::Vector mEndPoint;
    rmt::Vector mNormal;
  };
  struct SpatialNode {};
};

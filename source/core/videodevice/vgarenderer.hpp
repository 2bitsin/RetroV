#pragma once

#include <stop_token>
#include <cstdint>
#include <cstddef>


#include <core/videodevice/vgaregisters.hpp>

namespace core
{
  struct VideoDevice;
}

namespace core::videodevice
{
  struct VGARenderer
  {   
    inline VGARenderer() = default;
    auto Reset();
    auto RenderToClock(uint64_t clock_v, VGARegisters const& state_v, VideoDevice& machine_v) -> uint64_t;
    
  private:
    uint32_t m_hcounter;
    uint32_t m_vcounter;
  };
}
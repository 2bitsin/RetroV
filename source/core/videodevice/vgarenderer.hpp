#pragma once

#include <stop_token>
#include <cstdint>
#include <cstddef>


#include <core/videodevice/vgaregisters.hpp>

namespace core::videodevice
{
  struct VGARenderer
  {   
    constexpr inline VGARenderer() 
      : m_Hcounter{ 0u }
      , m_Vcounter{ 0u }
    {}

    void RenderToClock(std::uint64_t clock_v, VGARegisters const& state_v) 
    {}

  private:
    std::uint32_t m_Hcounter;
    std::uint32_t m_Vcounter;
  };
}
#pragma once

#include <cstdint>
#include <cstddef>

#include <utils/algorithm.hpp>

#include <win32/chrono.hpp>
#include <win32/waitabletimer.hpp>

namespace core::videodevice
{
  struct video_timer
  {
    using ftime_type = win32::filetime_clock::time_point;

    inline video_timer(std::uint64_t ckfreq_v, std::uint32_t htotal_v, std::uint32_t vtotal_v)
    {
      set_clock_freq(ckfreq_v, htotal_v, vtotal_v);
      reset();
    }

    inline auto set_clock_freq(std::uint64_t clockf_v, 
      std::uint64_t htotal_v, std::uint64_t vtotal_v) -> void 
    {
      using namespace win32;
      m_clock_freq = clockf_v;
      m_hori_total = htotal_v;
      m_vert_total = vtotal_v;
    }

    inline auto base() -> win32::filetime_clock::time_point {
      return m_base_ftime;
    }

    inline auto reset () -> void {
      using namespace win32;
      m_base_ftime = filetime_clock::now();      
    }

    inline auto time_since_reset() const -> double {
      using namespace win32;
      using namespace std::chrono;        
      using seconds_d = duration<double>;
      auto const dt100ns_v = filetime_clock::now() - m_base_ftime;
      auto dts_v = duration_cast<seconds_d>(dt100ns_v).count();
      return dts_v;
    }

    inline auto current_clock() const -> std::uint64_t {      
      return static_cast<uint64_t>(time_since_reset() * m_clock_freq);
    }

    inline auto current_dotclock() const -> std::uint32_t {      
      return current_clock() % m_hori_total;
    }

    inline auto current_scanline() const -> std::uint32_t {
      return (current_clock() / m_hori_total) % m_vert_total;
    }

    inline auto current_frame() const -> std::uint32_t {
      return current_clock() / (m_hori_total * m_vert_total);
    }

    inline auto next_sline_time() -> win32::filetime_clock::time_point {
      using namespace std::chrono;
      using namespace win32; 
      using namespace utils;
      using duration = filetime_clock::duration;

      auto const curr_clock_v = current_clock();
      auto const next_clock_v = next_integer_multiple(curr_clock_v, m_hori_total);
      duration const next_100nano_v { static_cast<uint64_t>(next_clock_v * 1e7 / m_clock_freq) };
      return m_base_ftime + next_100nano_v;
    }

    inline auto next_frame_time() -> win32::filetime_clock::time_point {
      using namespace std::chrono;
      using namespace win32;
      using namespace utils;
      using duration = filetime_clock::duration;

      auto const curr_clock_v = current_clock();  
      auto const next_clock_v = next_integer_multiple(curr_clock_v, m_hori_total*m_vert_total);
      duration const next_100nano_v { static_cast<uint64_t>(next_clock_v * 1e7 / m_clock_freq) };
      return m_base_ftime + next_100nano_v;
    }

  private:
    std::uint32_t m_clock_freq { 0u };
    std::uint32_t m_hori_total { 0u };
    std::uint32_t m_vert_total { 0u };
    
    std::uint64_t m_base_ticks { 0u };
    ftime_type    m_base_ftime {};
  };

}
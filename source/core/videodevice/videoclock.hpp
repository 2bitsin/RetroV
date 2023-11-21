#pragma once

#include <cstdint>
#include <cstddef>

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
      auto const pfreq_v = win32::query_performance_frequency();
      m_clock_freq = clockf_v;
      m_hori_total = htotal_v;
      m_vert_total = vtotal_v;
      m_sline_time = (m_hori_total * pfreq_v) / m_clock_freq;
      m_frame_time = (m_vert_total * m_hori_total * pfreq_v) / m_clock_freq;
    }

    inline auto reset () -> void {
      m_base_ftime = win32::filetime_clock::now();
      m_base_ticks = win32::query_performance_counter();
    }

    inline auto ticks_since_reset() const -> std::tuple<std::uint64_t, std::uint64_t> {
      return { win32::query_performance_counter() - m_base_ticks, win32::query_performance_frequency() };
    }

    inline auto current_clock() const -> std::uint64_t {
      auto const [pcter_v, pfreq_v] = ticks_since_reset();
      return (pcter_v * m_clock_freq) / pfreq_v; 
    }

    inline auto current_dotclock() const -> std::uint32_t {      
      return current_clock() % m_hori_total;
    }

    inline auto current_scanline() const -> std::uint32_t {
      auto const [pcter_v, pfreq_v] = ticks_since_reset();
      return (pcter_v - m_base_ticks) / m_sline_time;
    }

    inline auto current_frame() const -> std::uint32_t {
      auto const pcter_v = win32::query_performance_counter();
      auto const pfreq_v = win32::query_performance_frequency();
      return (pcter_v - m_base_ticks) / m_frame_time; 
    }

    inline auto next_scanline_time() -> win32::filetime_clock::time_point {
      using namespace win32;
      using duration = win32::filetime_clock::duration;
      auto const q = query_performance_counter() - m_base_ticks;
      auto const f = query_performance_frequency();
      auto const t = 10'000'000ull;
      return m_base_ftime + duration{ (((q * m_clock_freq / f) / m_sline_time) + 1u) * m_sline_time * t / m_clock_freq };
    }

    inline auto next_frame_time() -> win32::filetime_clock::time_point {
      using namespace win32;
      auto const q = query_performance_counter() - m_base_ticks;
      auto const f = query_performance_frequency();
      auto const t = 10'000'000ull;
      using duration = win32::filetime_clock::duration;
      return m_base_ftime + duration{ (((q * m_clock_freq / f) / m_frame_time) + 1u) * m_frame_time * t / m_clock_freq };
    }

  private:
    std::uint32_t m_clock_freq { 0u };
    std::uint32_t m_hori_total { 0u };
    std::uint32_t m_vert_total { 0u };
    std::uint32_t m_sline_time { 0u };
    std::uint32_t m_frame_time { 0u };
    
    std::uint64_t m_base_ticks { 0u };
    ftime_type    m_base_ftime {};
  };

}
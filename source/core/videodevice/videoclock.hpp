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
    using duration = win32::filetime_clock::duration;
    using time_point = win32::filetime_clock::time_point;
    using dseconds = std::chrono::duration<double>;
    using waitable = win32::waitable_timer;

    inline video_timer(uint64_t ckfreq_v, uint32_t htotal_v, uint32_t vtotal_v)
      : m_last_sync_time   { win32::filetime_clock::now() }
      , m_clock_frequency  { ckfreq_v }
      , m_horizontal_total { htotal_v }
      , m_vertical_total   { vtotal_v }
      , m_waitable_timer   {          }
    {}

    inline auto set_clock_freq(uint64_t clockf_v, uint64_t htotal_v, uint64_t vtotal_v) 
      -> void 
    {
      m_clock_frequency = clockf_v;
      m_horizontal_total = htotal_v;
      m_vertical_total = vtotal_v;
      sync_clock();
    }

    inline auto last_sync_time() const noexcept -> time_point {
      return m_last_sync_time;
    }

    inline auto sync_clock () -> void {
      using namespace win32;
      m_last_sync_time = filetime_clock::now();      
    }

    inline auto time_since_sync() const -> dseconds {
      using namespace win32;
      using namespace std::chrono;        
      return dseconds{ (filetime_clock::now() - last_sync_time()).count() * 1e-7 };
    }

    inline auto current_clock() const -> uint64_t {      
      return static_cast<uint64_t>(time_since_sync().count() * m_clock_frequency);
    }

    inline auto current_dotclock() const -> uint64_t {      
      return current_clock() % m_horizontal_total;
    }

    inline auto current_scanline() const -> uint64_t {
      return (current_clock() / m_horizontal_total) % m_vertical_total;
    }

    inline auto current_frame() const -> uint64_t {
      return current_clock() / (m_horizontal_total * m_vertical_total);
    }

    inline auto calculate_dotclock(uint64_t dotclock_v) const noexcept -> time_point {
      duration const offset100ns_v{ static_cast<uint64_t>(
        (dotclock_v * 1e7) / m_clock_frequency) };
      return last_sync_time() + offset100ns_v;
    }

    inline auto calculate_sline_clock(uint64_t sline_v) const noexcept -> time_point {
      return calculate_dotclock(sline_v * m_horizontal_total);
    }

    inline auto calculate_frame_clock(uint64_t frame_v) const noexcept -> time_point {
      return calculate_sline_clock(frame_v * m_vertical_total);
    }

    inline auto wait_until_dotclock(uint64_t clock_v) -> bool {
      auto time_v = calculate_dotclock(clock_v);
      if (time_v <= win32::filetime_clock::now() + duration{ 1 })
        return false;
      m_waitable_timer.set(time_v);
      m_waitable_timer.wait();
      return true;
    }

    inline auto wait_until_sline(uint64_t sline_v) -> bool {
      return wait_until_dotclock(sline_v * m_horizontal_total);
    }

    inline auto wait_until_frame(uint64_t frame_v) -> bool {
      return wait_until_sline(frame_v * m_vertical_total);
    }
    

  private:
    time_point m_last_sync_time {    };
    uint64_t m_clock_frequency  { 0u };
    uint32_t m_horizontal_total { 0u };
    uint32_t m_vertical_total   { 0u };    
    waitable m_waitable_timer   {    };
  };

}
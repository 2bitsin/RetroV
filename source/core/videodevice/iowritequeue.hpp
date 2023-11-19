#pragma once

#include <optional>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <deque>
#include <mutex>

namespace core::videodevice
{

  struct io_write_queue
  {    
  #pragma pack(push, 1)
    struct value_type
    {
      constexpr inline value_type
      (std::uint32_t time_v
        , std::uint32_t data_v
        , std::uint16_t size_v
        , std::uint16_t port_v)
        : time{ time_v }
        , data{ data_v }
        , size{ size_v }
        , port{ port_v }
      {}
      std::uint32_t time;
      std::uint32_t data;
      std::uint16_t size;
      std::uint16_t port;
    };
  #pragma pack(pop)


    inline io_write_queue() = default;

    inline auto push(uint32_t time_v, uint16_t port_v, uint16_t size_v, uint32_t data_v) -> void {
      std::lock_guard const lock_v{ m_Mutex };
      m_Queue.emplace_back(time_v, data_v, size_v, port_v);
    }

    inline auto pop_before(uint32_t time_v = std::numeric_limits<uint32_t>::max()) 
      -> std::optional<value_type> 
    {
      std::lock_guard const lock_v{ m_Mutex };
      if (m_Queue.empty()) 
        return std::nullopt;
      if (m_Queue.front().time > time_v)
        return std::nullopt;
      auto const value_v = std::move(m_Queue.front());
      m_Queue.pop_front();
      return value_v;
    }

  private:
  
    std::mutex m_Mutex;
    std::deque<value_type> m_Queue;
  };
}
#pragma once

#include <win32/windows.hpp>
#include <win32/error.hpp>

#include <type_traits>
#include <memory>
#include <future>
#include <thread>

namespace win32
{

  struct current_thread
  {
    static inline auto native_handle() noexcept -> void* {
      return ::GetCurrentThread();
    }
  };

  static inline auto drain_apc_queue() -> void {
    while (WAIT_IO_COMPLETION == ::SleepEx(0, TRUE));
  }

  struct drain_apc_queue_guard {
    ~drain_apc_queue_guard() {
      drain_apc_queue();
    }
  };

  template <typename Func>
  struct apc_future: std::future<Func> 
  {
    using std::future<Func>::future;

    template <typename... Args>
    inline apc_future(Args&&...args) 
      : std::future<Func>(std::forward<Args>(args)...)
    {}

    inline auto get() 
      -> decltype(std::future<Func>::get())
    {
      drain_apc_queue();
      return std::future<Func>::get();
    }

    inline auto valid() const noexcept 
      -> decltype(std::future<Func>::valid())
    {
      return std::future<Func>::valid();
    }

    inline auto wait() -> decltype(std::future<Func>::wait())
    {
      drain_apc_queue();
      return std::future<Func>::wait();
    }

    template <typename Q>
    inline auto wait_for(Q&& timeout_v) 
      -> decltype(std::future<Func>::wait_for(timeout_v))
    {
      drain_apc_queue();
      return std::future<Func>::wait_for(std::forward<Q>(timeout_v));
    }

    template <typename Q>
    inline auto wait_until(Q&& timeout_v) 
      -> decltype(std::future<Func>::wait_until(timeout_v))
    {
      drain_apc_queue();
      return std::future<Func>::wait_until(std::forward<Q>(timeout_v));
    }
  };

  template <typename _Thread, typename _Callee, typename... _Args>
  static inline auto queue_user_apc(_Thread const& thread_v, _Callee&& callee_v, _Args&&...args_v)
    -> apc_future<std::invoke_result_t<_Callee, _Args...>>
  {
    /*****************************************************************************
     *   
     *  WARNING! If the task is never actually executed, the task_ptr will leak.
     *  TODO: Need a way to ensure task_ptr is released even if APC does not run. 
     * 
     *****************************************************************************/

    using Return = std::invoke_result_t<_Callee, _Args...>;
    using Task = std::packaged_task<Return()>;

    auto const task_ptr = new Task(std::bind(std::forward<_Callee>(callee_v), std::forward<_Args>(args_v)...));
    auto future_v = task_ptr->get_future();
    auto const handle_v = static_cast<HANDLE>(thread_v.native_handle());
    auto const arg_uptr = std::bit_cast<uintptr_t>(task_ptr);
    
    static constexpr auto wrapper_s = [](ULONG_PTR task_ptr_v) -> void {
      auto task_ptr = std::bit_cast<Task*>(task_ptr_v);
      (*task_ptr)();
      delete task_ptr;
    };

    if (!::QueueUserAPC(wrapper_s, handle_v, arg_uptr)) {
      error::throw_last_error();
    }

    return future_v;
  }

}
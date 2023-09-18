#pragma once

#include <optional>
#include <atomic>
      #include <mutex>
#include <list>

namespace utils
{
	template <typename T, std::size_t SpinCount = 1u>
	struct coqueue
	{

		template <typename... Args>
		inline auto try_emplace(Args&&... args)
		{
			using namespace std;
			unique_lock const lock_v{ m_mutex, 
				try_to_lock };
			if (!lock_v.owns_lock())
				return false;
			m_store.emplace_back(
				forward<Args>(args)...);			
			return true;
		}

		template <typename... Q>
		inline auto emplace(Q&&... args)
		{
			using namespace std;
			if constexpr (SpinCount > 0u) 
			{
				auto spins_left_v = SpinCount;
				for (;spins_left_v > 0u; 
					spins_left_v--)
				{
					if (!try_emplace(
						forward<Q>(args)...))
						continue;
					return;				
				}
			}
			unique_lock const lock_v{ m_mutex };
			m_store.emplace_back(
				forward<Q>(args)...);
		}

		inline auto try_pop() -> std::optional<T>
		{
			using namespace std;
			unique_lock const lock_v{ m_mutex, 
				try_to_lock };
			if (!lock_v.owns_lock())
				return nullopt;
			if (m_store.empty()) return nullopt;
			auto const value_v = move(
				m_store.front());
			m_store.pop_front();
			return value_v;
		}

		inline auto pop() -> std::optional<T>
		{
			using namespace std;
			if constexpr (SpinCount > 0u) 
			{
				auto spins_left_v = SpinCount;
				for (;spins_left_v > 0u; 
					spins_left_v--)
				{
					if (auto const value_v = try_pop();
						value_v.has_value())
						return value_v;
				}
			}
			unique_lock const lock_v{ m_mutex };
			if (m_store.empty()) return nullopt;
			auto const value_v = move(
				m_store.front());
			m_store.pop_front();			
			return value_v;
		}

		auto empty() const noexcept -> bool
		{
			using namespace std;
			unique_lock const lock_v{ m_mutex };
			return m_store.empty();
		}


	private:
		mutable std::mutex m_mutex;
		std::list<T> m_store;
	};

}
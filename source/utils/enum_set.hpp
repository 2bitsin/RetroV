#pragma once

namespace utils
{
	template <std::integral T>
	struct enum_set
	{
		enum_set(T value = T()) noexcept: m_value { value } {}

		operator T () const noexcept { return m_value ; }

		template <typename U>	
		requires (std::is_enum_v<std::remove_cvref_t<U>>)
		bool contains(U&& value) const
		{
			using ul_type = std::underlying_type_t<U>;
			assert ((ul_type)value < sizeof(T)*8u);
			return m_value & (T(1u) << (ul_type)value);
		}

		template <typename... U>
		requires (sizeof...(U) > 0)
		auto any_of(U&& ... values) const -> bool
		{
			return (contains(std::forward<U>(values)) || ...);
		}

	private:

		T m_value;
	};
}
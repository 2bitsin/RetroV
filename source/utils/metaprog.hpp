#pragma once

namespace utils::metaprog 
{
	template<typename... T> struct type_list {};
	template<typename... L> struct concat;
	template<> struct concat<> { using type = type_list<>; };
	template<typename... T> struct concat<type_list<T...>> { using type = type_list<T...>; };
	template<typename... T1, typename... T2, typename... R>
	struct concat<type_list<T1...>, type_list<T2...>, R...>
		: concat<type_list<T1..., T2...>, R...>
	{};


	template<auto... V> struct value_list {};	
	template<typename... L> struct vl_concat;
	template<> struct vl_concat<> { using type = value_list<>; };
	template<auto... V> struct vl_concat<value_list<V...>> { using type = value_list<V...>; };
	template<auto... V1, auto... V2, typename... R>
	struct vl_concat<value_list<V1...>, value_list<V2...>, R...>
		: vl_concat<value_list<V1..., V2...>, R...>
	{};

	template<typename... L> using vl_concat_t = typename vl_concat<L...>::type;
	template<typename... L> using concat_t = typename concat<L...>::type;

	template <typename T> 
	struct is_type_list: std::false_type {};

	template <typename... T>
	struct is_type_list<type_list<T...>>: std::true_type {};

	template <typename T>
	constexpr bool is_type_list_v = is_type_list<T>::value;

	template <typename T>
	struct is_value_list: std::false_type {};

	template <auto... V>
	struct is_value_list<value_list<V...>>: std::true_type {};

	template <typename T>
	constexpr bool is_value_list_v = is_value_list<T>::value;

	namespace concepts
	{
		template <typename T>
		concept type_list = is_type_list_v<T>;

		template <typename T>
		concept value_list = is_value_list_v<T>;
	}
}

namespace ump = utils::metaprog;
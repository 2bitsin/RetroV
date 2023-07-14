#define TEST_MODE
#include <utils/interval_map.hpp>

#include <random>
#include <type_traits>
#include <iostream>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <format>
#include <vector>


template <typename I, typename V>
struct control_map
{
	static_assert(std::is_unsigned_v<I>);

	control_map(I max_value, V def_value = 0u) {
		m_map.resize(max_value+1u, def_value);
	}

	auto insert(std::pair<I, I> const& bounds_v, V const& value_v) {
		for (auto i = bounds_v.first; i < bounds_v.second; ++i) {
			m_map[i] = value_v;
		}
	}

	auto size() const {
		return m_map.size();
	}

	auto at(I const& index) {
		return m_map.at(index);
	}

	std::vector<V> m_map;
};



void TEST_interval_map() 
{
	using utils::interval_map;
	
	static constexpr const auto max_value = 0xFFFFu;

	static constexpr const auto X = 1000000000u;

	std::random_device rdrand{};
	std::mt19937_64 rng (rdrand());
	std::uniform_int_distribution<std::uint64_t> rand(0u, max_value);

	interval_map<std::uint64_t, std::uint64_t> testmap_v(X);
	control_map<std::uint64_t, std::uint64_t> control_v(max_value, X);

	std::uint64_t number_of_tests = 0u;
	while(number_of_tests < 10000u)
	{		
		for (auto i = 0u; i < 10u; ++i) {
			auto start_v = rand(rng);
			auto end_v = rand(rng);
			if (start_v == end_v) end_v += 1u;
			else if (start_v > end_v) std::swap(start_v, end_v);
			auto const uuid_v = X + rand(rng) + 1u;
			//std::cout<<std::format("[{}, {}] = {}\n", start_v, end_v, uuid_v);
			testmap_v.insert({start_v, end_v}, uuid_v);
			control_v.insert({start_v, end_v}, uuid_v);
		}

		for (auto j = 0u; j < control_v.size(); ++j) {
			auto acquired_v = testmap_v.at(j);
			auto expected_v = control_v.at(j);
			if (acquired_v != expected_v)
				std::cout<<std::format("{}: {} != {}\n", j, expected_v, acquired_v);
			assert(acquired_v == expected_v);
		}
		number_of_tests += 1u;
	}

}
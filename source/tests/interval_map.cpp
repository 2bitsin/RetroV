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

	auto value_at(I const& index) {
		return m_map.at(index);
	}

	std::vector<V> m_map;
};



void TEST_fuzz_test_interval_map() 
{
	using utils::interval_map;
	
	static constexpr const auto max_value = 0xFFFFu;

	static constexpr const auto X = 1000000000u;

	std::random_device rdrand{};
	std::mt19937_64 rng (rdrand());
	std::uniform_int_distribution<uint64_t> rand(0u, max_value);

	interval_map<uint64_t, uint64_t> testmap_v(X);
	control_map<uint64_t, uint64_t> control_v(max_value, X);

	uint64_t number_of_tests = 0u;
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
			auto acquired_v = testmap_v.value_at(j);
			auto expected_v = control_v.value_at(j);
			if (acquired_v != expected_v)
				std::cout<<std::format("{}: {} != {}\n", j, expected_v, acquired_v);
			assert(acquired_v == expected_v);
		}
		number_of_tests += 1u;
	}
}

struct ihello {
	virtual ~ihello() = default;
	virtual void greetings() const = 0;
};

struct world_hello : ihello {
	void greetings() const override {
		std::cout<<"Hello, World!\n";
	}
};

struct universe_hello : ihello {
	void greetings() const override {
		std::cout<<"Hello, Universe!\n";
	}
};

struct galaxy_hello : ihello {
	void greetings() const override {
		std::cout<<"Hello, Galaxy!\n";
	}
};

void TEST_interval_map_pointers() {
	
	using utils::interval_map;

	auto hello1_v = std::make_unique<world_hello>();
	auto hello2_v = std::make_unique<universe_hello>();
	auto hello3_v = std::make_unique<galaxy_hello>();

	interval_map<uint64_t, ihello const*> testmap_v(hello1_v.get());

	testmap_v.insert({ 5u, 10u}, hello1_v.get());
	testmap_v.insert({10u, 30u}, hello2_v.get());
	testmap_v.insert({15u, 20u}, hello3_v.get());

	for (auto i = 0u; i < 30u; ++i) {
		auto const hello_v = testmap_v.value_at(i);
		std::cout << std::format("{:<3}: ", i);
		hello_v->greetings();
	}

	__debugbreak();
}


void TEST_interval_map() {

	using utils::interval_map;
	interval_map<uint64_t, uint8_t> testmap_v(0x00u);

	testmap_v.insert({  5u, 10u }, 0x10u);
	testmap_v.insert({ 10u, 30u }, 0x20u);
	testmap_v.insert({ 15u, 20u }, 0x30u);


	for (auto&& [lhs_v, rhs_v, value_v] : testmap_v) {
		std::cout << std::format("[{:<3} ... {:<3}] => {:#04x}\n", lhs_v, rhs_v, value_v);
	}
	 
	__debugbreak();
}
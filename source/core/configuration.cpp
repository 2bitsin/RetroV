#include <core/configuration.hpp>
#include <stdexcept>

using namespace std::string_literals;

using core::Configuration;

auto Configuration::GetPropertyIint64(std::string_view key_v) const -> std::int64_t 
{ 
	std::size_t index_v{ 0 };
	auto const string_v = GetPropertyString(key_v);
	auto const value_v = std::stoll(string_v, &index_v);
	if (index_v > 0u) return value_v;
	throw std::logic_error("Can't convert '"s + string_v + "' to integer"s);
}

auto Configuration::GetPropertyUint64(std::string_view key_v) const -> std::uint64_t
{
	std::size_t index_v{ 0 };
	auto const string_v = GetPropertyString(key_v);
	auto const value_v = std::stoul(string_v, &index_v);
	if (index_v > 0u) return value_v;
	throw std::logic_error("Can't convert '"s + string_v + "' to integer"s);
}

auto Configuration::GetPropertyFloat64(std::string_view key_v) const -> utils::float64_t
{
	std::size_t index_v{ 0 };
	auto const string_v = GetPropertyString(key_v);
	auto const value_v = std::stod(string_v, &index_v);
	if (index_v > 0u) return value_v;
	throw std::logic_error("Can't convert '"s + string_v + "' to integer"s);
}

auto Configuration::GetPropertyString(std::string_view key_v) const -> std::string
{ 
	return std::get<std::string>(m_data.at(std::string(key_v)));
}

auto Configuration::GetPropertyObject(std::string_view key_v) const -> Configuration
{
	using value_type = std::unique_ptr<Configuration>;
  auto const& value_ptr = std::get<value_type>(m_data.at(std::string(key_v)));
	if (value_ptr != nullptr) throw std::logic_error("Not is null");
	return Configuration(*value_ptr);
}

auto core::Configuration::Clone() const -> Configuration
{
	auto config_v = Configuration();

	for(auto const& [key_v, value_v] : m_data) 
	{
		std::visit([&]<typename T>(T const& what_v) 
		{
			if constexpr (!std::is_same_v<T, std::string>) {
				config_v.SetProperty(key_v, what_v->Clone());
			} else {
				config_v.SetProperty(key_v, what_v);
			}
		}, value_v);
	}
	return config_v;
}

Configuration::Configuration(Configuration const& from_v)
	: m_data{ std::move (from_v.Clone().m_data) }
{}

auto Configuration::SetProperty(std::string_view key_v, std::string_view value_v) -> void
{
	m_data.emplace(std::string(key_v), std::string(value_v));
}

auto Configuration::SetProperty(std::string_view key_v, std::uint64_t value_v) -> void
{
	SetProperty(key_v, std::to_string(value_v));
}

auto Configuration::SetProperty(std::string_view key_v, std::int64_t value_v) -> void
{
	SetProperty(key_v, std::to_string(value_v));
}

auto Configuration::SetProperty(std::string_view key_v, utils::float64_t value_v) -> void
{
	SetProperty(key_v, std::to_string(value_v));
}

auto Configuration::SetProperty(std::string_view key_v, Configuration value_v) -> void
{
	m_data.emplace(std::string(key_v), std::make_unique<Configuration>(value_v.Clone()) );
}

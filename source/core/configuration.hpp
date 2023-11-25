#pragma once

#include <unordered_map>
#include <string_view>
#include <variant>
#include <string>
#include <memory>

#include <utils/float.hpp>

namespace core
{
	struct Configuration
	{
		auto GetPropertyIint64(std::string_view) const ->int64_t;
		auto GetPropertyUint64(std::string_view) const ->uint64_t;
		auto GetPropertyString(std::string_view) const -> std::string;
		auto GetPropertyFloat64(std::string_view) const -> utils::float64_t;
		auto GetPropertyObject(std::string_view) const -> Configuration;

		auto SetProperty(std::string_view, std::string_view) -> void;
		auto SetProperty(std::string_view, uint64_t) -> void;
		auto SetProperty(std::string_view, int64_t) -> void;
		auto SetProperty(std::string_view, utils::float64_t) -> void;
		auto SetProperty(std::string_view, Configuration) -> void;

		auto Clone() const->Configuration;

		Configuration() = default;
		Configuration(Configuration const& from_v);
		Configuration(Configuration&& from_v) noexcept = default;
		auto operator = (Configuration const& from_v)->Configuration & = default;
		auto operator = (Configuration&& from_v) noexcept -> Configuration & = default;

	private:
		using node_type = std::variant<std::string, std::unique_ptr<Configuration>>;

		std::unordered_map<std::string, node_type> m_data;
	};
}
#pragma once

#include <filesystem>

#include <core/mapgparange.hpp>
#include <win32/mappedfile.hpp>
#include <win32/whvpartition.hpp>

#include <utils/region.hpp>

namespace core
{
	struct Machine;
	
	struct RomImage
	{
		using partition_type = win32::WHvPartition;
		using region_type = utils::region64_type;
		using mapped_type = win32::MappedFile;

		struct validate 
		{
			inline constexpr validate(
				std::size_t granularity_v, 
				std::size_t min_size_v, 
				std::size_t max_size_v)
				: granularity{ granularity_v }
				, min_size{ min_size_v }
				, max_size{ max_size_v }
			{}

			std::size_t const granularity{ 0x1000u };
			std::size_t const min_size{ 0x00000000000001ull };
			std::size_t const max_size{ 0x10000000000000ull };
		};
		
		static inline constexpr auto kTopAligned = 0x1u;

		RomImage(partition_type& partition_v, 
			validate const& validate_v,
			std::filesystem::path const& image_path_v, 
			region_type target_region_v,
			std::uint32_t flags_v = 0u,
			region_type source_region_v = {});
			
		RomImage(partition_type& partition_v,
			std::filesystem::path const& image_path_v, 
			region_type target_region_v,
			std::uint32_t flags_v = 0u,
			region_type source_region_v = {});

		~RomImage() = default;

		RomImage(RomImage const&) = delete;
		auto operator=(RomImage const&) -> RomImage& = delete;

		RomImage(RomImage&&) = default;
		auto operator=(RomImage&&) -> RomImage& = default;

		auto Enable() -> void;
		auto Disable() -> void;
		auto Remap(region_type target_region_v) -> void;
	private:
		mapped_type m_Image;
		MapGpaRange m_Mapping;

		static inline const EventLog s_log{ "RomImage" };
	};
	

}
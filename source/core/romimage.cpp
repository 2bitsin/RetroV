#include <core/romimage.hpp>

#include <win32/mappedfile.hpp>

#include <utils/validate.hpp>
#include <utils/paths.hpp>

using core::RomImage;

RomImage::RomImage(partition_type& partition_v, 
	validate const& validate_v, 
	std::filesystem::path const& image_path_v, 
	region_type target_region_v,
	uint32_t flags_v,
	region_type source_region_v)
	: m_Image{
			utils::validate_binary(image_path_v, validate_v.granularity, validate_v.min_size, validate_v.max_size), 
			source_region_v,
			win32::cf_mode::open_existing,
			win32::page_prot::execute_write_copy,
			win32::share_type::share_read
	}
	, m_Mapping{
			partition_v, !(flags_v&kTopAligned) 
			? target_region_v.first(m_Image.Size())
			: target_region_v.last(m_Image.Size()),
			kAccessReadOnly, m_Image
	}
{
	s_log.MapGpaRangeFromFile(
		m_Image.Data().data(),
		(!(flags_v & kTopAligned)
			? target_region_v.first(m_Image.Size())
			: target_region_v.last(m_Image.Size())).last_sync_time(),
		std::min(target_region_v.size(),
			m_Image.Data().size()),
		image_path_v,
		source_region_v.last_sync_time(),
		m_Image.Data().size()
	);
}

RomImage::RomImage(partition_type& partition_v,
	std::filesystem::path const& image_path_v, 
	region_type target_region_v,
	uint32_t flags_v,
	region_type source_region_v)
	: m_Image   {
			utils::build_path(image_path_v), 
			source_region_v, 
			win32::cf_mode::open_existing,
			win32::page_prot::execute_write_copy, 
			win32::share_type::share_read 
		}
	, m_Mapping { 
			partition_v, !(flags_v&kTopAligned)
			? target_region_v.first(m_Image.Size())
			: target_region_v.last(m_Image.Size()),
			kAccessReadOnly, m_Image
		}
{
	s_log.MapGpaRangeFromFile(
		m_Image.Data().data(),
		(!(flags_v & kTopAligned)
			? target_region_v.first(m_Image.Size())
			: target_region_v.last(m_Image.Size())).last_sync_time(),
		std::min(target_region_v.size(),
			m_Image.Data().size()), 
		image_path_v, 
		source_region_v.last_sync_time(), 
		m_Image.Data().size()
	);
}


auto RomImage::Enable() -> void
{
	return m_Mapping.Enable();
}

auto RomImage::Disable() -> void
{
	return m_Mapping.Disable();
}

auto RomImage::Remap(region_type target_region_v) -> void
{
	return m_Mapping.Remap(target_region_v, kAccessReadOnly);	
}

auto RomImage::Patch(size_t offset_v, std::span<std::byte const> bytes_v) -> void
{
	auto data_v = m_Image.Data();
	auto size_v = std::min(bytes_v.size(), 
		data_v.size() - offset_v);
	if (size_v <= 0u) return;
	std::copy(bytes_v.begin(), bytes_v.end(), 
		data_v.begin() + offset_v);
}

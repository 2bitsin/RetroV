#include <core/romimage.hpp>

#include <win32/mappedfile.hpp>

#include <utils/validate.hpp>
#include <utils/paths.hpp>

using core::RomImage;

RomImage::RomImage(partition_type& partition_v, 
	validate const& validate_v, 
	std::filesystem::path const& image_path_v, 
	region_type target_region_v, 
	region_type source_region_v)
	: m_Image{
			utils::validate_binary(image_path_v, validate_v.granularity, validate_v.min_size, validate_v.max_size), 
			source_region_v,
			win32::cf_mode::open_existing,
			win32::page_prot::execute_write_copy,
			win32::share_type::share_read
	}
	, m_Mapping{
			partition_v, target_region_v,
			kAccessReadOnly, m_Image
	}
{
}

RomImage::RomImage(partition_type& partition_v,
	std::filesystem::path const& image_path_v, 
	region_type target_region_v,
	region_type source_region_v)
	: m_Image   {
			utils::build_path(image_path_v), 
			source_region_v, 
			win32::cf_mode::open_existing,
			win32::page_prot::execute_write_copy, 
			win32::share_type::share_read 
		}
	, m_Mapping { 
			partition_v, target_region_v, 
			kAccessReadOnly, m_Image
		}
{
	s_log.MapGpaRangeFromFile(
		m_Image.Data().data(),
		target_region_v.base(),
		std::min(target_region_v.size(),
			m_Image.Data().size()), 
		image_path_v, 
		source_region_v.base(), 
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

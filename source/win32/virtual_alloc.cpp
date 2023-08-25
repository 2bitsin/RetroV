#include "virtual_alloc.hpp"

using namespace win32;

auto win32::virtual_alloc(std::size_t size_v, allocation_flags_type flags_v, 
	page_protection_type protect_v, void* target_v) -> void*
{
	return ::VirtualAlloc(target_v, size_v, (DWORD)flags_v, (DWORD)protect_v);  
}

auto win32::virtual_free(void* address_v, std::size_t size_v, 
	allocation_flags_type flags_v) -> bool
{
	return ::VirtualFree(address_v, size_v, (DWORD)flags_v);		
}

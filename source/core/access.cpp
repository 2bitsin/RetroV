#include <core/access.hpp>
#include <win32/windows.hpp>

#include <stdexcept>

auto core::protect_from_access(std::uint32_t access_v) -> std::uint32_t
{
	auto protect_v = 0u;
	switch (access_v)
	{
	case core::access::read:
		protect_v = PAGE_READONLY;
		break;
	case core::access::write:
	case core::access::read | core::access::write:
		protect_v = PAGE_READWRITE;
		break;
	case core::access::execute:
		protect_v = PAGE_EXECUTE;
		break;
	case core::access::read | core::access::execute:
		protect_v = PAGE_EXECUTE_READ;
		break;
	case core::access::write | core::access::execute:
	case core::access::read | core::access::write | core::access::execute:
		protect_v = PAGE_EXECUTE_READWRITE;
		break;
	default:
		throw std::invalid_argument("invalid access");
	}
	return protect_v;
}
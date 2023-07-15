#pragma once 

#include <cstddef>
#include <cstdint>

#include <win32/windows.hpp>
#include <win32/winhvpx.hpp>

#include <utils/objects.hpp>

struct WHvPartition {

private:
	WHV_PARTITION_HANDLE m_Handle{ nullptr };
};

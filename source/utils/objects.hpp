#pragma once

namespace utils
{
	struct unmovable {
		unmovable() = default;
		unmovable(unmovable&&) = delete;
		unmovable& operator=(unmovable&&) = delete;	
	};

	struct uncopyable {
		uncopyable() = default;
		uncopyable(const uncopyable&) = delete;
		uncopyable& operator=(const uncopyable&) = delete;
	};
  
}
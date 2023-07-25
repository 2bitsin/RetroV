#pragma once

#include <type_traits>
#include <utility>

namespace utils
{
	template <typename Object, typename Member> 		
	static inline auto lambda(Object* this_v, Member that_v) {
		return [this_v, that_v]<typename... Args>(Args&&... args) { 
			return (this_v->*that_v)(std::forward<Args>(args)...); 
		};
	}

}
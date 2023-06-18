#pragma once

namespace core
{
	struct object { 
		virtual ~object() = default; 
		object() = default;
		object(const object&) = delete;
		object(object&&) = delete;
		object& operator=(const object &) = delete;
		object& operator=(object &&) = delete;
	};

}
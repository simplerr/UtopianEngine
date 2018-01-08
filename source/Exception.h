#pragma once

namespace
{
	class Exception
	{

	};

#define THROW_EXCEPTION(type, desc)	\
	{						\
		static_assert(std::is_base_of<Exception, type>::value, desc); \
	}						\

}
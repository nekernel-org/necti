/* -------------------------------------------

	Copyright SoftwareLabs

------------------------------------------- */

#pragma once

#include <algorithm>

namespace stdx
{
	/// @brief allocate a new class.
	/// @tparam KindClass the class type to allocate.
	template <typename KindClass, typename... Args>
	inline void* allocate(Args&&... args)
	{
		return new KindClass(std::forward(args)...);
	}

	/// @brief free a class.
	/// @tparam KindClass the class type to allocate.
	template <typename KindClass>
	inline void release(KindClass ptr)
	{
		if (!ptr)
			return;
		delete ptr;
	}
} // namespace stdx

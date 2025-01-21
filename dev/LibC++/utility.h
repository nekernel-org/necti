/* -------------------------------------------
										 \
 Copyright (C) 2024 Amlal EL Mahrouss, all rights reserved.               \
										 \
------------------------------------------- */

#ifndef LIBCXX_UTILITY_H
#define LIBCXX_UTILITY_H

namespace std
{
	/// @brief Forward object.
	/// @tparam Args the object type.
	/// @param arg the object.
	/// @return object's rvalue
	template <typename Args>
	inline Args&& forward(Args& arg)
	{
		return static_cast<Args&&>(arg);
	}

	/// @brief Move object.
	/// @tparam Args the object type.
	/// @param arg the object.
	/// @return object's rvalue
	template <typename Args>
	inline Args&& move(Args&& arg)
	{
		return static_cast<Args&&>(arg);
	}
} // namespace std

#endif // LIBCXX_UTILITY_H

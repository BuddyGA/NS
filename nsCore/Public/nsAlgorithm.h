#pragma once

#include "nsPlatform.h"



namespace nsAlgorithm
{
	template<typename T>
	static NS_INLINE void Swap(T& first, T& second) noexcept
	{
		T temp = first;
		first = second;
		second = temp;
	}


	template<typename T>
	static NS_INLINE void Sort_Swap(T* data, int first, int second) noexcept
	{
		if (first == second)
		{
			return;
		}

		Swap(data[first], data[second]);
	}


	template<typename T, typename TPredicate>
	static NS_INLINE int Sort_Partition(T* data, int count, int left, int right, TPredicate predicate) noexcept
	{
		NS_ValidateV(left >= 0 && left < count, "Out of range!");
		NS_ValidateV(right >= 0 && right < count, "Out of range!");
		NS_Validate(left < right);

		const T pivot = data[right];
		int i = left;

		for (int j = left; j < right; ++j)
		{
			if (predicate(data[j], pivot))
			{
				Sort_Swap(data, i, j);
				++i;
			}
		}

		Sort_Swap(data, i, right);

		return i;
	}


	template<typename T, typename TPredicate>
	static NS_INLINE void Sort_Internal(T* data, int count, int left, int right, TPredicate predicate) noexcept
	{
		if (left < right)
		{
			int p = Sort_Partition(data, count, left, right, predicate);
			Sort_Internal(data, count, left, p - 1, predicate);
			Sort_Internal(data, count, p + 1, right, predicate);
		}
	}


	template<typename T, typename TPredicate>
	static NS_INLINE void Sort(T* data, int count, TPredicate predicate) noexcept
	{
		if (count == 0 || count == 1)
		{
			return;
		}

		if (count == 2)
		{
			predicate(data[0], data[1]);
			return;
		}

		Sort_Internal(data, count, 0, count - 1, predicate);
	}

};

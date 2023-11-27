#pragma once
#include <cassert>
#include <numeric>

#include "Math/Lerps.hpp"

namespace Utils
{
	template <typename T>
	T Max(const T aFirst, const T aSecond)
	{
		if (aFirst > aSecond)
		{
			return aFirst;
		}

		return aSecond;
	}

	template <typename T>
	T Min(const T aFirst, const T aSecond)
	{
		if (aFirst < aSecond)
		{
			return aFirst;
		}

		return aSecond;
	}

	template <typename T>
	T Abs(const T aValue)
	{
		if (aValue < 0)
		{
			return aValue * -1;
		}

		return aValue;
	}

	template <typename T>
	T Clamp(const T aValue, const T aMin, const T aMax)
	{
		assert(aMin <= aMax && "Min was bigger than max");

		if (aValue < aMin)
		{
			return aMin;
		}

		if (aValue > aMax)
		{
			return aMax;
		}

		return aValue;
	}

	template <typename T>
	void Swap(T& aFirst, T& aSecond)
	{
		T tempFirst = aFirst;
		aFirst = aSecond;
		aSecond = tempFirst;
	}

	template <typename T>
	bool IsAlmostEqual(T aFirstValue, T aSecondValue, T aTolerance)
	{
		return Abs(aSecondValue - aFirstValue) <= aTolerance;
	}

	template <typename T>
	int Sign(T aValue)
	{
		if (aValue < 0)
		{
			return -1;
		}

		if (aValue > 0)
		{
			return 1;
		}

		return 0;
	}

	template<typename T>
	auto Remap(T aValue, T aLow1, T aHigh1, T aLow2, T aHigh2)
	{
		return aLow2 + (aValue - aLow1) * (aHigh2 - aLow2) / (aHigh1 - aLow1);
	}
}
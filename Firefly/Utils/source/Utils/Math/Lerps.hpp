#pragma once
#include <cmath>
#include <string>

namespace Utils
{
	enum class LerpType : int
	{
		Lerp,
		EaseIn, //Power
		EaseOut, //Power
		EaseInOut, //Power
		Bounce,
		Logerp,
		Parabola, //squish
		BounceCustom,
		COUNT
	};

	template <typename T>
	T Lerp(const T a, const T b, float t, bool aShouldClamp = true)
	{
		if (aShouldClamp)
		{
			if (t > 1)
			{
				t = 1;
			}

			if (t < 0)
			{
				t = 0;
			}
		}

		return a + t * (b - a);
	}

	/**
	 * \brief
	 * \param a Start value of the interpolation, this will be the result when t == 0.
	 * \param b End value of the interpolation, this will be the result when t == 1.
	 * \param t Interpolation value.
	 * \param aPower How smooth should the curve be? A value of 1 gives regular interpolation, values above 1 smoothes the curve.
	 * \param aShouldClamp Should the interpolation value t be clamped between 0-1? This can be useful for not wanting to go under the start value or above the end value.
	 * \return Interpolation result
	 */
	template <typename T>
	T EaseIn(const T a, const T b, float t, float aPower = 2.0f, bool aShouldClamp = true)
	{
		if (aShouldClamp)
		{
			if (t > 1)
			{
				t = 1;
			}

			if (t < 0)
			{
				t = 0;
			}
		}

		return Lerp(a, b, powf(t, aPower), false);
	}

	/**
	 * \brief
	 * \param a Start value of the interpolation, this will be the result when t == 0.
	 * \param b End value of the interpolation, this will be the result when t == 1.
	 * \param t Interpolation value.
	 * \param aPower How smooth should the curve be? A value of 1 gives regular interpolation, values above 1 smoothes the curve.
	 * \param aShouldClamp Should the interpolation value t be clamped between 0-1? This can be useful for not wanting to go under the start value or above the end value.
	 * \return Interpolation result
	 */
	template <typename T>
	T EaseOut(const T a, const T b, float t, float aPower = 2.0f, bool aShouldClamp = true)
	{
		if (aShouldClamp)
		{
			if (t > 1)
			{
				t = 1;
			}

			if (t < 0)
			{
				t = 0;
			}
		}

		return Lerp(a, b, 1.0f - powf(1.0f - t, aPower), false);
	}

	/**
	 * \brief
	 * \param a Start value of the interpolation, this will be the result when t == 0.
	 * \param b End value of the interpolation, this will be the result when t == 1.
	 * \param t Interpolation value.
	 * \param aPower How smooth should the curve be? A value of 1 gives regular interpolation, values above 1 smoothes the curve.
	 * \param aShouldClamp Should the interpolation value t be clamped between 0-1? This can be useful for not wanting to go under the start value or above the end value.
	 * \return Interpolation result
	 */
	template <typename T>
	T EaseInOut(const T a, const T b, float t, float aPower = 2.0f, bool aShouldClamp = true)
	{
		if (aShouldClamp)
		{
			if (t > 1)
			{
				t = 1;
			}

			if (t < 0)
			{
				t = 0;
			}
		}

		if (t < 0.5f)
		{
			return Lerp(a, b, powf(2.0f, aPower - 1.0f) * powf(t, aPower), false);
		}

		else
		{
			return Lerp(a, b, 1.0f - powf(-2.0f * t + 2.0f, aPower) / 2.0f, false);
		}
	}

	//Start at a and bounce towards b (3-4 times)
	template <typename T>
	T Bounce(const T a, const T b, float t, bool aShouldClamp = true)
	{
		if (aShouldClamp)
		{
			if (t > 1)
			{
				t = 1;
			}

			if (t < 0)
			{
				t = 0;
			}
		}

		const float nl = 7.5625f;
		const float dl = 2.75f;

		if (t < 1.0f / dl)
		{
			return Lerp(a, b, nl * t * t, false);
		}

		if (t < 2.0f / dl)
		{
			t -= 1.5f / dl;
			return Lerp(a, b, nl * t * t + 0.75f, false);
		}

		if (t < 2.5f / dl)
		{
			t -= 2.25f / dl;
			return Lerp(a, b, nl * t * t + 0.9375f, false);
		}

		t -= 2.625f / dl;
		return Lerp(a, b, nl * t * t + 0.984375f, false);
	}

	template <typename T>
	T Tlerp(const T a, const T b, float t)
	{
		return t < 0.5f ? a : b;
	}

	template <typename T>
	T Logerp(const T a, const T b, float t, bool aShouldClamp = true)
	{
		if (aShouldClamp)
		{
			if (t > 1)
			{
				t = 1;
			}

			if (t < 0)
			{
				t = 0;
			}
		}

		return Lerp(a, b, sqrtf(t), false);
	}

	template <typename T>
	T Parabola(const T a, const T b, float t, float squish = 2.0f, bool aShouldClamp = true)
	{
		if (aShouldClamp)
		{
			if (t > 1)
			{
				t = 1;
			}

			if (t < 0)
			{
				t = 0;
			}
		}

		return Lerp(a, b, powf(4.0f * t * (1.0f - t), squish), false);
	}

	//Start at a and bounce towards b, custom bounces, custom height loss per bounce
	template <typename T>
	T BounceCustom(const T a, const T b, float t, int aNumBounces = 3, float aHeightLoss = 0.5f, bool aShouldClamp = true)
	{
		if (aShouldClamp)
		{
			if (t > 1)
			{
				t = 1;
			}

			if (t < 0)
			{
				t = 0;
			}
		}

		const float ratio = 1.0f / (1 + aNumBounces);
		int bounce = 0;

		for (int i = 1; i <= aNumBounces + 1; ++i)
		{
			if (t <= i * ratio)
			{
				bounce = i;
			}
		}

		const float percentage = bounce * ratio;

		if (bounce == 0)
		{
			return Parabola(a, b, 0.5f + t * percentage);
		}

		if (t <= percentage)
		{
			return Parabola(a, b, (t - percentage) * ((bounce + 2.0f) / (1 + aNumBounces) - percentage)) / (powf(1.0f / aHeightLoss, bounce + 1));
		}
	}

	template <typename T>
	T LerpByType(LerpType aType, const T a, const T b, float t, float aPower, bool aShouldClamp = true, int aBounceCount = 3)
	{
		switch (aType)
		{
		case LerpType::Lerp: return Lerp(a, b, t, aShouldClamp);
		case LerpType::EaseIn: return EaseIn(a, b, t, aPower, aShouldClamp);
		case LerpType::EaseOut: return EaseOut(a, b, t, aPower, aShouldClamp);
		case LerpType::EaseInOut: return EaseInOut(a, b, t, aPower, aShouldClamp);
		case LerpType::Bounce: return Bounce(a, b, t, aShouldClamp);
		case LerpType::Logerp: return Logerp(a, b, t, aShouldClamp);
		case LerpType::Parabola: return Parabola(a, b, t, aPower, aShouldClamp);
		case LerpType::BounceCustom: return BounceCustom(a, b, t, aBounceCount, aPower);
		default: return b;
		}
	}
}

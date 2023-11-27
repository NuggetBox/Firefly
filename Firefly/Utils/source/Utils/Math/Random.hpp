#pragma once
#include <cstdlib>
#include <ctime>
#include <random>

#include "MathDefines.h"
#include "Vector.h"

namespace Utils
{
	//Call once when program starts to randomize std::rand() random generation
	static void SeedRandom()
	{
		srand(static_cast<unsigned>(time(NULL)));
	}

	static float RandomFloat(float aLowerLimit = 0.0f, float aUpperLimit = 1.0f)
	{
		if (aLowerLimit >= aUpperLimit)
		{
			return aLowerLimit;
		}

		return aLowerLimit + static_cast<float>(std::rand()) / (RAND_MAX / (aUpperLimit - aLowerLimit));
	}

	static double RandomDouble(double aLowerLimit = 0.0, double aUpperLimit = 1.0)
	{
		if (aLowerLimit >= aUpperLimit)
		{
			return aLowerLimit;
		}

		return aLowerLimit + static_cast<double>(std::rand()) / (RAND_MAX / (aUpperLimit - aLowerLimit));
	}

	//Returns a random integer between lower & upper limit. Lower limit inclusive, Upper limit exclusive
	static int RandomInt(int aLowerLimitInclusive = 0, int aUpperLimitExclusive = 10)
	{
		if (aLowerLimitInclusive >= aUpperLimitExclusive)
		{
			return aLowerLimitInclusive;
		}

		return aLowerLimitInclusive + std::rand() % abs(aUpperLimitExclusive - aLowerLimitInclusive);
	}

	static bool RandomBool(int aFalseChances = 50, int aTrueChances = 50)
	{
		if (aFalseChances == aTrueChances)
		{
			return RandomInt(0, 2);
		}

		return RandomInt(0, aFalseChances + aTrueChances) >= aFalseChances;
	}

	/**
	 * \brief  Returns a random angle in radians, 0-2PI
	 */
	static float RandomAngle()
	{
		return RandomFloat() * 2 * PI;
	}

	/**
	 * \brief Returns a random angle in degrees, no decimals, 0-360
	 */
	static int RandomAngleWholeDegrees()
	{
		return std::rand() % 360;
	}

	/**
	 * \brief Returns a random angle in degrees with decimals, 0-360
	 */
	static float RandomAngleDegrees()
	{
		return RandomFloat() * 360.0f;
	}

	static Vector2f RandomPointInCircle(float aRadius)
	{
		float randomDistance = aRadius * sqrt(RandomFloat());
		float randomAngle = RandomAngle();
		return Vector2f(cos(randomAngle) * randomDistance, sin(randomAngle) * randomDistance);
	}

	static Vector2f RandomPointOnCircleEdge(float aRadius)
	{
		float randomAngle = RandomAngle();
		return Vector2f(cos(randomAngle) * aRadius, sin(randomAngle) * aRadius);
	}

	static Vector3f RandomPointInSphere(float aRadius)
	{
		float x, y, z, length;

		do
		{
			x = RandomFloat(-1.0f, 1.0f);
			y = RandomFloat(-1.0f, 1.0f);
			z = RandomFloat(-1.0f, 1.0f);
			length = sqrt(x * x + y * y + z * z);
		} while (length > 1);

		return Vector3f(x * aRadius, y * aRadius, z * aRadius);
	}

	static Vector3f RandomPointOnSphereSurface(float aRadius)
	{
		Vector3f pointInSphere = RandomPointInSphere(1);
		return pointInSphere.GetNormalized() * aRadius;
	}

	static Vector2f RandomPointInRectangle(float aWidth, float aHeight)
	{
		return Vector2f(RandomFloat(0.0f, aWidth), RandomFloat(0.0f, aHeight));
	}

	static Vector2f RandomPointOnRectangleEdge(float aWidth, float aHeight)
	{
		const float widthX2 = 2 * aWidth;
		const float heightX2 = 2 * aHeight;
		const float randomLinearValue = RandomFloat(0.0f, widthX2 + heightX2);

		//Top side
		if (randomLinearValue < aWidth)
		{
			return Vector2f(randomLinearValue, aHeight);
		}

		//Bot side
		if (randomLinearValue < widthX2)
		{
			return Vector2f(randomLinearValue - aWidth, 0.0f);
		}

		//Left side
		if (randomLinearValue < widthX2 + aHeight)
		{
			return Vector2f(0.0f, randomLinearValue - widthX2);
		}

		//Right side
		return Vector2f(aWidth, randomLinearValue - (widthX2 + aHeight));
	}

	static Vector2f RandomPointInSquare(float aSideLength)
	{
		return RandomPointInRectangle(aSideLength, aSideLength);
	}

	static Vector2f RandomPointOnSquareEdge(float aSideLength)
	{
		return RandomPointOnRectangleEdge(aSideLength, aSideLength);
	}
}
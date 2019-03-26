#include "utility/Random.h"

namespace Utopian
{
	float GetRandomFloat(float min, float max)
	{
		if (min > max)
		{
			float tmp = min;
			min = max;
			max = tmp;
		}

		// Todo: Add Random helper class
		std::random_device rd;
		std::mt19937 mt(rd());
		std::uniform_real_distribution<float> random(min, max);

		return random(mt);
	}
}
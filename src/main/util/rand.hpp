#pragma once

#include <random>

namespace util
{

	/* Random Devices */

	std::random_device rd;

	/* Engines */

	std::mt19937 e2(rd());

	/* Functions */

	/**
	* Generate a random integer within the interval [min, max].
	* @param min The lower bound
	* @param max The upper bound
	* @returns   The generated number
	*/
	int rand_between(int min, int max)
	{
		std::uniform_int_distribution<> dist(min, max);
		return dist(e2);
	}

	/**
	* Generate a random floating point number within the interval [min, max].
	* @param min The lower bound
	* @param max The upper bound
	* @returns   The generated number
	*/
	float rand_between(float min, float max)
	{
		std::uniform_real_distribution<> dist(min, max);
		return dist(e2);
	}

}
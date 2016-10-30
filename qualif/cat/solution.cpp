#include "solution.hpp"

#include <iostream>

std::vector<size_t> FindRadioactiveBalls(size_t NumberOfBalls,
		size_t RadioActiveBalls,
		bool (*TestFunction)(const std::vector<size_t>& BallsToTest)) {

	(void)RadioActiveBalls;

	std::vector<size_t> result;

	for (size_t ball = 0; ball < NumberOfBalls; ++ball) {
		if (TestFunction({ball})) {
			result.push_back(ball);
		}
	}

	return result;
}

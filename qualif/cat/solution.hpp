#ifndef CAT_SOLUTION_HPP_
#define CAT_SOLUTION_HPP_

#include <stddef.h> // These idiots are using size_t instead of std::size_t.

#include <vector>

std::vector<size_t> FindRadioactiveBalls(size_t NumberOfBalls,
		size_t RadioActiveBalls,
		bool (*TestFunction)(const std::vector<size_t>& BallsToTest));

#endif // CAT_SOLUTION_HPP_

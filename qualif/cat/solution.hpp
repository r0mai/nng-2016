#ifndef CAT_SOLUTION_HPP_
#define CAT_SOLUTION_HPP_

#include <stddef.h> // These idiots are using size_t instead of std::size_t.

#include <algorithm>
#include <iostream>
#include <string>
#include <tuple>
#include <vector>

std::vector<size_t> FindRadioactiveBalls(size_t NumberOfBalls,
		size_t RadioActiveBalls,
		bool (*TestFunction)(const std::vector<size_t>& BallsToTest));

std::string printSolution(const std::vector<size_t>& solution) {
	return '{' +std::accumulate(solution.begin(), solution.end(),
			std::string{},
			[](std::string output, size_t element) {
				if (!output.empty()) {
					output += ", ";
				}
				output += std::to_string(element);
				return output;
			}) + '}';
}

class Tester {
	static std::vector<bool> currentBalls;
	static std::size_t checkCounter;
	static bool checker(const std::vector<size_t>& ballIndices) {
		++checkCounter;
		for (size_t index: ballIndices) {
			if (currentBalls[index]) {
				return true;
			}
		}
		return false;
	}

	std::vector<size_t> getIndices() const {
		std::vector<size_t> result;
		for(std::size_t index = 0; index < currentBalls.size(); ++index) {
			if(currentBalls[index]) {
				result.push_back(index);
			}
		}
		return result;
	}

public:
	void setBalls(std::vector<bool> balls) {
		currentBalls = std::move(balls);
	}

	std::tuple<std::size_t, std::size_t, std::size_t> verify() {
		checkCounter = 0;
		auto expectedResult = getIndices();
		auto result = FindRadioactiveBalls(currentBalls.size(),
				expectedResult.size(),
				&checker);

		if (expectedResult != result) {
			std::cerr << "Implementation not correct: " << std::endl;
			std::cerr << printSolution(result) << std::endl;
			std::cerr << "!=" << std::endl;
			std::cerr << printSolution(expectedResult) << std::endl;
			std::exit(1);
		}
		std::cerr << "Sample size was: " << currentBalls.size()
				<< ", verified in " << checkCounter << " measurements"
				<< std::endl;
		return std::make_tuple(
				currentBalls.size(), expectedResult.size(), checkCounter);
	}

};
#endif // CAT_SOLUTION_HPP_

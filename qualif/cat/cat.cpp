#include <iostream>
#include <vector>

#include <algorithm>

#include <boost/range/join.hpp>

#include "solution.hpp"


// Global, as NNG hasn't heard of std::function.
std::vector<bool> balls = {true, true, false, false};

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


bool checker(const std::vector<size_t>& ballIndices) {
	for (size_t index: ballIndices) {
		if (balls[index]) {
			return true;
		}
	}
	return false;
}

int main() {

	std::vector<size_t> expectedResult = {0, 1};
	auto result = FindRadioactiveBalls(balls.size(), expectedResult.size(),
			&checker);

	if (expectedResult != result) {
		std::cerr << "Implementation not correct" << std::endl;
		std::cerr << printSolution(result) << std::endl;
		std::exit(1);
	}
}

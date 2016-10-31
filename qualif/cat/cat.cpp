#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <vector>


#include "solution.hpp"

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

	void verify() {
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
	}

};

std::vector<bool> Tester::currentBalls;
std::size_t Tester::checkCounter;

std::vector<bool> generateRandomSample(std::size_t length) {
	static std::random_device rd;
	static std::mt19937 gen{rd()};
	std::bernoulli_distribution coinToss{0.5};
	std::vector<bool> result;
	std::generate_n(std::back_inserter(result), length, std::bind(coinToss, gen));
	return result;
}

int main() {

	Tester tester;

	for(std::size_t length = 1; length < 100; ++length) {
		std::cerr << "Running test against length of " << length << std::endl;
		tester.setBalls(generateRandomSample(length));
		tester.verify();
	}
	std::cerr << "Verification passed" << std::endl;
}

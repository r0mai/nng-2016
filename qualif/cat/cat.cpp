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

std::vector<bool> Tester::currentBalls;
std::size_t Tester::checkCounter;

std::vector<bool> generateRandomSample(std::size_t length,
		std::size_t numberOfRadioactiveBalls) {
	static std::random_device rd;
	static std::mt19937 gen{rd()};
	std::vector<bool> result;
	for(std::size_t i = 0; i < length; ++i) {
		result.push_back(false);
	}
	for(std::size_t i = 0; i < numberOfRadioactiveBalls; ++i) {
		result[i] = true;
	}

	std::shuffle(result.begin(), result.end(), gen);
	return result;
}

int main() {

	Tester tester;

	for(std::size_t length = 64; length < 65; ++length) {
		std::cerr << "Running test against length of " << length << std::endl;
		for(std::size_t radioActivity = 0; radioActivity <= 7;
				++radioActivity) {
			std::vector<std::size_t> tests;
			std::cerr << "Radioactivity: " << radioActivity << std::endl;
			for(std::size_t i = 0; i < 100; ++i) {
				tester.setBalls(generateRandomSample(length, radioActivity));
				auto result = tester.verify();
				tests.push_back(std::get<2>(result));
			}
			auto best = *std::min_element(tests.begin(), tests.end());
			auto worst = *std::max_element(tests.begin(), tests.end());
			auto sum = std::accumulate(tests.begin(), tests.end(), 0);
			std::cout << radioActivity << " " << float(sum)/tests.size() << " "
					<< best << " " << worst << std::endl;
		}
	}
	std::cerr << "Verification passed" << std::endl;
}

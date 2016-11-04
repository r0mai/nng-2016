#include <algorithm>
#include <functional>
#include <iostream>
#include <random>
#include <vector>

#include <boost/assert.hpp>

#include "solution.hpp"


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

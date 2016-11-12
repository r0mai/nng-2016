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

class Measurement {
public:
	size_t getBest() const {
		return *std::min_element(measurements.begin(), measurements.end());
	}
	size_t getWorst() const {
		return *std::max_element(measurements.begin(), measurements.end());
	}

	float getAverage() const {
		return float(
				std::accumulate(measurements.begin(), measurements.end(), 0))
				/ float(measurements.size());
	}

	void addResult(size_t value) {
		measurements.push_back(value);
	}
private:
	std::vector<size_t> measurements;
};

int main() {

	Tester tester;

	std::cout << "Total";
	for(std::size_t radioactivity = 0; radioactivity <= 7; ++radioactivity) {
		std::cout << "\t" << radioactivity;
	}
	std::cout << std::endl;
	for (std::size_t length = 0; length <= 64; ++length) {
		std::cout << length << "\t";
		for (std::size_t radioactivity = 0;
				radioactivity <= std::min(length, 7ul); ++radioactivity) {
			Measurement m;
			for (std::size_t sample = 0; sample < 100; ++sample) {
				auto balls = generateRandomSample(length, radioactivity);
				tester.setBalls(balls);
				auto count = std::get<2>(tester.verify());
				m.addResult(count);
			}
			std::cout << m.getWorst() << "\t";
		}
		std::cout << std::endl;
	}

	std::cerr << "Verification passed" << std::endl;
}

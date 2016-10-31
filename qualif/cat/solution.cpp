#include "solution.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>

#include <boost/assert.hpp>

std::vector<size_t> selectFrom(const std::vector<size_t>& from) {
	static std::random_device rd;
	static std::mt19937 gen{rd()};

	BOOST_ASSERT_MSG(!from.empty(), "Empty index vector passed to selection");
	double scaling = 0.5;
	std::bernoulli_distribution distribution{scaling};
	auto select = std::bind(distribution, gen);
	std::vector<size_t> result;
	std::copy_if(from.begin(), from.end(), std::back_inserter(result),
			select);
	if (result.empty()) {
		result.push_back(from.front());
	}
	return result;
}

std::vector<size_t> removePartition(std::vector<size_t> from,
		std::vector<size_t> what) {
	std::sort(from.begin(), from.end());
	std::sort(what.begin(), what.end());

	std::vector<size_t> result;
	std::set_difference(from.begin(), from.end(), what.begin(), what.end(),
			std::back_inserter(result));
	return result;
}

std::vector<size_t> FindRadioactiveBalls(size_t NumberOfBalls,
		size_t RadioActiveBalls,
		bool (*TestFunction)(const std::vector<size_t>& BallsToTest)) {

	(void)RadioActiveBalls;

	std::vector<size_t> result;

	std::vector<size_t> indices;
	for (size_t i = 0; i < NumberOfBalls; ++i) {
		indices.push_back(i);
	}

	BOOST_ASSERT(removePartition({}, {}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({0}, {0}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({1, 0}, {1, 0}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({1, 0}, {0}) == std::vector<size_t>{1});

	auto partition = selectFrom(indices);
	while(!indices.empty() || result.size() != RadioActiveBalls) {

		bool testResult = TestFunction(partition);

		if (!testResult) {
			// Whole partition is clean
			indices = removePartition(indices, partition);
			if (!indices.empty()) {
				partition = selectFrom(indices);
			}
		} else {
				if (testResult && partition.size() == 1) {
				// All of partition is defective
				std::copy(partition.begin(), partition.end(),
						std::back_inserter(result));
				indices = removePartition(indices, partition);
				if (!indices.empty()) {
					partition = selectFrom(indices);
				}
			} else {
				partition = selectFrom(partition);
			}
		}
	}

	std::sort(result.begin(), result.end());
	return result;
}

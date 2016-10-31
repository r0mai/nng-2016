#include "solution.hpp"

#include <algorithm>
#include <iostream>
#include <random>

#include <boost/assert.hpp>

std::vector<size_t> firstHalf(std::vector<size_t> elements) {

	auto desiredSize = std::max(
			0ul,
			std::min(elements.size(), (elements.size()+1)/2));

	elements.resize(desiredSize);
	return elements;
}

std::vector<size_t> firstSegment(std::vector<size_t> elements, size_t n) {
	for(size_t i=1; i<n; ++i) {
		elements = firstHalf(elements);
	}
	return elements;
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

	BOOST_ASSERT(firstHalf({}).empty());
	BOOST_ASSERT(firstHalf({0}).size() == 1);
	BOOST_ASSERT(firstHalf({0, 0}).size() == 1);
	BOOST_ASSERT(firstHalf({0, 0, 0}).size() == 2);
	BOOST_ASSERT(firstHalf({0, 0, 0, 0}).size() == 2);
	BOOST_ASSERT(firstHalf({0, 0, 0, 0, 0}).size() == 3);
	BOOST_ASSERT(firstHalf({0, 0, 0, 0, 0}).size() == 3);

	BOOST_ASSERT(removePartition({}, {}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({0}, {0}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({1, 0}, {1, 0}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({1, 0}, {0}) == std::vector<size_t>{1});

	size_t iteration = 1;
	while(!indices.empty() || result.size() != RadioActiveBalls) {

		auto partition = firstSegment(indices, iteration);
		bool testResult = TestFunction(partition);

		if (!testResult) {
			// Whole partition is clean
			indices = removePartition(indices, partition);
		} else if (testResult && partition.size() == 1) {
			// All of partition is defective
			std::copy(partition.begin(), partition.end(),
					std::back_inserter(result));
			indices = removePartition(indices, partition);
		} else {
			++iteration;
		}

	}

	return result;
}

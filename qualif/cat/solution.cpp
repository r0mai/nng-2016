#include "solution.hpp"

#include <algorithm>
#include <functional>
#include <iostream>
#include <random>

#include <boost/assert.hpp>
#include <boost/optional.hpp>

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
	if (result.size() == from.size()) {
		BOOST_ASSERT_MSG(from.size() > 1, "Attempting infinite recursion");
		result.pop_back();
	}
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

std::vector<size_t> add(
		const std::vector<size_t>& left, const std::vector<size_t>& right) {
	std::vector<size_t> result;
	std::set_union(left.begin(), left.end(), right.begin(), right.end(),
			std::back_inserter(result));
	return result;
}

std::vector<size_t> findRadioactivity(const std::vector<size_t>& balls,
		boost::optional<size_t> radioActiveBalls,
		const std::function<bool(const std::vector<size_t>&)>& testFunction) {
	if (radioActiveBalls && *radioActiveBalls == balls.size()) {
		return balls;
	}
	if (balls.size() == 1) {
		// Nothing we can really optimize here
		if (testFunction(balls)) {
			return balls;
		} else {
			return {};
		}
	}
	if (radioActiveBalls && *radioActiveBalls == 0) {
		return {};
	}
	if (radioActiveBalls && *radioActiveBalls == 1) {
		auto partition = selectFrom(balls);
		auto others = removePartition(balls, partition);
		bool testResult = testFunction(partition);
		if (!testResult) {
			// Partition is clean, one defective in other
			return findRadioactivity(others, 1, testFunction);
		} else {
			return findRadioactivity(partition, 1, testFunction);
		}
	}
	if (radioActiveBalls && *radioActiveBalls == 2) {
		auto partition = selectFrom(balls);
		auto others = removePartition(balls, partition);
		bool leftResult = testFunction(partition);
		bool rightResult = testFunction(others);
		if (leftResult && rightResult) {
			// Both have one each
			std::vector<size_t> lefts = findRadioactivity(partition,
					1, testFunction);
			std::vector<size_t> rights = findRadioactivity(others,
					1, testFunction);
			return add(lefts, rights);
		}
		if (leftResult) {
			return findRadioactivity(partition, 2, testFunction);
		}
		if (rightResult) {
			return findRadioactivity(others, 2, testFunction);
		}
	}

	auto left = selectFrom(balls);
	auto right = removePartition(balls, left);

	// left traversal has less information, make sure it is smaller
	if (left.size() > right.size() ) {
		using std::swap;
		swap(left, right);
	}

	auto leftResult = testFunction(left);
	auto rightResult = testFunction(right);

	std::vector<size_t> lefts;
	std::vector<size_t> rights;
	if (leftResult) {
		lefts = findRadioactivity(left,
				rightResult ? boost::none : radioActiveBalls, testFunction);
	}
	if (rightResult) {
		rights = findRadioactivity(right,
				radioActiveBalls ? 	boost::optional<size_t>{
						*radioActiveBalls - lefts.size()} : boost::none,
				testFunction);
	}
	return add(lefts, rights);
}

std::vector<size_t> linearStart(const std::vector<size_t>& balls,
		size_t radioActiveBalls,
		const std::function<bool(const std::vector<size_t>&)>& testFunction) {
	double cutOff = 0.1;
	std::vector<size_t> result;
	std::vector<size_t> checked;
	for (const auto& ball: balls) {
		auto remainingBallCount = balls.size() - checked.size();
		auto remainingRadioactive = radioActiveBalls - result.size();
		auto ratio = float(remainingRadioactive) / float(remainingBallCount);

		if (result.size() == radioActiveBalls) {
			return result;
		}

		if (ratio <= cutOff) {
			std::cerr << "Switching to logarithmic after " << checked.size()
					<< " with " << remainingRadioactive << " left to find"
					<< std::endl;
			auto remaining = findRadioactivity(removePartition(balls, checked),
					remainingRadioactive, testFunction);
			return add(result, remaining);
		} else {
			checked.push_back(ball);
			if (testFunction({ball})) {
				result.push_back(ball);
			}
		}
	}
	return result;
}

std::vector<size_t> FindRadioactiveBalls(size_t NumberOfBalls,
		size_t RadioActiveBalls,
		bool (*TestFunction)(const std::vector<size_t>& BallsToTest)) {

	static std::random_device rd;
	static std::mt19937 gen{rd()};
	std::vector<size_t> indices;
	for (size_t i = 0; i < NumberOfBalls; ++i) {
		indices.push_back(i);
	}
	std::shuffle(indices.begin(), indices.end(), gen);

	BOOST_ASSERT(removePartition({}, {}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({0}, {0}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({1, 0}, {1, 0}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({1, 0}, {0}) == std::vector<size_t>{1});
	auto result = linearStart(indices, RadioActiveBalls, TestFunction);
	std::sort(result.begin(), result.end());
	return result;
}

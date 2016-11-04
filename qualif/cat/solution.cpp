#include <algorithm>
#include <functional>
#include <iostream>
#include <random>

#include <stddef.h> // These idiots are using size_t instead of std::size_t.

#include <boost/assert.hpp>
#include <boost/optional.hpp>

std::vector<size_t> selectFrom(const std::vector<size_t>& from) {
	static std::random_device rd;
	static std::mt19937 gen{rd()};

	auto result = from;
	std::shuffle(result.begin(), result.end(), gen);

	BOOST_ASSERT_MSG(from.size() > 1, "What would be the point of that then?");

	result.resize(from.size() / 2);

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
std::vector<size_t> findRadioactivity1(const std::vector<size_t>& balls,
		const std::function<bool(const std::vector<size_t>&)>& testFunction) {
	if (balls.size() == 1) {
		return balls;
	}
	auto partition = selectFrom(balls);
	auto others = removePartition(balls, partition);
	bool testResult = testFunction(partition);
	if (!testResult) {
		// Partition is clean, one defective in other
		return findRadioactivity1(others, testFunction);
	} else {
		return findRadioactivity1(partition, testFunction);
	}
}
std::vector<size_t> findRadioactivity2(const std::vector<size_t>& balls,
		const std::function<bool(const std::vector<size_t>&)>& testFunction) {

	BOOST_ASSERT_MSG(balls.size() >=2, "Less than two can't have two balls");

	if (balls.size() == 2) {
		return balls;
	}

	if (balls.size() == 3) {
		bool first = testFunction({balls[0]});
		if (!first) {
			return {balls[1], balls[2]};
		} else {
			bool second = testFunction({balls[1]});
			if (!second) {
				return {balls[0], balls[2]};
			}
			return {balls[0], balls[1]};
		}
	}

	if (balls.size() == 4) {
		bool first = testFunction({balls[0]});
		if (first) {
			return add({balls[0]}, findRadioactivity1(removePartition(balls,
					{balls[0]}), testFunction));
		}
		return findRadioactivity2(removePartition(balls, {balls[0]}),
				testFunction);
	}

	auto left = selectFrom(balls);
	auto right = removePartition(balls, left);
	if (left.size() > right.size()) {
		using std::swap;
		swap(left, right);
	}
	bool leftResult = testFunction(left);
	bool rightResult = !leftResult || testFunction(right);
	if (leftResult && rightResult) {
		// Both have one each
		std::vector<size_t> lefts = findRadioactivity1(left, testFunction);
		std::vector<size_t> rights = findRadioactivity1(right, testFunction);
		return add(lefts, rights);
	}
	if (leftResult) {
		return findRadioactivity2(left, testFunction);
	}
	if (rightResult) {
		return findRadioactivity2(right, testFunction);
	}
	BOOST_ASSERT_MSG(false, "Searching for two, but neither had it");
	return {};
}

std::vector<size_t> findRadioactivity(const std::vector<size_t>& balls,
		boost::optional<size_t> radioActiveBalls,
		const std::function<bool(const std::vector<size_t>&)>& testFunction) {
	if (radioActiveBalls && *radioActiveBalls == 0) {
		return {};
	}
	if (radioActiveBalls && *radioActiveBalls == balls.size()) {
		return balls;
	}
	if (balls.size() == 1) {
		if (testFunction(balls)) {
			return balls;
		} else {
			return {};
		}
	}
	if (radioActiveBalls && *radioActiveBalls == 1) {
		return findRadioactivity1(balls, testFunction);
	}
	if (radioActiveBalls && *radioActiveBalls == 2) {
		return findRadioactivity2(balls, testFunction);
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
	auto result = findRadioactivity(indices, RadioActiveBalls, TestFunction);
	std::sort(result.begin(), result.end());
	return result;
}

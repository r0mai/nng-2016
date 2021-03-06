#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <random>

#include <stddef.h> // These idiots are using size_t instead of std::size_t.

#include <boost/assert.hpp>
#include <boost/optional.hpp>

std::vector<size_t> selectFrom(const std::vector<size_t>& from) {
	static std::mt19937 gen{3};

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
std::pair<std::vector<size_t>, std::vector<size_t>>
findRadioactivity1(std::vector<size_t> balls,
		const std::function<bool(const std::vector<size_t>&)>& testFunction) {
	BOOST_ASSERT_MSG(!balls.empty(), "Can't find one in empty");
	if (balls.size() == 1) {
		return std::make_pair(balls, std::vector<size_t>{});
	}
	auto partition = selectFrom(balls);
	auto others = removePartition(balls, partition);
	bool testResult = testFunction(partition);
	if (!testResult) {
		// Partition is clean, one defective in other
		std::vector<size_t> radioActive;
		std::vector<size_t> knownGood;
		std::tie(radioActive, knownGood) =
				findRadioactivity1(others, testFunction);
		return std::make_pair(radioActive, add(partition, knownGood));
	} else {
		std::vector<size_t> radioActive;
		std::vector<size_t> knownGood;
		std::tie(radioActive, knownGood) =
				findRadioactivity1(partition, testFunction);
		// don't add others, we don't know if there wasn't a defect there
		return std::make_pair(radioActive, knownGood);
	}
}
std::pair<std::vector<size_t>, std::vector<size_t>>
findRadioactivity2(const std::vector<size_t>& balls,
		const std::function<bool(const std::vector<size_t>&)>& testFunction) {

	BOOST_ASSERT_MSG(balls.size() >=2, "Less than two can't have two balls");

	if (balls.size() == 2) {
		return std::make_pair(balls, std::vector<size_t>{});
	}

	if (balls.size() == 3) {
		bool first = testFunction({balls[0]});
		if (!first) {
			return std::make_pair(std::vector<size_t>({balls[1], balls[2]}),
					std::vector<size_t>({balls[0]}));
		} else {
			bool second = testFunction({balls[1]});
			if (!second) {
				return std::make_pair(std::vector<size_t>({balls[0], balls[2]}),
						std::vector<size_t>({balls[1]}));
			}
			return std::make_pair(std::vector<size_t>({balls[0], balls[1]}),
					std::vector<size_t>({balls[2]}));
		}
	}

	if (balls.size() == 4) {
		bool first = testFunction({balls[0]});
		if (first) {
			std::vector<size_t> radioActive;
			std::vector<size_t> knownGood;
			std::tie(radioActive, knownGood) =
					findRadioactivity1(removePartition(balls, {balls[0]}),
							testFunction);
			return std::make_pair(add(radioActive, {balls[0]}), knownGood);
		}
		std::vector<size_t> radioActive;
		std::vector<size_t> knownGood;
		std::tie(radioActive, knownGood) =
				findRadioactivity2(removePartition(balls, {balls[0]}),
				testFunction);
		knownGood.push_back(balls[0]);
		return std::make_pair(radioActive, knownGood);
	}

	auto left = selectFrom(balls);
	auto right = removePartition(balls, left);
	bool leftResult = testFunction(left);
	bool rightResult = !leftResult || testFunction(right);
	if (leftResult && rightResult) {
		// Both have one each
		std::vector<size_t> leftRadio;
		std::vector<size_t> leftGood;
		std::tie(leftRadio, leftGood) = findRadioactivity1(left, testFunction);
		std::vector<size_t> rightRadio;
		std::vector<size_t> rightGood;
		std::tie(rightRadio, rightGood) = findRadioactivity1(right, testFunction);
		return std::make_pair(add(leftRadio, rightRadio),
				add(leftGood, rightGood));
	}
	if (leftResult) {
		std::vector<size_t> radioActive;
		std::vector<size_t> knownGood;
		std::tie(radioActive, knownGood) =
				findRadioactivity2(left, testFunction);
		return std::make_pair(radioActive, add(knownGood, right));
	}
	if (rightResult) {
		std::vector<size_t> radioActive;
		std::vector<size_t> knownGood;
		std::tie(radioActive, knownGood) =
				findRadioactivity2(right, testFunction);
		return std::make_pair(radioActive, add(knownGood, left));
	}
	BOOST_ASSERT_MSG(false, "Searching for two, but neither had it");
	return {};
}

struct Linear {
	bool applicable(size_t, size_t) {
		return true;
	}

	std::vector<size_t> apply(const std::vector<size_t>& balls,
			size_t d,
			std::function<bool(const std::vector<size_t>&)> testFunction) {
		std::vector<size_t> result;
		for (size_t ballIndex = 0; ballIndex < balls.size(); ++ballIndex) {
			if (result.size() == d) {
				break;
			}
			if (d - result.size() == (balls.size() - ballIndex)) {
				std::copy(balls.begin() + ballIndex, balls.end(),
						std::back_inserter(result));
				break;
			}
			const auto& ball = balls[ballIndex];
			if (testFunction({ball})) {
				result.push_back(ball);
			}
		}
		return result;
	}
};

struct BinarySearch {
	bool applicable(size_t, size_t d) {
		return d >= 1;
	}

	std::vector<size_t> apply(std::vector<size_t> balls,
			size_t d,
			std::function<bool(const std::vector<size_t>&)> testFunction) {
		std::vector<size_t> result;
		while(result.size() < d) {
			if (balls.size() == d - result.size()) {
				std::copy(balls.begin(), balls.end(),
						std::back_inserter(result));
				break;
			}
			std::vector<size_t> radioActive;
			std::vector<size_t> knownGood;
			std::tie(radioActive, knownGood) =
					findRadioactivity1(balls, testFunction);
			auto match = radioActive.front();
			balls.erase(std::find(balls.begin(), balls.end(), match));
			result.push_back(match);
			for(const auto& good: knownGood) {
				balls.erase(std::find(balls.begin(), balls.end(), good));
			}
		}
		std::sort(result.begin(), result.end());
		return result;
	}
};

struct AllRadioactiveOptimization {
	bool applicable(size_t n, size_t d) {
		return n == d;
	}

	std::vector<size_t> apply(std::vector<size_t> balls,
			size_t d,
			std::function<bool(const std::vector<size_t>&)>) {
		(void)d;
		BOOST_ASSERT_MSG(d == balls.size(), "Optimization not applicable");
		return balls;
	}
};

struct SpecialCaseFor2 {
	bool applicable(size_t n, size_t d) {
		return d == 2 && n >= d;
	}
	std::vector<size_t> apply(std::vector<size_t> balls,
			size_t d,
			std::function<bool(const std::vector<size_t>&)> testFunction) {
		(void)d;
		BOOST_ASSERT_MSG(d == 2, "Optimization not applicable");
		std::vector<size_t> radioActive;
		std::vector<size_t> knownGood;
		std::tie(radioActive, knownGood) =
				findRadioactivity2(balls, testFunction);
		return radioActive;
	}
};

template<typename... Approaches>
class Launchpad;

template<typename Approach>
class Launchpad<Approach> {
public:
	bool applicable(size_t n, size_t d) {
		return approach.applicable(n, d);
	}

	std::vector<size_t> apply(const std::vector<size_t>& balls,
			std::size_t d,
			std::function<bool(const std::vector<size_t>&)> tester) {
		return approach.apply(balls, d, tester);
	}
private:
	Approach approach;
};

template<typename Approach, typename... Approaches>
class Launchpad<Approach, Approaches...> {
public:
	bool applicable(size_t n, size_t d) {
		return approach.applicable(n, d) || others.applicable(n, d);
	}

	std::vector<size_t> apply(const std::vector<size_t>& balls,
			std::size_t d,
			std::function<bool(const std::vector<size_t>&)> tester) {
		auto it = std::find(useFirst.begin(), useFirst.end(),
				std::make_pair(balls.size(), d));
		if (it != useFirst.end()) {
			// Approach is best
			return approach.apply(balls, d, tester);
		}
		return others.apply(balls, d, tester);
	}

	Launchpad() {
		for (size_t length = 0; length <= 64; ++length) {
			for(size_t d = 0; d <= std::min(length, 7ul); ++d) {
				if (!others.applicable(length, d)) {
					// No point in checking performance of other
					useFirst.push_back(std::make_pair(length, d));
					continue;
				}
				if (!approach.applicable(length, d)) {
					// We aren't applicable, so don't use us.
					continue;
				}
				size_t betterCount = 0;
				size_t iterations = 20;
				for(size_t iteration = 0; iteration < iterations; ++iteration) {
					if (isOursBetter(length, d)) {
						++betterCount;
					}
				}
				if (betterCount > iterations / 2) {
					useFirst.push_back(std::make_pair(length, d));
				}
			}
		}
	}

private:
	bool isOursBetter(size_t length, size_t d) {
		auto testSample = getSample(length, d);
		size_t checkCount = 0;
		auto testFunction = [&checkCount, &testSample](
				const std::vector<size_t>& balls) mutable {
			++checkCount;
			for (const auto& ball: balls) {
				if (testSample[ball]) {
					return true;
				}
			}
			return false;
		};
		std::vector<size_t> balls;
		for (size_t i = 0; i < length; ++i) {
			balls.push_back(i);
		}

		auto leftResult = approach.apply(balls, d, testFunction);
		size_t left = checkCount;
		checkCount = 0;
		auto rightResult = others.apply(balls, d, testFunction);
		size_t right = checkCount;
		if (length == d) {
			BOOST_ASSERT(0 == std::min(left, right));
		}
		if (leftResult == rightResult) {
			return left < right;
		}
		return true;
	}

	std::vector<bool> getSample(size_t length, size_t d) {
		std::vector<bool> result;
		result.resize(length);
		for(size_t i=0; i<d; ++i) {
			result[i] = true;
		}
		std::shuffle(result.begin(), result.end(), gen);
		BOOST_ASSERT(std::count(result.begin(), result.end(), true) == d);
		return result;
	}

	std::mt19937 gen{3};
	std::vector<std::pair<size_t, size_t>> useFirst;
	Approach approach;
	Launchpad<Approaches...> others;
};

class MemoizingTester {
public:
	MemoizingTester(std::function<bool(const std::vector<size_t>&)> tester) :
		tester(std::move(tester)),
		knownToBeGood(std::make_shared<std::vector<size_t>>()) {
	}

	bool operator()(const std::vector<size_t>& balls) const {
		bool areAllKnownToBeGood = true;

		for(const auto& ball: balls) {
			bool isKnownToBeGood = std::find(knownToBeGood->begin(),
					knownToBeGood->end(), ball) != knownToBeGood->end();
			if (!isKnownToBeGood) {
				areAllKnownToBeGood = false;
				break;
			}
		}

		if (areAllKnownToBeGood) {
			BOOST_ASSERT(false);
			return false;
		}
		bool result = tester(balls);
		if (!result) {
			std::copy(balls.begin(), balls.end(),
					std::back_inserter(*knownToBeGood));
		}
		return result;
	}
private:
	std::function<bool(const std::vector<size_t>&)> tester;
	std::shared_ptr<std::vector<size_t>> knownToBeGood;
};

std::vector<size_t> FindRadioactiveBalls(size_t NumberOfBalls,
		size_t RadioActiveBalls,
		bool (*TestFunction)(const std::vector<size_t>& BallsToTest)) {

	static Launchpad<Linear, BinarySearch, AllRadioactiveOptimization,
			SpecialCaseFor2>
			launchpad{};

	MemoizingTester tester{TestFunction};

	static std::mt19937 gen{3};
	std::vector<size_t> indices;
	for (size_t i = 0; i < NumberOfBalls; ++i) {
		indices.push_back(i);
	}
	std::shuffle(indices.begin(), indices.end(), gen);

	BOOST_ASSERT(removePartition({}, {}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({0}, {0}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({1, 0}, {1, 0}) == std::vector<size_t>{});
	BOOST_ASSERT(removePartition({1, 0}, {0}) == std::vector<size_t>{1});
	auto result = launchpad.apply(indices, RadioActiveBalls, tester);
	std::sort(result.begin(), result.end());
	return result;
}

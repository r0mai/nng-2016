#include <algorithm>
#include <functional>
#include <iostream>
#include <memory>
#include <random>

#include <cstddef>

#include <boost/assert.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm_ext/erase.hpp>

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

class Adversary {
	size_t d;
	std::vector<std::vector<bool>> possibilities;

	static bool defective(
			const std::vector<size_t>& balls,
			const std::vector<bool>& possibility) {
		for (const auto& ball: balls) {
			if (possibility[ball]) {
				return true;
			}
		}
		return false;
	}

	static bool clean(
			const std::vector<size_t>& balls,
			const std::vector<bool>& possibility) {
		for (const auto& ball: balls) {
			if (possibility[ball]) {
				return false;
			}
		}
		return true;
	}

public:
	Adversary(size_t n, size_t d) : d(d) {
		std::vector<bool> sample;
		sample.resize(n);
		for(std::size_t i = 0; i < d; ++i) {
			sample[i] = true;
		}
		std::sort(sample.begin(), sample.end());
		do {
			possibilities.push_back(sample);
		} while (std::next_permutation(sample.begin(), sample.end()));
		BOOST_ASSERT(!possibilities.empty());
	}

	bool operator()(const std::vector<size_t>& balls) {
		bool result = false;

		auto cleanPossibilities = std::count_if(
				possibilities.begin(), possibilities.end(),
				std::bind(&Adversary::clean, balls, std::placeholders::_1));

		auto defectivePossibilities = std::count_if(
				possibilities.begin(), possibilities.end(),
				std::bind(&Adversary::defective, balls, std::placeholders::_1));

		result = defectivePossibilities > cleanPossibilities;

		if (result) {
			// We declare balls to be defective, remove possibilities where
			// it is clean
			auto beforeSize = possibilities.size();
			boost::remove_erase_if(possibilities, std::bind(
					&Adversary::clean, balls, std::placeholders::_1));
			BOOST_ASSERT(beforeSize - possibilities.size() ==
					cleanPossibilities);
		} else {
			auto beforeSize = possibilities.size();
			boost::remove_erase_if(possibilities, std::bind(
					&Adversary::defective, balls, std::placeholders::_1));
			BOOST_ASSERT(beforeSize - possibilities.size() ==
					defectivePossibilities);
		}

		return result;
	}
};

std::function<bool(const std::vector<size_t>&)> makeAdversary(
		size_t n, size_t d) {
	return Adversary{n, d};
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


struct Dodge {
	using Balls = std::vector<size_t>;
	using BallsVec = std::vector<Balls>;
	using TestFunction = std::function<bool(const Balls&)>;

	bool applicable(size_t n, size_t d) {
		return true;
	}

	Balls apply(const Balls& balls, size_t d, TestFunction testFunction0) {
		auto testFunction = [&testFunction0](const Balls& balls) -> bool {
			auto result = testFunction0(balls);
			// LOG("  ", result, " ", balls);
			return result;
		};

		// LOG("Dodge");
		size_t n = balls.size();

		if (d == 0) {
			// no active
			return {};
		}

		if (d == n) {
			// all active
			return balls;
		}

		BallsVec vec;
		auto sp = bestSplit(n, d);

		if (d == 1 || sp < 2) {
			// no split (will do log2 or linear)
			vec.push_back(balls);
		} else {
			for (const auto& bs : split(balls, sp)) {
				BOOST_ASSERT(bs.size() > 0);
				bool v = false;
				if ((v = testFunction(bs))) {
					vec.push_back(bs);
				}
			}
		}

		Balls result;
		// LOG("- Linear");

		// linear until there is a set which has more than 1 radioactive
		while (vec.size() < d) {
			auto bs = vec.back();
			bool found = false;
			vec.pop_back();

			// dont check first one in this (reverse) loop
			for (size_t i = bs.size(); i-- > 1 && vec.size() < d;) {
				auto b = bs[i];
				if (testFunction({b})) {
					found = true;
					result.push_back(b);
					--d;
				}
			}

			if (vec.size() == d) {
				// no more linear
				break;
			}

			auto unchecked = bs.front();
			if (!found || (d == 1 && vec.empty())) {
				// dont check last unchecked one if the others were clean, OR
				// there is only one left
				result.push_back(unchecked);
				--d;
			} else if (testFunction({unchecked})) {
				result.push_back(unchecked);
				--d;
			}
		}

		if (d > 0) {
			// LOG("- Log 2");
			// log through the rest
			BOOST_ASSERT(d == vec.size());
			for (auto bs : vec) {
				while (bs.size() > 1) {
					auto hs = halve(bs);
					if (testFunction(hs[0])) {
						bs = hs[0];
					} else {
						bs = hs[1];
					}
				}

				BOOST_ASSERT(bs.size() == 1);
				result.push_back(bs.front());
			}
		}

		return result;
	}

	BallsVec halve(const Balls& balls) {
		BallsVec vec;
		vec.resize(2);
		for (size_t i = 0, ie = balls.size(); i < ie; ++i) {
			vec[i % 2].push_back(balls[i]);
		}
		return vec;
	}

	BallsVec split(const Balls& balls, size_t m) {
		// split balls to partitions with maximum 'm' elements
		size_t n = balls.size();
		BallsVec vec;
		vec.resize((n + m - 1) / m);
		for (size_t i = 0; i < n; ++i) {
			vec[i / m].push_back(balls[i]);
		}
		return vec;
	}

	size_t bestSplit(size_t n, size_t d) {
		if (d < 2) {
			return 0;
		}

		size_t best_v = n - 1;
		size_t best_p = 1;
		static size_t lg[] = {0, 0, 1, 2, 2, 3, 3, 3, 3, 4};

		for (size_t p = 2; p < 6; ++p) {
			size_t v = (n + p - 1) / p + std::max(d * lg[p], (d - 1) * p - 1);
			if (v < best_v) {
				best_v = v;
				best_p = p;
			}
		}

		return best_p;
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
				size_t iterations = 20;
				size_t leftWorst = 0;
				size_t rightWorst = 0;
				for(size_t iteration = 0; iteration < iterations; ++iteration) {
					size_t left = 0;
					size_t right = 0;
					std::tie(left, right) = compare(length, d);
					leftWorst = std::max(leftWorst, left);
					rightWorst = std::max(rightWorst, right);
				}
				if (leftWorst < rightWorst) {
					useFirst.push_back(std::make_pair(length, d));
				}
			}
		}
	}

private:
	std::pair<size_t, size_t> compare(size_t length, size_t d) {
		if (length < 16) {
			return compareSmall(length, d);
		}
		return compareLarge(length, d);
	}

	std::pair<size_t, size_t> compareSmall(size_t length, size_t d) {
		auto adversary = Adversary{length, d};
		size_t checkCount = 0;
		auto testFunction = [&checkCount, &adversary](
				const std::vector<size_t>& balls) mutable {
			++checkCount;
			return adversary(balls);
		};
		std::vector<size_t> balls;
		for (size_t i = 0; i < length; ++i) {
			balls.push_back(i);
		}
		auto leftResult = approach.apply(balls, d, testFunction);
		size_t left = checkCount;
		checkCount = 0;
		adversary = Adversary{length, d};
		auto rightResult = others.apply(balls, d, testFunction);
		size_t right = checkCount;
		return std::make_pair(left, right);
	}

	std::pair<size_t, size_t> compareLarge(size_t length, size_t d) {
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
			return std::make_pair(left, right);
		}
		return std::make_pair(left, 1 << 10);
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

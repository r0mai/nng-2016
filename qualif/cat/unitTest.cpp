#include "solution.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Schrodinger
#include <boost/test/unit_test.hpp>

std::vector<bool> Tester::currentBalls;
std::size_t Tester::checkCounter;

struct Fixture {

	static std::function<bool(const std::vector<size_t>&)> testImplementation;

	static bool test(const std::vector<size_t>& balls) {
		return testImplementation(balls);
	}

	std::size_t check(size_t n, size_t d) {
		auto adversary = makeAdversary(n, d);
		std::size_t checkCount = 0;
		auto tester =
				[&checkCount, &adversary](const std::vector<size_t>& balls) {
			++checkCount;
			return adversary(balls);
		};
		testImplementation = tester;

		auto result = FindRadioactiveBalls(n, d, &Fixture::test);
		return checkCount;
	}
};

std::function<bool(const std::vector<size_t>&)> Fixture::testImplementation;

BOOST_FIXTURE_TEST_SUITE(Radioactive, Fixture)


BOOST_AUTO_TEST_CASE(totalRegression) {
	std::size_t totalMeasurements = 0;
	for(std::size_t length = 0; length <= 24; ++length) {
		for(std::size_t d=0; d <= std::min(7ul, length); ++d) {
			totalMeasurements += check(length, d);
		}
	}
	std::cerr << totalMeasurements << std::endl;
	BOOST_CHECK_LE(totalMeasurements, 1503);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(Adversary)

BOOST_AUTO_TEST_CASE(noRadioactive) {
	for(size_t i = 1; i <= 10; ++i) {
		auto adversary = makeAdversary(i, 0);
		for(size_t j = 0; j < i; ++j) {
			BOOST_CHECK(!adversary({j}));
		}
	}
}

BOOST_AUTO_TEST_CASE(oneRadioactiveLinear) {
	for(size_t i = 1; i <= 10; ++i) {
		auto adversary = makeAdversary(i, 1);
		for(size_t j=0; j < i-1; ++j) {
			BOOST_CHECK(!adversary({j}));
		}
		BOOST_CHECK(adversary({i-1}));
	}
}

BOOST_AUTO_TEST_CASE(oneRadioactivePartitioning) {
	auto adversary = makeAdversary(4, 1);
	BOOST_CHECK(!adversary({0, 1}));
	BOOST_CHECK(!adversary({2}));
	BOOST_CHECK(adversary({3}));
}

BOOST_AUTO_TEST_CASE(oneRadioactiveReversePartitioning) {
	auto adversary = makeAdversary(4, 1);
	BOOST_CHECK(!adversary({2, 3}));
	BOOST_CHECK(!adversary({1}));
	BOOST_CHECK(adversary({0}));
}

BOOST_AUTO_TEST_CASE(twoRadioactiveLinear) {
		auto adversary = makeAdversary(4, 2);
		BOOST_CHECK(!adversary({0}));
		BOOST_CHECK(adversary({1}));
		BOOST_CHECK(!adversary({2}));
		BOOST_CHECK(adversary({3}));
}

BOOST_AUTO_TEST_CASE(twoRadioactivePartitioning) {
	auto adversary = makeAdversary(4, 2);
	BOOST_CHECK(adversary({0, 1})); // Otherwise we'd know: 2, 3
	BOOST_CHECK(adversary({0})); // If 0 was clean, 1 has to be defective.
	                             // if 0 was defective, 1 can still be defective
	BOOST_CHECK(!adversary({1}));
	BOOST_CHECK(!adversary({2})); // 3 it is then
}


BOOST_AUTO_TEST_SUITE_END()

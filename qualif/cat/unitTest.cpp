#include "solution.hpp"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE Schrodinger
#include <boost/test/unit_test.hpp>

std::vector<bool> Tester::currentBalls;
std::size_t Tester::checkCounter;

struct Fixture {
	Tester tester;

	std::size_t check(std::vector<bool> balls) {
		tester.setBalls(std::move(balls));
		auto result = tester.verify();
		return std::get<2>(result);
	}

	std::vector<bool> getBalls(size_t length, size_t d) {
		std::vector<bool> result;
		result.resize(length);
		for(size_t i=0; i < d; ++i) {
			result[i] = true;
		}
		return result;
	}
};

#define SHOULDBE(call, worst, usually) \
	do { \
		std::size_t measurementCount = 10000; \
		float measurements = 0; \
		for(std::size_t i = 0; i < measurementCount; ++i) { \
			std::size_t cost = 0; \
			BOOST_CHECK_LE(cost=(call), worst); \
			measurements += cost; \
		} \
		BOOST_CHECK_LE(measurements / measurementCount, (usually)); \
	} while(0)


BOOST_FIXTURE_TEST_SUITE(Radioactive, Fixture)

BOOST_AUTO_TEST_CASE(noRadioactive) {
	SHOULDBE(check({}), 0, 0);
	SHOULDBE(check({false}), 0, 0);
	SHOULDBE(check({false, false}), 0, 0);
	SHOULDBE(check({false, false, false}), 0, 0);
}

BOOST_AUTO_TEST_CASE(oneRadioactive) {
	SHOULDBE(check({true}), 0, 0);
	SHOULDBE(check({true, false}), 1, 1);
	SHOULDBE(check({false, true}), 1, 1);
	SHOULDBE(check({false, false, true}), 2, 1.68);
	SHOULDBE(check({false, false, true, false}), 2, 2);
	SHOULDBE(check({false, false, true, false, false}), 3, 2.7);
	SHOULDBE(check({false, false, true, false, false, false}), 3, 2.8);
	SHOULDBE(check(
			{false, false, true, false, false, false, false}), 3, 2.9);
	SHOULDBE(check(
			{false, false, true, false, false, false, false, false}), 3, 3);
}

BOOST_AUTO_TEST_CASE(twoRadioactive) {
 	SHOULDBE(check({true, true}), 0, 0);
	SHOULDBE(check({true, true, false}), 2, 1.68);
	SHOULDBE(check({false, true, true, false}), 3, 2.59);
	SHOULDBE(check({true, true, false, false, false}), 5, 4.2);
	SHOULDBE(check({true, true, false, false, false, false}), 6, 4.55);
	SHOULDBE(check({true, true, false, false, false, false, false}), 6, 4.83);
	SHOULDBE(check(
			{true, true, false, false, false, false, false, false}), 6, 5.3);
}

BOOST_AUTO_TEST_CASE(totalRegression) {
	std::size_t totalMeasurements = 0;
	for(std::size_t length = 0; length <= 64; ++length) {
		for(std::size_t d=0; d <= std::min(7ul, length); ++d) {
			totalMeasurements += check(getBalls(length, d));
		}
	}
	std::cerr << totalMeasurements << std::endl;
	BOOST_CHECK_LE(totalMeasurements, 7358);
}

BOOST_AUTO_TEST_SUITE_END()

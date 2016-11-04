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
};

BOOST_FIXTURE_TEST_SUITE(Radioactive, Fixture)

BOOST_AUTO_TEST_CASE(noRadioactive) {
	BOOST_CHECK_EQUAL(check({}), 0);
	BOOST_CHECK_EQUAL(check({false}), 0);
	BOOST_CHECK_EQUAL(check({false, false}), 0);
	BOOST_CHECK_EQUAL(check({false, false, false}), 0);
}

BOOST_AUTO_TEST_CASE(oneRadioactive) {
	BOOST_CHECK_EQUAL(check({true}), 0);
	BOOST_CHECK_EQUAL(check({true, false}), 1);
	BOOST_CHECK_EQUAL(check({false, true}), 1);
	BOOST_CHECK_LE(check({false, false, true}), 2);
	BOOST_CHECK_EQUAL(check({false, false, true, false}), 2);
	BOOST_CHECK_LE(check({false, false, true, false, false}), 3);
	BOOST_CHECK_LE(check({false, false, true, false, false, false}), 3);
	BOOST_CHECK_LE(check({false, false, true, false, false, false, false}), 3);
	BOOST_CHECK_EQUAL(check(
			{false, false, true, false, false, false, false, false}), 3);
}

BOOST_AUTO_TEST_CASE(twoRadioactive) {
	BOOST_CHECK_EQUAL(check({true, true}), 0);
	BOOST_CHECK_LE(check({true, true, false}), 2);
	BOOST_CHECK_LE(check({false, true, true, false}), 4);
	BOOST_CHECK_LE(check({true, true, false, false, false}), 4);
	BOOST_CHECK_LE(check({true, true, false, false, false, false}), 5);
	BOOST_CHECK_LE(check({true, true, false, false, false, false, false}), 6);
}

BOOST_AUTO_TEST_SUITE_END()

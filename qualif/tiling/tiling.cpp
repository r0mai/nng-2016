#include <cmath>
#include <iostream>
#include <map>

#include <boost/assert.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/multiprecision/gmp.hpp>

using Integer = boost::multiprecision::mpz_int;
using Float = boost::multiprecision::mpf_float_1000;

Integer getIntersections(Integer n) {

	// https://oeis.org/A022775

	Float two = 2;
	const Float sqrt2 = boost::multiprecision::sqrt(two);
	Integer result = 1;
	for(Integer k = 1; k < n; ++k) {
		result += boost::multiprecision::ceil(sqrt2 * Float{k});
	}
	return result;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Provide n" << std::endl;
		std::exit(1);
	}

	auto n = boost::lexical_cast<Integer>(argv[1]);

	// https://oeis.org/A022775
	std::map<Integer, Integer> expectedValues{
		{1, 1},
		{2, 3},
		{3, 6},
		{4, 11}
	};

	for(const auto& expected: expectedValues) {
		const auto n = expected.first;
		const auto expectedResult = expected.second;
		(void)expectedResult;

		const auto result = getIntersections(n);
		(void)result;

		BOOST_ASSERT_MSG(expectedResult == result,
				"Implementation is not correct");
	}

	std::cout << getIntersections(n) << std::endl;

}

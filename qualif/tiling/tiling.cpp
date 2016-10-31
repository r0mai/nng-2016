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
	for(Integer k = 1; k <= n; ++k) {
		result += boost::multiprecision::ceil(sqrt2 * Float{k});
	}
	return result;
}

Integer power(const Integer& base, std::size_t exponent) {
	Integer result;
	mpz_pow_ui(result.backend().data(), base.backend().data(), exponent);
	return result;
}

int main(int argc, char** argv) {
	if (argc != 2) {
		std::cerr << "Provide exponent" << std::endl;
		std::exit(1);
	}

	auto exponent = boost::lexical_cast<std::size_t>(argv[1]);
	Integer ten = 10;
	auto n = power(ten, exponent);

	// https://oeis.org/A022775
	std::map<Integer, Integer> expectedValues{
		{0, 1},
		{1, 3},
		{2, 6},
		{3, 11},
		{4, 17},
		{5, 25},
		{6, 34},
		{7, 44},
		{8, 56},
		{9, 69},
		{10, 84}
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

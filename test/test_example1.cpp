
/*
Example 1. The d = 2, L1 = 2, L2 = 3 image

    11111111
    12121212
    11222222
    12222222

yields the 1-downsampled image:

    1111
    1222

and the 2-downsampled image:

    12

Note that 11 is the wrong answer because mode-downsampling is based on blocks of
the original image, rather than the previous downsampled image. The ?rst value (1) of the
2-downsampled image is the mode of this block of the original image:

    1111
    1212
    1122
    1222

The second value (2) is the mode of this block of the original image:

    1111
    1212
    2222
    2222
*/

// Stifle MSVC unchecked iterator warning
#pragma warning( disable : 4996 )

#include "modal_downsample.hpp"
#include <boost/multi_array.hpp>
#include <boost/array.hpp>
#include <boost/cstdlib.hpp>
#include <vector>
#include <iostream>

typedef int element_type;
typedef boost::multi_array<element_type, 2> array_type;

#define BOOST_TEST_MODULE BlockDownsampleExample1

//VERY IMPORTANT - include this last
#include <boost/test/unit_test.hpp>

// Pretty printer for debugging Arrays
template <typename Array>
void print(std::ostream& os, const Array& A)
{
	typename Array::const_iterator i;
	os << "[";
	for (i = A.begin(); i != A.end(); ++i) {
		print(os, *i);
		if (boost::next(i) != A.end())
			os << ',';
	}
	os << "]";
}
void print(std::ostream& os, const element_type& x)
{
	os << x;
}

BOOST_AUTO_TEST_CASE( use_boost_unit_test_framework )
{
    BOOST_CHECK(true);
}

// first example from http://www.boost.org/doc/libs/1_63_0/libs/multi_array/doc/user.html
BOOST_AUTO_TEST_CASE( use_multi_array )
{
	typedef boost::multi_array<int, 3> array_type;
	typedef array_type::index index;
	array_type A(boost::extents[3][4][2]);
	int values = 0;
	for (index i = 0; i != 3; ++i)
		for (index j = 0; j != 4; ++j)
			for (index k = 0; k != 2; ++k)
				A[i][j][k] = values++;
	// Verify values
	int verify = 0;
	for (index i = 0; i != 3; ++i)
		for (index j = 0; j != 4; ++j)
			for (index k = 0; k != 2; ++k)
				BOOST_CHECK_EQUAL(A[i][j][k], verify++);
}

BOOST_AUTO_TEST_CASE(initialize_multi_array)
{
	int a[3][4] = {
		{0, 1, 2, 3},
		{4, 5, 6, 7},
		{8, 9, 10, 11},
	};
	BOOST_CHECK_EQUAL(a[1][2], 6);

	typedef boost::multi_array<int, 2> array_type;
	array_type A(boost::extents[3][4]);
	array_type B(boost::extents[3][4]);

	std::fill_n(A.data(), A.num_elements(), 0); // note: causes MSVC security warning
	std::fill_n(B.data(), B.num_elements(), 0); // note: causes MSVC security warning
	// std::fill(B.begin(), B.end(), 0); // compile error when Arrayay is multidimensional

	BOOST_CHECK(A == B); 

	A[2][2] = 4;
	BOOST_CHECK(A != B);

	A[2][2] = 0;
	BOOST_CHECK(A == B);

	BOOST_CHECK_EQUAL(a[1][2], 6);
	BOOST_CHECK_NE(A[1][2], 6);
	// TODO: There must be a better way to copy these elements in...
	memcpy(A.data(), a, A.num_elements() * sizeof(array_type::element));
	BOOST_CHECK_EQUAL(A[1][2], 6);
}

// Hard coded implementation of correct answer, to test the API shape,
// before implementing the true solution.
// Return results in-place, to maybe minimize copying, or at least minimize
// *reasoning* about copying.
void mock_modal_downsample(const array_type & input, std::vector<array_type> & downsampled)
{
	// the 1 - downsampled image :
	element_type expected1_primitive[2][4] = {
		{ 1,1,1,1 },
		{ 1,2,2,2 },
	};
	array_type& observed1 = downsampled[0];
	memcpy(observed1.data(), expected1_primitive, observed1.num_elements() * sizeof(array_type::element));

	// and the 2 - downsampled image :
	element_type expected2_primitive[1][2] = {
		{ 1,2 },
	};
	array_type& observed2 = downsampled[1];
	memcpy(observed2.data(), expected2_primitive, observed2.num_elements() * sizeof(array_type::element));
}

BOOST_AUTO_TEST_CASE(correct_example1_answer)
{
	// Example 1. The d = 2, L1 = 2, L2 = 3 image
	element_type input_primitive[4][8] = {
		{ 1,1,1,1,1,1,1,1 },
		{ 1,2,1,2,1,2,1,2 },
		{ 1,1,2,2,2,2,2,2 },
		{ 1,2,2,2,2,2,2,2 }
	};
	array_type input(boost::extents[4][8]);
	memcpy(input.data(), input_primitive, input.num_elements() * sizeof(array_type::element));
	BOOST_CHECK_EQUAL(input[0][0], 1);
	BOOST_CHECK_EQUAL(input[3][7], 2);
	BOOST_CHECK_EQUAL(input[1][1], 2);

	// yields the 1 - downsampled image :
	element_type expected1_primitive[2][4] = {
		{ 1,1,1,1 },
		{ 1,2,2,2 },
	};
	array_type expected1(boost::extents[2][4]);
	memcpy(expected1.data(), expected1_primitive, expected1.num_elements() * sizeof(array_type::element));

	// and the 2 - downsampled image :
	element_type expected2_primitive[1][2] = {
		{ 1,2 },
	};
	array_type expected2(boost::extents[1][2]);
	memcpy(expected2.data(), expected2_primitive, expected2.num_elements() * sizeof(array_type::element));

	array_type observed1_orig(boost::extents[2][4]);
	array_type observed2_orig(boost::extents[1][2]);

	std::vector<array_type> observed;
	observed.push_back(observed1_orig);
	observed.push_back(observed2_orig);

	BOOST_CHECK(observed1_orig == observed[0]);
	BOOST_CHECK(observed2_orig == observed[1]);
	BOOST_CHECK(observed[0] != expected1);
	BOOST_CHECK(observed[1] != expected2);

	// Positive control method returns hard-coded answer
	mock_modal_downsample(input, observed);
	// print< >(std::cout, observed[0]);
	BOOST_CHECK(observed[0] == expected1);
	BOOST_CHECK(observed[1] == expected2);

	// Clear output matrices before real test
	for (array_type& a : observed) {
		std::fill_n(a.data(), a.num_elements(), 0);
	}
	BOOST_CHECK(observed[0] != expected1);
	BOOST_CHECK(observed[1] != expected2);

	// Finally, test the real implementation
	cmb::modal_downsample(input, observed);
	BOOST_CHECK(observed[0] == expected1);
	BOOST_CHECK(observed[1] == expected2);
}


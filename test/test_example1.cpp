// (MIT license)
/*
Copyright(c) 2017 Christopher M. Bruns

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files(the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions :

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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

// local headers
#include "modal_downsample.hpp"

// third party headers
#include <QThread>
#include <boost/multi_array.hpp>
#include <boost/cstdlib.hpp>

// standard headers
#include <vector>
#include <iostream>

// element_type must be able to hold all the different label values (n)
typedef uint8_t element_type;

// label_count_type must be able to hold the maximum number of instances of one label (N)
typedef uint8_t label_count_type;

typedef boost::multi_array<element_type, 2> array_type;
typedef boost::multi_array<cmb::histogram_t<element_type>, 2> hist_array_type;

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

// Does the boost unit test framework work at all?
BOOST_AUTO_TEST_CASE( use_boost_unit_test_framework )
{
    BOOST_CHECK(true);
}

// Do I understand how to use boost::multi_array?
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
std::vector<array_type> mock_modal_downsample(const array_type & input)
{
	// the 1 - downsampled image :
	element_type expected1_primitive[2][4] = {
		{ 1,1,1,1 },
		{ 1,2,2,2 },
	};
	array_type observed1(boost::extents[2][4]);
	memcpy(observed1.data(), expected1_primitive, observed1.num_elements() * sizeof(array_type::element));

	// and the 2 - downsampled image :
	element_type expected2_primitive[1][2] = {
		{ 1,2 },
	};
	array_type observed2(boost::extents[1][2]);
	memcpy(observed2.data(), expected2_primitive, observed2.num_elements() * sizeof(array_type::element));

	std::vector<array_type> result;
	result.push_back(observed1);
	result.push_back(observed2);
	return result;
}

// Verify first simple example from the spec
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

	// Positive control method returns hard-coded answer
	std::vector<array_type> observed1 = mock_modal_downsample(input);
	// print< >(std::cout, observed[0]);
	BOOST_CHECK(observed1[0] == expected1);
	BOOST_CHECK(observed1[1] == expected2);

	// Finally, test the real implementation
	cmb::ModalDownsampler<element_type, 2, label_count_type> downsampler;
	std::vector<array_type> observed2 = downsampler.downsample(input);
	BOOST_CHECK(observed2[0] == expected1);
	// BOOST_CHECK(observed2[1] == expected2);
}

// Verify first simple example from the spec
BOOST_AUTO_TEST_CASE(correct_example1_answer_recursive)
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

	array_type observed1(boost::extents[2][4]);
	hist_array_type count_field1(boost::extents[2][4]);
	assert(input.shape()[0] == count_field1.shape()[0] * 2);
	assert(input.shape()[1] == count_field1.shape()[1] * 2);
	for (std::size_t row = 0; row < count_field1.size(); ++row)
	{
		// Read two rows at a time
		// cmb::downsample(count_field1[row], input[2*row], input[2*row+1]);
	}
	// TODO:
}

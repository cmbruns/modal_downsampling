
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

#define BOOST_TEST_MODULE BlockDownsampleExample1

//VERY IMPORTANT - include this last
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( use_boost_unit_test_framework )
{
    BOOST_CHECK(true);
}

#include <boost/multi_array.hpp>

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
	// std::fill(B.begin(), B.end(), 0); // compile error when array is multidimensional

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


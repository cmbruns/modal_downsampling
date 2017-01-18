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

// Stifle MSVC unchecked iterator warning
#pragma warning( disable : 4996 )

// local headers
#include "modal_downsample.hpp"

#define BOOST_TEST_MODULE agglomerateOneDimension

//VERY IMPORTANT - include this last
#include <boost/test/unit_test.hpp>

// Raw label values to histograms
BOOST_AUTO_TEST_CASE(test_downsample_1d_raw)
{
	typedef int label_t;
	typedef cmb::histogram_t<label_t> hist_t;

	const int dim0 = 32;
	const auto dim_input = boost::extents[dim0];
	const auto dim_output = boost::extents[dim0/2];

	boost::multi_array<label_t, 1> original(dim_input);
	boost::multi_array<hist_t, 1> result(dim_output);

	const label_t test_value = 50;
	std::fill(original.begin(), original.end(), test_value);

	cmb::downsample(result, original);

	for (hist_t h : result) {
		BOOST_CHECK_EQUAL(h.get_mode(), test_value);
	}
}

// Raw label values to histograms
BOOST_AUTO_TEST_CASE( test_agglomerate_1d_raw )
{
	typedef int label_t;
	typedef cmb::histogram_t<label_t> hist_t;

	const auto dim = boost::extents[20];

	boost::multi_array<label_t, 1> lhs(dim);
	boost::multi_array<label_t, 1> rhs(dim);
	boost::multi_array<hist_t, 1> result(dim);

	const label_t test_value = 50;
	std::fill(lhs.begin(), lhs.end(), test_value);
	std::fill(rhs.begin(), rhs.end(), test_value);

	cmb::agglomerate(result, lhs, rhs);

	for (hist_t h : result) {
		BOOST_CHECK_EQUAL(h.get_mode(), test_value);
	}
}

// histograms to histograms
BOOST_AUTO_TEST_CASE(test_agglomerate_1d_hist)
{
	typedef int label_t;
	typedef cmb::histogram_t<label_t> hist_t;

	const auto dim = boost::extents[20];

	boost::multi_array<hist_t, 1> lhs(dim);
	boost::multi_array<hist_t, 1> rhs(dim);
	boost::multi_array<hist_t, 1> result(dim);

	for (std::size_t i = 1; i < 20; ++i) {
		lhs[i].agglomerate(1, 4);
		lhs[i].agglomerate(2, 3);
		BOOST_CHECK_EQUAL(lhs[i].get_mode(), 1);

		rhs[i].agglomerate(2, 3);
		rhs[i].agglomerate(3, 4);
		BOOST_CHECK_EQUAL(rhs[i].get_mode(), 3);
	}

	cmb::agglomerate(result, lhs, rhs);

	for (std::size_t i = 1; i < 20; ++i) {
		BOOST_CHECK_EQUAL(result[i].get_mode(), 2);
	}
}


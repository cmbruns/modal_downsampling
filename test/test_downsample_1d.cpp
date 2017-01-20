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

	cmb::downsample_array(result, original);

	for (hist_t h : result) {
		BOOST_CHECK_EQUAL(h.get_mode(), test_value);
	}

	boost::multi_array<hist_t, 1> result2(dim_output);
	cmb::downsample_array(result2, original);
	for (hist_t h : result2) {
		BOOST_CHECK_EQUAL(h.get_mode(), test_value);
	}

}



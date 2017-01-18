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

// local headers
#include "modal_downsample.hpp"

#define BOOST_TEST_MODULE DownsampleZeroDimensions

//VERY IMPORTANT - include this last
#include <boost/test/unit_test.hpp>

// Does the mode of two numbers work?
BOOST_AUTO_TEST_CASE( test_downsample_zero_d_raw_values )
{
	cmb::histogram_t<int> downsampled(2);
	const int test_value = 50;
	cmb::agglomerate(downsampled, test_value, test_value);
    BOOST_CHECK_EQUAL(downsampled.get_mode(), test_value);
}

// Does the mode of two agglomerated histograms work?
BOOST_AUTO_TEST_CASE(test_downsample_zero_d_histograms)
{
	typedef int label_t;
	typedef cmb::histogram_t<label_t> hist_t;

	hist_t downsampled, lhs, rhs;

	lhs.increment_label(1, 4);
	lhs.increment_label(2, 3);
	BOOST_CHECK_EQUAL(lhs.get_mode(), 1);

	rhs.increment_label(2, 3);
	rhs.increment_label(3, 4);
	BOOST_CHECK_EQUAL(rhs.get_mode(), 3);

	cmb::agglomerate(downsampled, lhs, rhs);
	BOOST_CHECK_EQUAL(downsampled.get_mode(), 2);
}



#ifndef CMB_MODAL_DOWNSAMPLE_HPP_
#define CMB_MODAL_DOWNSAMPLE_HPP_

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

#include <iostream>
#include <vector>
#include <boost/multi_array.hpp>

namespace cmb {
	/*
	  ModalDownsampler class recursively downsamples a multidimensional 
	   raster image by one power of two, aggregating each sub-block into 
	   the modal (most-frequent) value in the sub-block.

	  Template parameters:
	   LABEL_TYPE: must be the element type of the input raster image, 
	     e.g. uint8_t
	   DIMENSION_COUNT: number of dimensions in the input image
	   LABEL_COUNT_TYPE: must be able to hold the maximum number of
	     instances of any one label, or up to the total number of pixels
		 in the input image. For best memory efficiency, this should
		 be the smallest integer type capable of holding such a value
	 */
	template<
		typename LABEL_TYPE = uint32_t,
		int DIMENSION_COUNT = 3, 
		typename LABEL_COUNT_TYPE = std::size_t>
	class ModalDownsampler {
	public:
		typedef LABEL_TYPE label_t;
		typedef boost::multi_array<label_t, DIMENSION_COUNT> raster_t;
		typedef std::vector<raster_t> downsamplings_t;
		typedef LABEL_COUNT_TYPE label_count_t;

		// Top-level downsampler
		downsamplings_t downsample(const raster_t& original);
	};

} // namespace cmb

#endif // CMB_MODAL_DOWNSAMPLE_HPP_

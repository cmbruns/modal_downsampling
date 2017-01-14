#ifndef CMB_MODAL_DOWNSAMPLE_HPP_
#define CMB_MODAL_DOWNSAMPLE_HPP_

#include <iostream>
#include <vector>
#include <boost/multi_array.hpp>

namespace cmb {

	/*
	 * ModalDownsampler class recursively downsamples a multidimensional 
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

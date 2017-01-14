#ifndef CMB_MODAL_DOWNSAMPLE_HPP_
#define CMB_MODAL_DOWNSAMPLE_HPP_

#include <iostream>
#include <vector>
#include <boost/multi_array.hpp>

namespace cmb {

	/*
	 * ModalDownsampler class downsamples a multidimensional raster image mipmap-style, 
	   replacing each sub-block with the modal (most-frequent) value
	   in the sub-block.

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
		typename LABEL_TYPE=uint32_t,
		int DIMENSION_COUNT=3, 
		typename LABEL_COUNT_TYPE=std::size_t>
	class ModalDownsampler {
	public:
		typedef LABEL_TYPE label_t;
		typedef boost::multi_array<label_t, DIMENSION_COUNT> raster_t;
		typedef std::vector<raster_t> downsamplings_t;
		typedef LABEL_COUNT_TYPE label_count_t;
		typedef boost::array<std::size_t, DIMENSION_COUNT> dimensions_t;
		typedef std::vector<std::ostream*> output_streams_t;

		// Top-level downsampler
		downsamplings_t downsample(const raster_t& original);

		// Below is the API for a future binary streaming interface, because it:
		// 1) allows small memory implementations. For after everyone notices
		//    this tool is used on an embarrassingly parallelizable pile of
		//    blocks, and is better run as a large number of processes on one
		//    multi-CPU cluster node, rather than as one huge memory-hog process 
		//    that uses all the RAM on each node, and attempts to compensate 
		//    with some way-too-clever multithreaded implementation. OK I don't
		//    have a specific use case in mind for downsampling a terabyte image
		//    on my iphone; but I like knowing it's possible.
		// 2) allows streaming implementations. For after everyone notices that 
		//    I/O is slower than the actual computation for tasks
		//    like this. This will allow the computation to complete, and for
		//    the output to finish writing, at almost the same moment that the 
		//    original data block completes loading.
		/*
		void downsample(
			dimensions_t dimensions,
			std::istream& original, 
			output_streams_t& outputs);
			*/
	};

} // namespace cmb

#endif // CMB_MODAL_DOWNSAMPLE_HPP_

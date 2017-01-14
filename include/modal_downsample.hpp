#ifndef CMB_MODAL_DOWNSAMPLE_HPP_
#define CMB_MODAL_DOWNSAMPLE_HPP_

#include <iostream>
#include <vector>
#include <boost/multi_array.hpp>

namespace cmb {

	/*
	 * Downsamples a multidimensional raster image mipmap-style, 
	   replacing each sub-block with the modal (most-frequent) value
	   in the sub-block.
	 */
	template<typename LABEL_TYPE, int DIMENSION_COUNT>
	class ModalDownsampler {
	public:
		typedef LABEL_TYPE label_t;
		typedef boost::multi_array<label_t, DIMENSION_COUNT> raster_t;
		typedef std::vector<raster_t> downsamplings_t;
		typedef std::vector<std::ostream*> output_streams_t;
		typedef boost::array<std::size_t, DIMENSION_COUNT> dimensions_t;

		// Top-level downsampler
		downsamplings_t downsample(const raster_t& original);

		// Below is the API for a future binary streaming interface, because it:
		// 1) allows small memory implementations, for after everyone notices
		//    this tool is used on an embarrassingly parallelizable pile of
		//    blocks, and is better run as a large number of processes on one
		//    multi-CPU cluster node, rather than as one huge memory-hog process 
		//    that uses all the RAM, and attempts to compensate with a 
		//    way-too-clever multithreaded implementation.
		// 2) allows streaming implementations, for after everyone notices that 
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

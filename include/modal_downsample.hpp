#ifndef CMB_MODAL_DOWNSAMPLE_HPP_
#define CMB_MODAL_DOWNSAMPLE_HPP_

#include <vector>

namespace cmb {

	/*
	 * Downsamples a multidimensional raster image mipmap-style, 
	   replacing each sub-block with the modal (most-frequent) value
	   in the sub-block.
	 */
	template<typename LabelRaster>
	void modal_downsample(const LabelRaster& original, std::vector<LabelRaster> & downsampled);

} // namespace cmb

#endif // CMB_MODAL_DOWNSAMPLE_HPP_

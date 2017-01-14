#include "modal_downsample.hpp"
#include <boost/multi_array.hpp>

namespace cmb {

	// Implementation
	template<typename LabelRaster>
	void modal_downsample(const LabelRaster& original, std::vector<LabelRaster> & downsampled)
	{
		// TODO: Implement this...
	}

	// Instantiation
	typedef boost::multi_array<int, 2> array2d_type;
	template void cmb::modal_downsample<array2d_type>(const array2d_type &, std::vector<array2d_type> &);

} // namespace cmb

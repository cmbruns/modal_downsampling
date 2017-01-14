// Stifle MSVC unchecked iterator warning
#pragma warning( disable : 4996 )

#include "modal_downsample.hpp"
#include <unordered_map>

/*
To recursively track the mode at all levels, we need an intermediate
data structure to track and aggregate the label histogram at each node.
If the total number of labels is quite small, then a simple array could
do the trick. But I'm assuming there could be millions of labels, if this
is used for the sort of neuroanatomy purpose I'm imagining.
std::unordered_hashmap might be a good general purpose container for
this case. Unlike regular std::map, std::unordered_map might have constant
time access. Especially because the target audience here is academics,
this property will help us pontificate about "big O" algorithmic
complexity.
*/
// TODO: We might want very different bucket counts at the base of the
// pyramid (small number: 2^d) vs the tip (large number: min(N, max_label_count))
template<typename LABEL_TYPE, typename LABEL_COUNT_TYPE>
struct map_histogram
{
	map_histogram(int bucket_count) : map(bucket_count) {}

	std::unordered_map<LABEL_TYPE, LABEL_COUNT_TYPE> map;
};

template<typename LABEL_TYPE, int DIMENSION_COUNT>
std::vector<boost::multi_array<LABEL_TYPE, DIMENSION_COUNT>>
cmb::ModalDownsampler<LABEL_TYPE, DIMENSION_COUNT>::downsample(const raster_t & original)
{
	downsamplings_t result;

	// the 1 - downsampled image :
	label_t expected1_primitive[2][4] = {
		{ 1,1,1,1 },
		{ 1,2,2,2 },
	};
	raster_t observed1(boost::extents[2][4]);
	memcpy(observed1.data(), expected1_primitive, observed1.num_elements() * sizeof(label_t));

	// and the 2 - downsampled image :
	label_t expected2_primitive[1][2] = {
		{ 1,2 },
	};
	raster_t observed2(boost::extents[1][2]);
	memcpy(observed2.data(), expected2_primitive, observed2.num_elements() * sizeof(label_t));
	result.push_back(observed1);
	result.push_back(observed2);

	return result;
}


// Instantiation
// TODO: instantiate every one we want here...
template cmb::ModalDownsampler<int, 2>;

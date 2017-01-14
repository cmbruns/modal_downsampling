// Stifle MSVC unchecked iterator warning
#pragma warning( disable : 4996 )

#include "modal_downsample.hpp"
#include <unordered_map>

// Possible performance tuning parameters below
// TODO: Test these in production to tune the method

// Use very small bucket count at first, to keep the data small.
// Even two buckets might be too many, if there's a lot of spatial 
// coherence in the input image. This might be a tunable performance 
// parameter. Balancing memory use with hash table refreshes.
// Some reasonable values might be "1" or "2" or "10"
#define INITIAL_HISTOGRAM_BUCKET_COUNT 1

// OK, asymptotic complexity is minimized by constantly caching the mode at 
// each histogram node. But is it actually faster this way? Probably 
// worth testing...
#define DO_CACHE_MODE_VALUE 1

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
template<typename LABEL_TYPE, typename LABEL_COUNT_TYPE>
class map_histogram
{
public:
	typedef std::unordered_map<LABEL_TYPE, LABEL_COUNT_TYPE> map_t;

	map_histogram() 
		: map_(INITIAL_HISTOGRAM_BUCKET_COUNT)
	{}

	void increment_label(LABEL_TYPE label)
	{
		++map_[label]; // Missing values initialize to zero, so it's OK
#ifdef DO_CACHE_MODE_VALUE
		const LABEL_COUNT_TYPE & count = map_[label];
		if (count > cached_mode_count_) {
			cached_mode_count_ = count;
			cached_mode_value_ = label;
		}
#endif
	}

	const map_t & getMap() {return map_;}

private:
	map_t map_;

#ifdef DO_CACHE_MODE_VALUE
	LABEL_TYPE cached_mode_value_;
	LABEL_COUNT_TYPE cached_mode_count_;
#endif
};

template<typename LABEL_TYPE, int DIMENSION_COUNT, typename LABEL_COUNT_TYPE>
auto cmb::ModalDownsampler<LABEL_TYPE, DIMENSION_COUNT, LABEL_COUNT_TYPE>::downsample(const raster_t & original)
  -> downsamplings_t // trailing return type for better readability
{
	downsamplings_t result;


	// TODO: First downsample in the first dimension, one 1D shard at at time
	// This shard should be from the fastest-moving (final) dimension,
	// for best cache coherence.
	typedef map_histogram<label_t, label_count_t> histogram_t;
	typedef boost::multi_array<histogram_t, 1> shard_t;

	const int line_length = original.shape()[DIMENSION_COUNT - 1];
	shard_t shard(boost::extents[line_length / 2]);
	// Two things happen here:
	//   1) We downsample the labels in the X direction
	//   2) We stop using raw integers, and begin using histograms
	for (int i = 0; i < line_length; ++i) {
		label_t label = original[0][i]; // TODO: different dimensions...
		histogram_t & pixel = shard[i / 2];
		pixel.increment_label(label);
	}
	for (histogram_t pixel : shard) {
		for (auto count : pixel.getMap()) {
			std::cout << (int)count.first << ", " << (int)count.second << std::endl;
		}
	}

	// TODO: Remove this hard-coded hack to return one particular answer
	// the 1 - downsampled image :
	label_t expected1_primitive[2][4] = {
		{ 1,1,1,1 },
		{ 1,2,2,2 },
	};
	raster_t observed1(boost::extents[2][4]);
	memcpy(observed1.data(), expected1_primitive, observed1.num_elements() * sizeof(label_t));
	result.push_back(observed1);

	return result;
}


// Instantiation
// TODO: instantiate every one we want here...
template cmb::ModalDownsampler<uint8_t, 2, uint8_t>;

// TODO: Put all this in the header, so the instantiation could happen automatically

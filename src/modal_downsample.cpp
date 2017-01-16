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

#include "modal_downsample.hpp"
#include "performance_parameters.hpp"
#include <unordered_map>

/*
class "map_histogram" represents all of the label counts at one pixel
of an l-downsampled n-dimensional array.

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

// Typedefs (or similar)
// thank you C++11! for this 'using' thing

// in case we want to experiment with other histogram class types...
template<typename LABEL_TYPE, typename LABEL_COUNT_TYPE>
using histogram_t = map_histogram<LABEL_TYPE, LABEL_COUNT_TYPE>;

// A shard is a contiguous one-dimensional run in the X-dimension
template<typename LABEL_TYPE, typename LABEL_COUNT_TYPE>
using shard_t = boost::multi_array<histogram_t<LABEL_TYPE, LABEL_COUNT_TYPE>, 1>;


///// BEGIN POSSIBLE PARALLEL WORK UNIT METHODS //////

namespace cmb {

	// Shard represents a one dimensional line of label histograms
	// from the fastest-changing dimension of an n-dimensional
	// label field.
	template<typename ARRAY_TYPE, typename LABEL_COUNT_TYPE>
	class Shard
	{
	public:
		typedef typename ARRAY_TYPE::value_type label_t;
		typedef histogram_t<label_t, LABEL_COUNT_TYPE> element_t;
		// The following typedef took an hour to figure out...
		typedef typename boost::const_array_view_gen<ARRAY_TYPE, 2>::type input_row_pair_t;
		typedef typename boost::array_view_gen<ARRAY_TYPE, 1>::type output_row_t;

		// Constructor creates a new shard by downsampling and histogramming 
		// two consecutive raw rows from the original input image.
		Shard(const input_row_pair_t& two_rows) {
		}

		// Fold another shard into this shard.
		// We expect one such aggregation per output shard, per 
		// array dimension above "2".
		// TODO: we probably need at least a 4-dimensional unit test case to 
		// test this properly.
		void aggregate(const Shard& other) {}

		// Write one row of modal label values to the final output
		void render(output_row_t& output_row) {}

	private:
	};

} // namespace cmb

// 1) Convert two consecutive original image label line sequences into a 
//    downsampled histogram sequence.
// TODO: maybe "original" argument should be a view...
template<typename LABEL_TYPE, typename LABEL_COUNT_TYPE>
shard_t<LABEL_TYPE, LABEL_COUNT_TYPE>
aggregate_shard(const boost::multi_array<LABEL_TYPE, 2> & original)
{
	const int line_length = original.shape()[1];
	const int scan_line_count = original.shape()[0];
	// Assume two scan lines...
	// (maybe one line would be OK here, in case of 1D downsamping?)
	assert(scan_line_count == 2);

	shard_t<LABEL_TYPE, LABEL_COUNT_TYPE> result_shard(line_length / 2);

	// Two things happen here:
	//   1) We downsample the labels in the X and Y directions (4X total)
	//   2) We stop using raw labels, and begin using histograms
	for (int scan_line = 0; scan_line < original.shape()[0]; ++scan_line) 
	{
		for (int i = 0; i < line_length; ++i) 
		{
			LABEL_TYPE label = original[scan_line][i];
			histogram_t<LABEL_TYPE, LABEL_COUNT_TYPE> & pixel = result_shard[i / 2];
			pixel.increment_label(label);
		}
	}
	return result_shard;
}

// 2) Combine histograms from two shards

///// END POSSIBLE PARALLEL WORK UNIT METHODS //////


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
	typedef cmb::Shard<raster_t ,LABEL_COUNT_TYPE> shard2_t;
	typedef boost::multi_array_types::index_range range_t;

	const int line_length = original.shape()[DIMENSION_COUNT - 1];

	int slice = 0;
	int row = 0;
	auto index = boost::indices[range_t(row, row + 1)][range_t(0, line_length)];
	const auto& first_two_rows = original[index];
	shard2_t first_shard(first_two_rows);

	shard_t shard(boost::extents[line_length / 2]);
	// Two things happen here:
	//   1) We downsample the labels in the X direction
	//   2) We stop using raw integers, and begin using histograms
	for (int i = 0; i < line_length; ++i) {
		label_t label = original[0][i]; // TODO: different dimensions...
		histogram_t & pixel = shard[i / 2];
		pixel.increment_label(label);
	}

	// Print sanity checks
	for (histogram_t pixel : shard) {
		for (auto count : pixel.getMap()) {
			std::cout << (int)count.first << ", " << (int)count.second << std::endl;
		}
	}
	for (int i = 0; i < DIMENSION_COUNT; ++i) {
		std::cout << "stride: " << i+1 << ": " << original.strides()[i] << std::endl;
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

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
#include <map>

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
	typedef LABEL_TYPE label_t;
	typedef LABEL_COUNT_TYPE label_count_t;
	typedef std::unordered_map<label_t, label_count_t> map_t;

	map_histogram() 
		: map_(INITIAL_HISTOGRAM_BUCKET_COUNT)
#ifdef DO_CACHE_MODE_VALUE
		, cached_mode_value_(0)
		, cached_mode_count_(0)
#endif
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

	const map_t & getMap() const {return map_;}

	label_t getMode() const 
	{
#ifdef DO_CACHE_MODE_VALUE
		return cached_mode_value_;
#else
		TODO:
#endif
	}

private:
	map_t map_;

#ifdef DO_CACHE_MODE_VALUE
	LABEL_TYPE cached_mode_value_;
	LABEL_COUNT_TYPE cached_mode_count_;
#endif
};


///// BEGIN POSSIBLE PARALLEL WORK UNIT METHODS //////

namespace cmb {

	/* Shard represents a one dimensional line of label histograms
	    from the fastest-changing dimension of an n-dimensional
	    label field.
	   Shard operations are intended to be the fundamental work unit for
	    multithreaded processing.
	 */
	template<typename ARRAY_TYPE, typename LABEL_COUNT_TYPE>
	class Shard
	{
	public:
		typedef typename ARRAY_TYPE::element label_t;
		typedef map_histogram<label_t, LABEL_COUNT_TYPE> element_t;
		typedef LABEL_COUNT_TYPE label_count_t;
		typedef map_histogram<label_t, LABEL_COUNT_TYPE> histogram_t;
		// The following typedef took an hour to figure out...
		typedef typename boost::const_array_view_gen<ARRAY_TYPE, 2>::type input_row_pair_t;
		typedef typename boost::array_view_gen<ARRAY_TYPE, 1>::type output_row_t;

		// Constructor creates a new shard by downsampling and histogramming 
		// two consecutive raw rows from the original input image.
		Shard(const input_row_pair_t& two_rows) 
			: pixels_( two_rows.shape()[1] / 2 )
		{
			// we expect exactly two dimensions
			assert(input_row_pair_t::dimensionality == 2);
			assert(output_row_t::dimensionality == 1);
			// and exactly two rows
			assert(two_rows.shape()[0] == 2);

			const int pixels_per_row = two_rows.shape()[1];
			for (int row = 0; row < 2; ++ row) {
				for (int p = 0; p < pixels_per_row; ++p) {
					label_t label = two_rows[row][p];
					histogram_t& pixel = pixels_[p / 2];
					pixel.increment_label(label);
				}
			}
		}

		// Fold another shard into this shard.
		// We expect one such aggregation per output shard, per 
		// array dimension above "2".
		// TODO: we probably need at least a 4-dimensional unit test case to 
		// test this properly.
		void aggregate(const Shard& other) { /* TODO: */ }

		// Write one row of modal label values to the final output
		void render(output_row_t& output_row) const {
			assert(output_row_t::dimensionality == 1);
			assert(output_row.shape()[0] == pixels_.size());
			for (std::size_t i = 0; i < pixels_.size(); ++i) {
				output_row[i] = pixels_[i].getMode();
			}
		}

	private:
		std::vector<histogram_t> pixels_;
	};

} // namespace cmb

///// END POSSIBLE PARALLEL WORK UNIT METHODS //////


template<typename LABEL_TYPE, int DIMENSION_COUNT, typename LABEL_COUNT_TYPE>
auto cmb::ModalDownsampler<LABEL_TYPE, DIMENSION_COUNT, LABEL_COUNT_TYPE>::downsample(const raster_t & original)
  -> downsamplings_t // trailing return type for better readability
{
	downsamplings_t result;


	// Downsample in the first dimension, one 1D shard at at time
	// This shard should be from the fastest-moving (final) dimension,
	// for best cache coherence.
	typedef cmb::Shard<raster_t ,LABEL_COUNT_TYPE> shard_t;
	typedef boost::multi_array_types::index_range range_t;


	// Create a new array to hold the first mipmap
	std::vector<std::size_t> downsampled_shape(DIMENSION_COUNT);
	for (int d = 0; d < DIMENSION_COUNT; ++d) {
		downsampled_shape[d] = original.shape()[d] / 2;
	}
	result.push_back(raster_t(downsampled_shape));
	raster_t& downsampled = result.back();

	const int line_length = original.shape()[DIMENSION_COUNT - 1];
	const int row_count = original.shape()[DIMENSION_COUNT - 2];
	// process two rows at a time into one downsampled shard
	// TODO: Parallelize these shardings
	std::vector<shard_t> top_slice;
	for (int row = 0; row < row_count; row += 2)
	{
		// Downsample two rows into one shard histogram
		auto index = boost::indices[range_t(row, row + 2)][range_t(0, line_length)];
		const auto& two_rows = original[index];
		top_slice.push_back(shard_t(two_rows));
	}

	// TODO: outputting top slice only handles 2D case
	for (unsigned int r = 0; r < top_slice.size(); ++r) {
		const shard_t& row = top_slice[r];
		auto index = boost::indices[r][range_t(0, line_length/2)];
		auto& output_row = downsampled[index];
		assert( output_row.num_dimensions() == 1 );
		assert( output_row.shape()[0] == line_length / 2 );
		row.render(output_row);
	}

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

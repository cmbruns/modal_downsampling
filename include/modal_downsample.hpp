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
#include <unordered_map>

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


	// New way below?

	/* 
		Class histogram_t stores the counts of all label values
		observed in a particular downsampled image pixel.
	 */
	template<typename LABEL_TYPE>
	class histogram_t
	{
	public:
		typedef LABEL_TYPE label_t;
		typedef std::size_t count_t;

		// parameter "initial_size" set the number of buckets in the hash.
		// this should be set to the expected number of distinct labels at
		// this particular downsampling, to avoid frequent rehashing.
		histogram_t(std::size_t initial_size = 2)
			: map_(initial_size)
			, cached_mode_count(0)
			, cached_mode_label(0)
		{}

		const label_t& get_mode() const {return cached_mode_label;}

		// Increment raw label value counts
		void agglomerate_scalar(const label_t& label, const count_t& inc = 1) {
			map_[label] += inc;
			// cache mode value, so we can access mode in constant time
			const count_t& c = map_[label];
			if (c > cached_mode_count) {
				cached_mode_count = c;
				cached_mode_label = label;
			}
		}

		// Agglomerate another histogram into this one
		void agglomerate_scalar(const histogram_t& rhs) {
			for (auto entry : rhs.map_) {
				const label_t& label = entry.first;
				const count_t& count = entry.second;
				agglomerate_scalar(label, count);
			}
		}

	private:
		std::unordered_map<label_t, count_t> map_;
		count_t cached_mode_count;
		label_t cached_mode_label;
	};


	/// Agglomeration: Combine two images into one ///

	// Zero dimensional kernel (just numbers -> histogram)
	template<typename AGGLOMERATED_T, typename SOURCE_T>
	void agglomerate_scalar(AGGLOMERATED_T& result, const SOURCE_T& lhs, const SOURCE_T& rhs)
	{
		result.agglomerate_scalar(lhs);
		result.agglomerate_scalar(rhs);
	}

	// Agglomerate two n-dimensional arrays.
	// This method is recursive on the number of dimensions.
	// When the number of dimensions gets to zero, it automatically reverts
	// to the zero-dimensional downsample kernel (above).
	// 1) convert raw values to histograms
	template<typename LABEL_TYPE, int DIMENSION_COUNT>
	void agglomerate_array(
		boost::multi_array<histogram_t<LABEL_TYPE>, DIMENSION_COUNT>& result,
		const boost::multi_array<LABEL_TYPE, DIMENSION_COUNT>& lhs,
		const boost::multi_array<LABEL_TYPE, DIMENSION_COUNT>& rhs)
	{
		for (std::size_t i = 0; i < result.size(); ++i) {
			agglomerate_array(result[i], lhs[i], rhs[i]);
		}
	}
	// 2) agglomerate histograms
	template<typename LABEL_TYPE, int DIMENSION_COUNT>
	void agglomerate_array(
		boost::multi_array<histogram_t<LABEL_TYPE>, DIMENSION_COUNT>& result,
		const boost::multi_array<histogram_t<LABEL_TYPE>, DIMENSION_COUNT>& lhs,
		const boost::multi_array<histogram_t<LABEL_TYPE>, DIMENSION_COUNT>& rhs)
	{
		assert(lhs.size() == result.size());
		assert(rhs.size() == result.size());
		for (std::size_t i = 0; i < result.size(); ++i) {
			agglomerate_array(result[i], lhs[i], rhs[i]);
		}
	}

	// 1-D specialization
	template<typename LABEL_TYPE>
	void agglomerate_array(
		boost::multi_array<histogram_t<LABEL_TYPE>, 1>& result,
		const boost::multi_array<LABEL_TYPE, 1>& lhs,
		const boost::multi_array<LABEL_TYPE, 1>& rhs)
	{
		for (std::size_t i = 0; i < result.size(); ++i) {
			agglomerate_scalar(result[i], lhs[i], rhs[i]);
		}
	}
	// 2) agglomerate histograms
	template<typename LABEL_TYPE>
	void agglomerate_array(
		boost::multi_array<histogram_t<LABEL_TYPE>, 1>& result,
		const boost::multi_array<histogram_t<LABEL_TYPE>, 1>& lhs,
		const boost::multi_array<histogram_t<LABEL_TYPE>, 1>& rhs)
	{
		assert(lhs.size() == result.size());
		assert(rhs.size() == result.size());
		for (std::size_t i = 0; i < result.size(); ++i) {
			agglomerate_scalar(result[i], lhs[i], rhs[i]);
		}
	}

	// General downsampling for dimensions 2 and higher
	template<typename LABEL_TYPE, int DIMENSION_COUNT>
	void downsample(
		boost::multi_array<histogram_t<LABEL_TYPE>, DIMENSION_COUNT>& result,
		const boost::multi_array<LABEL_TYPE, DIMENSION_COUNT>& original)
	{
		assert(original.size() == 2 * result.size());

		// Create two intermediate downsampled subarrays
		auto slice1 = result[0];
		auto slice2 = result[0];
		histogram_t<LABEL_TYPE> empty_histogram;

		for (std::size_t i = 0; i < result.size(); ++i)
		{
			// Clear intermediate data structures
			std::fill(slice1.begin(), slice1.end(), empty_histogram);
			std::fill(slice2.begin(), slice2.end(), empty_histogram);

			// Downsample subfields
			downsample(slice1, original[2 * i]);
			downsample(slice2, original[2 * i + 1]);

			agglomerate_scalar(result[i], slice1, slice2);
		}
	}

	// 1-dimensional specialization
	template<typename LABEL_TYPE>
	void downsample(
		boost::multi_array<histogram_t<LABEL_TYPE>, 1>& result,
		const boost::multi_array<LABEL_TYPE, 1>& original)
	{
		assert(original.size() == 2 * result.size());

		for (std::size_t i = 0; i < result.size(); ++i)
			agglomerate_scalar(result[i], original[2*i], original[2*i+1]);
	}


	/// DOWNSAMPLING: Convert larger arrays to smaller ones

	// n-dimensional general method
	// wrapped in a class so we could do partial specialization, below
	template<typename RESULT_ARRAY_TYPE, typename ORIGINAL_ARRAY_TYPE, int DIMENSIONALITY>
	struct ArrayDownsampler {
		static void downsample_array(RESULT_ARRAY_TYPE& result, const ORIGINAL_ARRAY_TYPE& original)
		{
			assert(RESULT_ARRAY_TYPE::dimensionality == DIMENSIONALITY);
			assert(ORIGINAL_ARRAY_TYPE::dimensionality == DIMENSIONALITY);
			assert(original.size() == 2 * result.size());
			typedef typename RESULT_ARRAY_TYPE::reference result_subarray_t;
			typedef typename ORIGINAL_ARRAY_TYPE::value_type original_subarray_t;
			for (std::size_t i = 0; i < result.size(); ++i) {
				ArrayDownsampler<
					result_subarray_t,
					original_subarray_t,
					DIMENSIONALITY - 1>
					::downsample_array(result[i], original[2*i]);
				ArrayDownsampler<
					result_subarray_t,
					original_subarray_t, 
					DIMENSIONALITY - 1>
					::downsample_array(result[i], original[2*i + 1]);
			}
		}
	};

	// 1-dimensional specialization
	template<typename RESULT_ARRAY_TYPE, typename ORIGINAL_ARRAY_TYPE>
	struct ArrayDownsampler<RESULT_ARRAY_TYPE, ORIGINAL_ARRAY_TYPE, 1> 
	{
		static void downsample_array(RESULT_ARRAY_TYPE& result, const ORIGINAL_ARRAY_TYPE& original)
		{
			assert(RESULT_ARRAY_TYPE::dimensionality == 1);
			assert(ORIGINAL_ARRAY_TYPE::dimensionality == 1);
			assert(original.size() == 2 * result.size());
			for (std::size_t i = 0; i < result.size(); ++i) {
				agglomerate_scalar(result[i], original[2 * i], original[2 * i + 1]);
			}
		}
	};

	// Easier-to-call delegate to ArrayDownsampler class
	template<typename RESULT_ARRAY_TYPE, typename ORIGINAL_ARRAY_TYPE>
	void downsample_array(RESULT_ARRAY_TYPE& result, const ORIGINAL_ARRAY_TYPE& original)
	{
		ArrayDownsampler<RESULT_ARRAY_TYPE, ORIGINAL_ARRAY_TYPE, ORIGINAL_ARRAY_TYPE::dimensionality>::downsample_array(result, original);
	}


	/// RENDERING: Convert histograms back to raw labels ///

	// n-dimensional general method
	// wrapped in a class so we could do partial specialization, below
	template<typename RESULT_ARRAY_TYPE, typename ORIGINAL_ARRAY_TYPE, int DIMENSIONALITY>
	struct ArrayRenderer {
		static void render_array(RESULT_ARRAY_TYPE& result, const ORIGINAL_ARRAY_TYPE& original)
		{
			assert(RESULT_ARRAY_TYPE::dimensionality == DIMENSIONALITY);
			assert(ORIGINAL_ARRAY_TYPE::dimensionality == DIMENSIONALITY);
			assert(original.size() == result.size());
			typedef typename RESULT_ARRAY_TYPE::reference result_subarray_t;
			typedef typename ORIGINAL_ARRAY_TYPE::value_type original_subarray_t;
			for (std::size_t i = 0; i < result.size(); ++i) {
				ArrayRenderer<
					result_subarray_t,
					original_subarray_t,
					DIMENSIONALITY - 1>
					::render_array(result[i], original[i]);
			}
		}
	};

	// 1-dimensional specialization
	template<typename RESULT_ARRAY_TYPE, typename ORIGINAL_ARRAY_TYPE>
	struct ArrayRenderer<RESULT_ARRAY_TYPE, ORIGINAL_ARRAY_TYPE, 1>
	{
		static void render_array(RESULT_ARRAY_TYPE& result, const ORIGINAL_ARRAY_TYPE& original)
		{
			assert(RESULT_ARRAY_TYPE::dimensionality == 1);
			assert(ORIGINAL_ARRAY_TYPE::dimensionality == 1);
			assert(original.size() == result.size());
			for (std::size_t i = 0; i < result.size(); ++i) {
				result[i] = original[i].get_mode();
			}
		}
	};

	// Easier-to-call delegate to ArrayRenderer class
	template<typename RESULT_ARRAY_TYPE, typename ORIGINAL_ARRAY_TYPE>
	void render_array(RESULT_ARRAY_TYPE& result, const ORIGINAL_ARRAY_TYPE& original)
	{
		ArrayRenderer<RESULT_ARRAY_TYPE, ORIGINAL_ARRAY_TYPE, ORIGINAL_ARRAY_TYPE::dimensionality>::render_array(result, original);
	}

} // namespace cmb

#endif // CMB_MODAL_DOWNSAMPLE_HPP_

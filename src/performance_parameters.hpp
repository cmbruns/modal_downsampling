#ifndef CMB_PERFORMANCE_PARAMETERS_HPP_
#define CMB_PERFORMANCE_PARAMETERS_HPP_

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

// Other things to test:
// * std::unordered_map vs std::map
// * number of shards per parallel work unit

// These parameters should be tested for very large images,
// including images with very high spatial coherence (maybe just one label)
// plus images with very low spatial coherence (maybe random distinct labels, with one duplicate)

#endif // CMB_PERFORMANCE_PARAMETERS_HPP_
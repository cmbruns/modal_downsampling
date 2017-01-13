
/*
Example 1. The d = 2, L1 = 2, L2 = 3 image

    11111111
    12121212
    11222222
    12222222

yields the 1-downsampled image:

    1111
    1222

and the 2-downsampled image:

    12

Note that 11 is the wrong answer because mode-downsampling is based on blocks of
the original image, rather than the previous downsampled image. The ?rst value (1) of the
2-downsampled image is the mode of this block of the original image:

    1111
    1212
    1122
    1222

The second value (2) is the mode of this block of the original image:

    1111
    1212
    2222
    2222

*/

#define BOOST_TEST_MODULE BlockDownsampleExample1

//VERY IMPORTANT - include this last
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE( positive_control )
{
    BOOST_CHECK(true);
}


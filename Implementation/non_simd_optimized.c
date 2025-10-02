
#include <math.h>
#include <stdint.h>
#include <stdio.h>

// Precomputed table of the squares of the values from zero to 255, to
// efficiently compute the standard deviation
static const uint16_t squares[256] = {
    0,     1,     4,     9,     16,    25,    36,    49,    64,    81,    100,   121,   144,
    169,   196,   225,   256,   289,   324,   361,   400,   441,   484,   529,   576,   625,
    676,   729,   784,   841,   900,   961,   1024,  1089,  1156,  1225,  1296,  1369,  1444,
    1521,  1600,  1681,  1764,  1849,  1936,  2025,  2116,  2209,  2304,  2401,  2500,  2601,
    2704,  2809,  2916,  3025,  3136,  3249,  3364,  3481,  3600,  3721,  3844,  3969,  4096,
    4225,  4356,  4489,  4624,  4761,  4900,  5041,  5184,  5329,  5476,  5625,  5776,  5929,
    6084,  6241,  6400,  6561,  6724,  6889,  7056,  7225,  7396,  7569,  7744,  7921,  8100,
    8281,  8464,  8649,  8836,  9025,  9216,  9409,  9604,  9801,  10000, 10201, 10404, 10609,
    10816, 11025, 11236, 11449, 11664, 11881, 12100, 12321, 12544, 12769, 12996, 13225, 13456,
    13689, 13924, 14161, 14400, 14641, 14884, 15129, 15376, 15625, 15876, 16129, 16384, 16641,
    16900, 17161, 17424, 17689, 17956, 18225, 18496, 18769, 19044, 19321, 19600, 19881, 20164,
    20449, 20736, 21025, 21316, 21609, 21904, 22201, 22500, 22801, 23104, 23409, 23716, 24025,
    24336, 24649, 24964, 25281, 25600, 25921, 26244, 26569, 26896, 27225, 27556, 27889, 28224,
    28561, 28900, 29241, 29584, 29929, 30276, 30625, 30976, 31329, 31684, 32041, 32400, 32761,
    33124, 33489, 33856, 34225, 34596, 34969, 35344, 35721, 36100, 36481, 36864, 37249, 37636,
    38025, 38416, 38809, 39204, 39601, 40000, 40401, 40804, 41209, 41616, 42025, 42436, 42849,
    43264, 43681, 44100, 44521, 44944, 45369, 45796, 46225, 46656, 47089, 47524, 47961, 48400,
    48841, 49284, 49729, 50176, 50625, 51076, 51529, 51984, 52441, 52900, 53361, 53824, 54289,
    54756, 55225, 55696, 56169, 56644, 57121, 57600, 58081, 58564, 59049, 59536, 60025, 60516,
    61009, 61504, 62001, 62500, 63001, 63504, 64009, 64516, 65025};

/**
 * @brief  This method does the contrast adjustment.
 * It is needed to perform this separately from the other computations, because
 * the contrast adjustment needs the standard deviation and the average over all
 * pixel, that needs to be computed before. So we have no performance loss by
 * calculating it in an extra function.
 *
 * @param input_image A pointer to the pixeldata
 * @param width The width of the picture
 * @param height The height of the picture
 * @param contrast The given constant that is used for the contrast adjustment
 * @param sigma The calculated standard deviation over all pixel after the
 * greyscaling and the adjustment of the brightness
 * @param mu The calculated average over all pixel after the greyscaling and the
 * adjustment of the brightness
 */
static void adjust_contrast (uint8_t* input_image,
                             size_t img_size,
                             float k,
                             double standard_deviation,
                             double average)
{
    double k_div_sigma = k / (double) standard_deviation;

    // If the standard deviation comes near to zero, k_div_sigma results in
    // infinity (if k != 0) or in NaN (if k = 0) In this case every pixels value
    // of the output image is set to the average of all pixels in the input
    // image
    if (isinf (k_div_sigma) || isnan (k_div_sigma))
    {
        uint8_t rounded_average = round (average);
        for (size_t i = 0; i < img_size; ++i)
        {
            input_image[i] = rounded_average;
        }
        return;
    }

    // extract exponent and significand of k divided by sigma (k/sigma)
    // and transform significand from double to 12.52 fixed-point
    int k_div_sigma_exponent;
    int64_t k_div_sigma_fix_point_significand =
        frexp (k_div_sigma, &k_div_sigma_exponent) * 0x0010000000000000;

    // compute the exponent of k divided by sigma as fixed point
    int16_t k_div_sigma_fixed_point_exponent = k_div_sigma_exponent - 52;

    // Precompute (1- (k/sigma)*mu with normalizing values with 1/256 to prevent
    // double overflow
    double precomputed = (((1 / (double) 256) - (k_div_sigma / 256)) * average);

    // isolate the exponent and significand
    int precomputed_exponent;
    int64_t precomputed_significand =
        frexp (precomputed, &precomputed_exponent) * 0x0010000000000000;

    // calculate fixed point exponent and denormalize it by adding 8 to the
    // exponent (multiply with 256)
    int16_t precomputed_fix_point_exponent = precomputed_exponent - 52 + 8;

    // bring the precomputed fixed point number and k/sigma as fixed point
    // number to the same exponent
    if (precomputed_fix_point_exponent < k_div_sigma_fixed_point_exponent)
    {
        precomputed_significand = precomputed_significand >> ((k_div_sigma_fixed_point_exponent)
                                                              - (precomputed_fix_point_exponent));
    }
    else if (precomputed_fix_point_exponent > k_div_sigma_fixed_point_exponent)
    {
        k_div_sigma_fix_point_significand =
            k_div_sigma_fix_point_significand
            >> (precomputed_fix_point_exponent - k_div_sigma_fixed_point_exponent);
        k_div_sigma_fixed_point_exponent = precomputed_fix_point_exponent;
    }

    // rounding mask for efficient rounding by applying bitmask to isolate the
    // first digit behind the point (in binary), shift it to the right, so the
    // isolated bit is at position zero. Then add it to the result which cuts of
    // by shifting
    uint64_t rounding_mask;
    uint8_t shift;
    if (k_div_sigma_fixed_point_exponent < 0)
    {
        shift         = -k_div_sigma_fixed_point_exponent - 1;
        rounding_mask = exp2 (shift);
    }
    else
    {
        shift         = 0;
        rounding_mask = 0;
    }

    // loop to compute pixel values
    for (size_t i = 0; i < img_size; i++)
    {
        // calculate fixed-point unclamped values
        int64_t unclamped_fix =
            input_image[i] * k_div_sigma_fix_point_significand + precomputed_significand;

        // convert to integer by shifting and rounding
        int64_t unclamped = ((unclamped_fix) >> (-k_div_sigma_fixed_point_exponent))
                            + ((unclamped_fix & rounding_mask) >> shift);

        if (unclamped > 255)
        {
            input_image[i] = 255;
        }
        else if (unclamped < 0)
        {
            input_image[i] = 0;
        }
        else
        {
            input_image[i] = unclamped;
        }
    }
}
/**
 * @brief  This method is greyscaling an ppm image and adjusting the brightness
 * and the contrast by given constants
 *
 * @param img A pointer to the colored pixeldata
 * @param width The width of the picture
 * @param height The height of the picture
 * @param a The weight for the red value
 * @param b The weight for the green value
 * @param c The weight for the blue value
 * @param brightness The brightness value by which the pixels are adjusted
 * @param contrast The given constant that is used for the contrast adjustment
 * @param input_image A pointer to the greyscaled and adjusted pixeldata
 */
void brightness_contrast_V3 (const uint8_t* img,
                             size_t width,
                             size_t height,
                             float a,
                             float b,
                             float c,
                             int16_t brightness,
                             float contrast,
                             uint8_t* result)
{
    // catch a,b,c equals zero to prevent division by zero in to_greyscale()
    if (a == 0 && b == 0 && c == 0)
    {
        if (brightness < 0)
            brightness = 0;
        for (size_t i = 0; i < width * height; ++i)
        {
            result[i] = brightness;
        }
        return;
    }

    // cast a,b,c to double to prevent overflow
    double sum_of_weights = ((double) a) + ((double) b) + ((double) c);

    // normalize a,b,c to a+b+c = 1 and transform them into 12.52 fixed point
    // numbers
    uint64_t normalized_a = (a / sum_of_weights) * 0x0010000000000000;
    uint64_t normalized_b = (b / sum_of_weights) * 0x0010000000000000;
    uint64_t normalized_c = (c / sum_of_weights) * 0x0010000000000000;

    size_t img_size_color = (width * height * 3);
    uint64_t sum          = 0;
    uint64_t sum_squares  = 0;
    double standard_deviation;
    double average;
    int16_t unclamped;

    // pixel value computation loop
    uint64_t n = 0;
    for (size_t i = 0; i < img_size_color; i += 3)
    {
        // calculate greyscale without division because the values are already
        // correctly normalized
        uint64_t fixed_point_unclamped =
            (normalized_a * img[i] + normalized_b * img[i + 1] + normalized_c * img[i + 2]);

        // convert fixed point to integer values and adjust brightness. Round by
        // isolating and shifting the first digit behind the point to position
        // zero and then adding it to the result which cut the digits behind the
        // point
        unclamped = (fixed_point_unclamped >> 52)
                    + ((fixed_point_unclamped & 0x0008000000000000) >> 51) + brightness;

        // clamp, write to pixel array and add the values and their square to
        // the sum accumulators, for computing standard deviation and average
        if (unclamped > 255)
        {
            result[n++] = 255;
            sum += 255;
            sum_squares += 65025;
        }
        else if (unclamped < 0)
        {
            result[n++] = 0;
        }
        else
        {
            result[n++] = unclamped;
            sum += unclamped;
            sum_squares += squares[unclamped];
        }
    }

    // calculate average and standard deviation according to the optimized
    // formula (have a look on the presentation slides for details)
    average            = sum / (double) n;
    standard_deviation = sqrt ((sum_squares - (2 * average * sum) + (n * average * average)) / n);

    // adjust the contrast
    adjust_contrast (result, width * height, contrast, standard_deviation, average);
}

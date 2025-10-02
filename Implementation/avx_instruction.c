#include <emmintrin.h>
#include <immintrin.h>
#include <math.h>
#include <smmintrin.h>
#include <stdint.h>
#include <stdio.h>
#include <tmmintrin.h>

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
static void adjust_contrast_avx (uint8_t* input_image,
                                 size_t width,
                                 size_t height,
                                 float contrast,
                                 float sigma,
                                 float mu)
{
    // We do not adjust the contrast if sigma is zero. If sigma is zero then all
    // pixel have the same value. It is impossible to adjust a contrast on a
    // picture without existing contrast, that's why we decided to just ignore
    // the contrast adjustment in this case.
    if (sigma == 0.0)
    {
        return;
    }

    // General Declarations
    uint64_t size = width * height;
    uint64_t n    = 0;

    // Precomputing all parts of the given formula, that are independent from
    // the pixel value, because these values/ymm-register(with values) are the
    // same for every pixel and we save time with not calculating them every
    // time again.
    double k_divided_by_sigma = contrast / sigma;

    // If the standard deviation comes near to zero, k_div_sigma results in
    // infinity. In this case every pixels value
    // of the output image is set to the average of all pixels in the input
    // image
    if (isinf (k_divided_by_sigma))
    {
        __m256i mu_vec = _mm256_set1_epi8 (round (mu));
        if (size > 31)
        {
            while (n < size - 31)
            {
                _mm256_storeu_si256 ((__m256i*) &input_image[n], mu_vec);
                n += 32;
            }
        }
        while (n < size)
        {
            input_image[n] = round (mu);
            n++;
        }
        return;
    }
    // Precomputing
    double one_minus_k_divided_by_sigma_times_mu = (1 - k_divided_by_sigma) * mu;
    __m256 k_divided_by_sigma_vec                = _mm256_set1_ps (k_divided_by_sigma);
    __m256 one_minus_k_divided_by_sigma_times_mu_vec =
        _mm256_set1_ps (one_minus_k_divided_by_sigma_times_mu);

    // The Zero ymm-register is used multiple times, that's why it´s faster to
    // precompute it.
    __m256i zero_vec = _mm256_setzero_si256 ();

    if (size >= 32)
    {
        // begin of the SIMD Computations
        while (n < size - 31)
        {
            // Loading 32 8 bit unsigned integers and putting them to 4 float
            // ymm registers where every register has 8 floats
            __m256i data = _mm256_loadu_si256 ((__m256i*) &input_image[n]);

            __m256i data_lo = _mm256_unpacklo_epi8 (data, zero_vec);
            __m256i data_hi = _mm256_unpackhi_epi8 (data, zero_vec);

            __m256i data_int_1 = _mm256_unpacklo_epi16 (data_lo, zero_vec);
            __m256i data_int_2 = _mm256_unpackhi_epi16 (data_lo, zero_vec);
            __m256i data_int_3 = _mm256_unpacklo_epi16 (data_hi, zero_vec);
            __m256i data_int_4 = _mm256_unpackhi_epi16 (data_hi, zero_vec);

            __m256 data_1 = _mm256_cvtepi32_ps (data_int_1);
            __m256 data_2 = _mm256_cvtepi32_ps (data_int_2);
            __m256 data_3 = _mm256_cvtepi32_ps (data_int_3);
            __m256 data_4 = _mm256_cvtepi32_ps (data_int_4);

            // Using the precomputed ymm register to adjust the pixel values in
            // parallel, based on the given formula (Needed for every of the 4
            // registers)
            data_1 = _mm256_mul_ps (k_divided_by_sigma_vec, data_1);
            data_2 = _mm256_mul_ps (k_divided_by_sigma_vec, data_2);
            data_3 = _mm256_mul_ps (k_divided_by_sigma_vec, data_3);
            data_4 = _mm256_mul_ps (k_divided_by_sigma_vec, data_4);

            data_1 = _mm256_add_ps (one_minus_k_divided_by_sigma_times_mu_vec, data_1);
            data_2 = _mm256_add_ps (one_minus_k_divided_by_sigma_times_mu_vec, data_2);
            data_3 = _mm256_add_ps (one_minus_k_divided_by_sigma_times_mu_vec, data_3);
            data_4 = _mm256_add_ps (one_minus_k_divided_by_sigma_times_mu_vec, data_4);

            // Converting the 4 ymm registers with floats back to one register
            // with 8 bit unsigned values and write them back to memory
            data_int_1 = _mm256_cvtps_epi32 (data_1);
            data_int_2 = _mm256_cvtps_epi32 (data_2);
            data_int_3 = _mm256_cvtps_epi32 (data_3);
            data_int_4 = _mm256_cvtps_epi32 (data_4);

            data_lo = _mm256_packus_epi32 (data_int_1, data_int_2);
            data_hi = _mm256_packus_epi32 (data_int_3, data_int_4);

            __m256i result = _mm256_packus_epi16 (data_lo, data_hi);

            _mm256_storeu_si256 ((__m256i*) &input_image[n], result);
            n += 32;
        }
    }
    // Calculating the rest of the pixels, which can't be calculated by SIMD
    while (n < size)
    {
        float data = k_divided_by_sigma * input_image[n] + one_minus_k_divided_by_sigma_times_mu;
        // Clamping if necessary
        if (data <= 0)
        {
            input_image[n] = 0;
        }
        else if (data >= 255)
        {
            input_image[n] = 255;
        }
        else
        {
            input_image[n] = round (data);
        }
        n++;
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
void brightness_contrast_V5 (const uint8_t* img,
                             size_t width,
                             size_t height,
                             float a,
                             float b,
                             float c,
                             int16_t brightness,
                             float contrast,
                             uint8_t* input_image)
{
    // General Declarations
    uint64_t size           = width * height;
    uint64_t n              = 0;
    uint64_t i              = 0;
    uint64_t sum            = 0;
    uint64_t sum_of_squares = 0;

    // Case that a=b=c=0, in which only the adjustment of the brightness makes
    // sense, because the values after greyscale are zero and after adjustment
    // of the brightness, they are clamped brightness. In this case all pixels
    // have the same value and a contrast adjustment makes no sense.
    if (a == 0 && b == 0 && c == 0)
    {
        // we write the brightness in every pixel if the brightness is
        // positiv because otherwise the clamping will put it to zero
        if (brightness > 0)
        {
            // a ymm register filled with the brightness as 8 bit unsigned value
            __m256i brightness_vec = _mm256_set1_epi8 (brightness);
            if (size >= 16)
            {
                while (n < size - 15)
                {
                    _mm256_storeu_si256 ((__m256i*) &input_image[n], brightness_vec);
                    n += 32;
                }
                while (n < size)
                {
                    input_image[n] = brightness;
                    n++;
                }
            }
        }
        else
        {
            // using a ymm register full of zeros for the clamped negative
            // brightness
            __m256i brightness_vec = _mm256_setzero_si256 ();
            if (size >= 16)
            {
                while (n < size - 15)
                {
                    _mm256_storeu_si256 ((__m256i*) &input_image[n], brightness_vec);
                    n += 32;
                }
                while (n < size)
                {
                    input_image[n] = 0;
                    n++;
                }
            }
        }
        return;
    }
    // We are normalizing the a,b and c, so that a+b+c= 1. That
    // makes it easier to add the weighted values, because the result of them is
    // between 0 and 255.
    double a_mod  = a;
    double b_mod  = b;
    double c_mod  = c;
    a             = 255 * a_mod / (a_mod + b_mod + c_mod);
    b             = 255 * b_mod / (a_mod + b_mod + c_mod);
    c             = 255 * c_mod / (a_mod + b_mod + c_mod);
    uint8_t a_dis = round (a);
    uint8_t b_dis = round (b);
    uint8_t c_dis = round (c);
    // It is possible that the sum is 256 (caused by rounding), which is caught
    // here by subtracting one from the highest
    if ((uint16_t) a_dis + (uint16_t) b_dis + (uint16_t) c_dis > 255)
    {
        if (a_dis > b_dis)
        {
            if (a_dis > c_dis)
            {
                a_dis -= 1;
            }
            else
            {
                c_dis -= 1;
            }
        }
        else
        {
            if (b_dis > c_dis)
            {
                b_dis -= 1;
            }
            else
            {
                c_dis -= 1;
            }
        }
    }

    // Precalculation of masks that are made for reordering the data. We're
    // reading the 96 bytes and reorder them so that the first register has only
    // red values the second only green values and the third only blue values.
    // And also the values on the same place in the register are the
    // corresponding ones. The masks are not intuitiv ordered because in that
    // way we're saving registers by reusing some masks.
    __m128i mask_a_11 = _mm_set_epi8 (-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0);
    __m128i mask_a_12 = _mm_set_epi8 (-1, -1, -1, -1, -1, 14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1);
    __m128i mask_a_13 = _mm_set_epi8 (13, 10, 7, 4, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m128i mask_b_11 = _mm_set_epi8 (-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1);
    __m128i mask_b_12 = _mm_set_epi8 (-1, -1, -1, -1, -1, 15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1);
    __m128i mask_b_13 = _mm_set_epi8 (14, 11, 8, 5, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m128i mask_c_11 = _mm_set_epi8 (-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 14, 11, 8, 5, 2);
    __m128i mask_c_12 = _mm_set_epi8 (-1, -1, -1, -1, -1, -1, 13, 10, 7, 4, 1, -1, -1, -1, -1, -1);
    __m128i mask_c_13 = _mm_set_epi8 (15, 12, 9, 6, 3, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    // these are 16 bit ymm registers, every register filled with all a, b or c
    // values
    __m256i a_vec = _mm256_set1_epi16 (a_dis);
    __m256i b_vec = _mm256_set1_epi16 (b_dis);
    __m256i c_vec = _mm256_set1_epi16 (c_dis);

    // In this case distinction we check, if the brightness is positive or
    // negative. By this we only have to save the absolute
    // of the values and then either add or subtract. That gives us one bit more
    // for information because of the missing sign bit.

    if (brightness > 0)
    {
        // we store it as unsigned 16 bit value, because that needs less space
        // than a signed value
        __m256i brightness_vec = _mm256_set1_epi16 (256 * brightness);

        // The following code-block is in a very similar way two times in this
        // code, because in that way we can exclude an if statement from loop,
        // what gives us better performance. The first time we're calculating
        // with a positiv brightness, what needs an addition from pixel value
        // and brightness. In the other case the brightness is negativ, what
        // needs a subtraction of the brightness from the pixel value
        if (size >= 32)
        {
            while (n < size - 31)
            {
                // reading the data in blocks of 16 8 bit unsigned integern
                __m128i load_rgb_1 = _mm_loadu_si128 ((__m128i*) &img[i]);
                __m128i load_rgb_2 = _mm_loadu_si128 ((__m128i*) &img[i + 16]);
                __m128i load_rgb_3 = _mm_loadu_si128 ((__m128i*) &img[i + 32]);
                __m128i load_rgb_4 = _mm_loadu_si128 ((__m128i*) &img[i + 48]);
                __m128i load_rgb_5 = _mm_loadu_si128 ((__m128i*) &img[i + 64]);
                __m128i load_rgb_6 = _mm_loadu_si128 ((__m128i*) &img[i + 80]);

                // Converting the data in 16 bit ymm register that only have
                // red, green, or blue values. The values which belong to each
                // other are at the same position of the ymm register, but they
                // are not ordered like the data is in the beginning, because
                // were saving overhead with that.
                __m256i store_a_1 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_a_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_a_12),
                                                _mm_shuffle_epi8 (load_rgb_3, mask_a_13))));
                __m256i store_a_2 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_4, mask_a_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_5, mask_a_12),
                                                _mm_shuffle_epi8 (load_rgb_6, mask_a_13))));

                __m256i store_b_1 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_b_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_b_12),
                                                _mm_shuffle_epi8 (load_rgb_3, mask_b_13))));
                __m256i store_b_2 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_4, mask_b_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_5, mask_b_12),
                                                _mm_shuffle_epi8 (load_rgb_6, mask_b_13))));

                __m256i store_c_1 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_c_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_c_12),
                                                _mm_shuffle_epi8 (load_rgb_3, mask_c_13))));
                __m256i store_c_2 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_4, mask_c_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_5, mask_c_12),
                                                _mm_shuffle_epi8 (load_rgb_6, mask_c_13))));

                // Multiplying the rgb values with the normalized values of
                // a,b,c. That's the reason why we chose the 16 bit values
                // because the product of two 8 bit values has a maximum of 16
                // bits.

                store_a_1 = _mm256_mullo_epi16 (store_a_1, a_vec);
                store_a_2 = _mm256_mullo_epi16 (store_a_2, a_vec);
                store_b_1 = _mm256_mullo_epi16 (store_b_1, b_vec);
                store_b_2 = _mm256_mullo_epi16 (store_b_2, b_vec);
                store_c_1 = _mm256_mullo_epi16 (store_c_1, c_vec);
                store_c_2 = _mm256_mullo_epi16 (store_c_2, c_vec);

                // Now we can add the values r, g and b, that are weighted in a
                // way that the sum is a value between 0 and 255
                __m256i store_1 =
                    _mm256_adds_epu16 (store_a_1, _mm256_adds_epu16 (store_b_1, store_c_1));
                __m256i store_2 =
                    _mm256_adds_epu16 (store_a_2, _mm256_adds_epu16 (store_b_2, store_c_2));

                // Adjusting brightness by adding the positive brightness to the
                // pixel values. The operation does automate clamping
                store_1 = _mm256_adds_epu16 (store_1, brightness_vec);
                store_2 = _mm256_adds_epu16 (store_2, brightness_vec);

                // We're making 8 bit values out of 16 bit, so that we can
                // square them in the next step
                store_1 = _mm256_srli_epi16 (store_1, 8);

                store_2 = _mm256_srli_epi16 (store_2, 8);

                // calculating the squares of the values for the standard
                // deviation
                __m256i square_1 = _mm256_mullo_epi16 (store_1, store_1);
                __m256i square_2 = _mm256_mullo_epi16 (store_2, store_2);

                // Adding the squares of the values by casting them to a 32 bit
                // integer and performing an addition of all values in the ymm
                // register
                __m256i square_12 = _mm256_unpacklo_epi16 (square_1, _mm256_setzero_si256 ());
                square_1          = _mm256_unpackhi_epi16 (square_1, _mm256_setzero_si256 ());
                __m256i square_22 = _mm256_unpacklo_epi16 (square_2, _mm256_setzero_si256 ());
                square_2          = _mm256_unpackhi_epi16 (square_2, _mm256_setzero_si256 ());

                square_12           = _mm256_add_epi32 (_mm256_add_epi32 (square_1, square_12),
                                              _mm256_add_epi32 (square_2, square_22));
                __m128i sum_128vecl = _mm256_extracti128_si256 (square_12, 0);
                __m128i sum_128vech = _mm256_extracti128_si256 (square_12, 1);
                __m128i sum_vec     = _mm_add_epi32 (sum_128vecl, sum_128vech);
                sum_vec             = _mm_hadd_epi32 (sum_vec, sum_vec);
                sum_vec             = _mm_hadd_epi32 (sum_vec, sum_vec);
                sum_of_squares += _mm_extract_epi32 (sum_vec, 0);

                // Calculating sum of all pixel values for the standard
                // deviation
                __m256i sum_vec256 = _mm256_adds_epi16 (store_1, store_2);
                __m128i low        = _mm256_extracti128_si256 (sum_vec256, 0);
                __m128i high       = _mm256_extracti128_si256 (sum_vec256, 1);
                __m128i sum_128vec = _mm_add_epi16 (low, high);
                sum_128vec         = _mm_hadds_epi16 (sum_128vec, sum_128vec);
                sum_128vec         = _mm_hadds_epi16 (sum_128vec, sum_128vec);
                sum_128vec         = _mm_hadds_epi16 (sum_128vec, sum_128vec);
                sum += _mm_extract_epi16 (sum_128vec, 0);

                // Convert everything back to one ymm register with 8 bit values
                __m256i result = _mm256_packus_epi16 (store_1, store_2);
                // Reordering the entries
                result = _mm256_permute4x64_epi64 (result, 0b11011000);

                _mm256_storeu_si256 ((__m256i*) &input_image[n], result);

                n += 32;
                i += 96;
            }
        }
        // Calculating the rest, that is not possible to parallelize
        while (n < size)
        {
            uint8_t data = (a_dis * img[i] + b_dis * img[i + 1] + c_dis * img[i + 2]) / 256;

            // Clamping
            if (brightness > 255 - data)
            {
                data = 255;
            }
            else
            {
                data += brightness;
            }
            // Calculating the sums for the standard deviation
            sum += data;
            sum_of_squares += data * data;
            input_image[n] = data;
            n++;
            i += 3;
        }
    }
    else
    {
        // In this case we're subtracting the absolute of the brightness instead
        // of adding the negative brightness That is the precomputed
        // ymm-resister with the absolute of the brightness, that is shifted to
        // the first 8 bit of the 16 bit integer. That makes the automated
        // clamping easier.
        __m256i brightness_vec = _mm256_set1_epi16 (256 * -brightness);

        // This block is the duplicate of the one above, with the little
        // difference, that this one subtracts the absolute of the brightness
        // and do not add it.
        if (size >= 32)
        {
            while (n < size - 31)
            {
                // reading the data in blocks of 16 8 bit unsigned integern
                __m128i load_rgb_1 = _mm_loadu_si128 ((__m128i*) &img[i]);
                __m128i load_rgb_2 = _mm_loadu_si128 ((__m128i*) &img[i + 16]);
                __m128i load_rgb_3 = _mm_loadu_si128 ((__m128i*) &img[i + 32]);
                __m128i load_rgb_4 = _mm_loadu_si128 ((__m128i*) &img[i + 48]);
                __m128i load_rgb_5 = _mm_loadu_si128 ((__m128i*) &img[i + 64]);
                __m128i load_rgb_6 = _mm_loadu_si128 ((__m128i*) &img[i + 80]);

                // Converting the data in 16 bit ymm register that only have
                // red, green, or blue values. The values which belong to each
                // other are at the same position of the ymm register, but they
                // are not ordered like the data is in the beginning, because
                // were saving overhead with that.
                __m256i store_a_1 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_a_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_a_12),
                                                _mm_shuffle_epi8 (load_rgb_3, mask_a_13))));
                __m256i store_a_2 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_4, mask_a_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_5, mask_a_12),
                                                _mm_shuffle_epi8 (load_rgb_6, mask_a_13))));
                __m256i store_b_1 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_b_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_b_12),
                                                _mm_shuffle_epi8 (load_rgb_3, mask_b_13))));
                __m256i store_b_2 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_4, mask_b_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_5, mask_b_12),
                                                _mm_shuffle_epi8 (load_rgb_6, mask_b_13))));
                __m256i store_c_1 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_c_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_c_12),
                                                _mm_shuffle_epi8 (load_rgb_3, mask_c_13))));
                __m256i store_c_2 = _mm256_cvtepu8_epi16 (
                    _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_4, mask_c_11),
                                  _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_5, mask_c_12),
                                                _mm_shuffle_epi8 (load_rgb_6, mask_c_13))));

                // Multiplying the rgb values with the normalized values of
                // a,b,c. That's the reason why we chose the 16 bit values
                // because the product of two 8 bit values has a maximum of 16
                // bits.
                store_a_1 = _mm256_mullo_epi16 (store_a_1, a_vec);
                store_a_2 = _mm256_mullo_epi16 (store_a_2, a_vec);
                store_b_1 = _mm256_mullo_epi16 (store_b_1, b_vec);
                store_b_2 = _mm256_mullo_epi16 (store_b_2, b_vec);
                store_c_1 = _mm256_mullo_epi16 (store_c_1, c_vec);
                store_c_2 = _mm256_mullo_epi16 (store_c_2, c_vec);

                // Now we can add the values r, g and b, that are weighted in a
                // way that the sum is a value between 0 and 255
                __m256i store_1 =
                    _mm256_adds_epu16 (store_a_1, _mm256_adds_epu16 (store_b_1, store_c_1));
                __m256i store_2 =
                    _mm256_adds_epu16 (store_a_2, _mm256_adds_epu16 (store_b_2, store_c_2));

                // Adjusting brightness by subtracting the positive brightness
                // to the pixel values. The operation does automate clamping
                store_1 = _mm256_subs_epu16 (store_1, brightness_vec);
                store_2 = _mm256_subs_epu16 (store_2, brightness_vec);

                // We're making 8 bit values out of 16 bit, so that we can
                // square them in the next step
                store_1 = _mm256_srli_epi16 (store_1, 8);

                store_2 = _mm256_srli_epi16 (store_2, 8);

                // Calculating the squares of the values for the standard
                // deviation
                __m256i square_1 = _mm256_mullo_epi16 (store_1, store_1);
                __m256i square_2 = _mm256_mullo_epi16 (store_2, store_2);

                // Adding the squares of the values by casting them into 32 bit
                // integer and performing an addition of all values in the ymm
                // register
                __m256i square_12 = _mm256_unpacklo_epi16 (square_1, _mm256_setzero_si256 ());
                square_1          = _mm256_unpackhi_epi16 (square_1, _mm256_setzero_si256 ());
                __m256i square_22 = _mm256_unpacklo_epi16 (square_2, _mm256_setzero_si256 ());
                square_2          = _mm256_unpackhi_epi16 (square_2, _mm256_setzero_si256 ());

                square_12           = _mm256_add_epi32 (_mm256_add_epi32 (square_1, square_12),
                                              _mm256_add_epi32 (square_2, square_22));
                __m128i sum_128vecl = _mm256_extracti128_si256 (square_12, 0);
                __m128i sum_128vech = _mm256_extracti128_si256 (square_12, 1);
                __m128i sum_vec     = _mm_add_epi32 (sum_128vecl, sum_128vech);
                sum_vec             = _mm_hadd_epi32 (sum_vec, sum_vec);
                sum_vec             = _mm_hadd_epi32 (sum_vec, sum_vec);
                sum_of_squares += _mm_extract_epi32 (sum_vec, 0);

                // Calculating sum of all pixel values for the standard
                // deviation
                __m256i sum_vec256 = _mm256_adds_epi16 (store_1, store_2);
                __m128i low        = _mm256_extracti128_si256 (sum_vec256, 0);
                __m128i high       = _mm256_extracti128_si256 (sum_vec256, 1);
                __m128i sum_128vec = _mm_add_epi16 (low, high);
                sum_128vec         = _mm_hadds_epi16 (sum_128vec, sum_128vec);
                sum_128vec         = _mm_hadds_epi16 (sum_128vec, sum_128vec);
                sum_128vec         = _mm_hadds_epi16 (sum_128vec, sum_128vec);
                sum += _mm_extract_epi16 (sum_128vec, 0);

                // Convert everything back to one xmm register with 8 bit values
                __m256i result = _mm256_packus_epi16 (store_1, store_2);
                // Reordering the entries
                result = _mm256_permute4x64_epi64 (result, 0b11011000);

                _mm256_storeu_si256 ((__m256i*) &input_image[n], result);

                n += 32;
                i += 96;
            }
        }
        // Calculating the rest, that is not possible to parallelize
        while (n < size)
        {
            uint8_t data = (a_dis * img[i] + b_dis * img[i + 1] + c_dis * img[i + 2]) / 256;

            // Clamping
            if (-brightness > data)
            {
                data = 0;
            }
            else
            {
                data += brightness;
            }

            // Calculating the sums for the standard deviation
            sum += data;
            sum_of_squares += data * data;
            input_image[n] = data;
            n++;
            i += 3;
        }
    }

    // Computing the average necessary for the contrast adjustment
    double mu = ((double) sum) / size;
    // Computing the standard deviation, by using a mathematical transformation
    // of the given formula. Have a look at the Projektbericht.md for a better
    // understanding.
    double sigma = sqrt (((double) sum_of_squares + (-2 * mu * sum) + (mu * mu * size)) / size);

    adjust_contrast_avx (input_image, width, height, contrast, sigma, mu);
}

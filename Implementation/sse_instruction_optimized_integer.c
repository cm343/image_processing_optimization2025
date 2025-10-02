#include <emmintrin.h>
#include <math.h>
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
static void adjust_contrast (uint8_t* input_image,
                             size_t width,
                             size_t height,
                             float contrast,
                             float sigma,
                             float mu)
{
    // For a better understanding we would recommend, to read the
    // project report first, because the following code is based on some
    // mathematical transformations of the given formula.

    // We do not adjust the contrast if sigma is zero. If sigma is zero then all
    // pixel have the same value. It is impossible to adjust a contrast on a
    // picture without existing contrast, that's why we decided to just ignore
    // the contrast adjustment in this case.
    if (sigma == 0.0)
    {
        return;
    }

    // General Declarations
    uint64_t size   = width * height;
    uint64_t n      = 0;
    __m128i bitmask = _mm_set1_epi16 (255);

    // Precomputing all parts of the given formula, that are independent from
    // the pixel value, because these values/xmm-register(with values) are the
    // same for every pixel and we save time with not calculating them every
    // time again.
    double k_divided_by_sigma = contrast / sigma;
    uint16_t k_divided_by_sigma_16;
    __m128i k_divided_by_sigma_vec;

    // If the contrast is negative then we're multiplying it with -1 as well as
    // the other part, what makes it possible to store unsigned values which
    // gave us more bits for precession. We're using a xmm register with 8 16
    // bit values of the contrast as an 8.8 fixpoint number
    if (contrast < 0.0)
    {
        // clamp, if the value is smaller than 2^8
        if (-1 * k_divided_by_sigma * 256 >= 65535)
        {
            k_divided_by_sigma_16 = 65535;
        }
        else
        {
            k_divided_by_sigma_16 = (-1 * k_divided_by_sigma * 256);
        }
        k_divided_by_sigma_vec = _mm_set1_epi16 (k_divided_by_sigma_16);
    }
    else
    {
        // clamp, if the value is bigger than 2^8
        if (k_divided_by_sigma * 256 >= 65535)
        {
            k_divided_by_sigma_16 = 65535;
        }
        else
        {
            k_divided_by_sigma_16 = (k_divided_by_sigma * 256);
        }
        k_divided_by_sigma_vec = _mm_set1_epi16 (k_divided_by_sigma_16);
    }

    // if k is zero then regarding to the formula, the result should be mu. In
    // our case with the changes formula we would get a division by zero, what
    // would be undefined behavior. Thats why were catching it here. if k/sigma
    // is zero what can happens with k > 0 because of less permission, than we
    // would multiplying with zero, what would change the results. Thats why
    // we're catching it here, because the real value would be near to mu.
    if (k_divided_by_sigma_16 == 0)
    {
        __m128i mu_vec = _mm_set1_epi8 (mu);
        if (size > 15)
        {
            while (n < size - 16)
            {
                _mm_storeu_si128 ((__m128i*) &input_image[n], mu_vec);
                n += 16;
            }
        }
        while (n < size)
        {
            input_image[n] = mu;
            n++;
        }
        return;
    }

    // Precalculation
    double one_minus_k_divided_by_sigma_times_mu = (1 - k_divided_by_sigma) * mu;

    // Precalculation of the zero xmm-register, that is used multiple times in
    // the code -> faster
    __m128i zero_vec = _mm_setzero_si128 ();

    // positive case: if mu_minus_mu_times_sigma_divided_by_contrast is negative
    // and k is positive, then we have to use mu - mu * sigma / contrast
    float mu_minus_mu_times_sigma_divided_by_contrast = mu - (sigma / contrast) * mu;
    __m128i mu_minus_mu_times_sigma_divided_by_contrast_vec;

    // Tht following Code Block is three times very similar. Thats because we're
    // extracting two loops from the loopbody what gives us a better performance
    if (mu_minus_mu_times_sigma_divided_by_contrast >= 0.0 && contrast > 0.0)
    {
        // Precalculations of a clamped value to calculate with
        uint16_t mu_minus_mu_times_sigma_divided_by_contrast_16 =
            mu_minus_mu_times_sigma_divided_by_contrast >= 65535
                ? 65535
                : round (mu_minus_mu_times_sigma_divided_by_contrast);
        mu_minus_mu_times_sigma_divided_by_contrast_vec =
            _mm_set1_epi16 (mu_minus_mu_times_sigma_divided_by_contrast_16);

        if (size >= 16)
        {
            while (n < size - 15)
            {
                // Load 16 8 bit unsigned integers and convert them to 4 float
                // xmm register where every register has 4 floats
                __m128i data = _mm_loadu_si128 ((__m128i*) &input_image[n]);

                // Converting the data in unsigned 16 bit values
                __m128i data_lo = _mm_unpacklo_epi8 (data, zero_vec);
                __m128i data_hi = _mm_unpackhi_epi8 (data, zero_vec);

                // This is the step that is different in the code Blocks
                // In the case of  contrast > 0 and
                // mu_minus_mu_times_sigma_divided_by_contrast > 0 were
                // subtracting the mu_minus_mu_times_sigma_divided_by_contrast
                // from the pixel value
                data_lo = _mm_subs_epu16 (data_lo, mu_minus_mu_times_sigma_divided_by_contrast_vec);
                data_hi = _mm_subs_epu16 (data_hi, mu_minus_mu_times_sigma_divided_by_contrast_vec);

                // The high multiplication shows us if there are overflows that
                // we have to clamp, because an 8.8 integer multiplied with an 8
                // bit integer can lead into an overflow
                __m128i data_lo_hi = _mm_mulhi_epu16 (data_lo, k_divided_by_sigma_vec);
                __m128i data_hi_hi = _mm_mulhi_epu16 (data_hi, k_divided_by_sigma_vec);

                // We're calculating the product of k/sigma times the adjusted
                // pixel value with a mul low and proofing after that, that if
                // an overflow happened, to value is clamped to 255.
                data_lo    = _mm_mullo_epi16 (data_lo, k_divided_by_sigma_vec);
                data_lo_hi = _mm_cmpgt_epi16 (data_lo_hi, zero_vec);
                data_lo_hi = _mm_and_si128 (bitmask, data_lo_hi);

                data_hi    = _mm_mullo_epi16 (data_hi, k_divided_by_sigma_vec);
                data_hi_hi = _mm_cmpgt_epi16 (data_hi_hi, zero_vec);
                data_hi_hi = _mm_and_si128 (bitmask, data_hi_hi);

                // here we're catching the overflow if it happens
                data_lo = _mm_srli_epi16 (data_lo, 8);
                data_lo = _mm_or_si128 (data_lo, data_lo_hi);

                data_hi = _mm_srli_epi16 (data_hi, 8);
                data_hi = _mm_or_si128 (data_hi, data_hi_hi);

                // combining the data back to 16 8 bit unsigned integer
                data = _mm_packus_epi16 (data_lo, data_hi);

                _mm_storeu_si128 ((__m128i*) &input_image[n], data);
                n += 16;
            }
        }
        // calculating the rest of the pixels that aren't enough for using SIMD
        while (n < size)
        {
            // using the formula for calculations
            float data =
                k_divided_by_sigma * input_image[n] + one_minus_k_divided_by_sigma_times_mu;
            // Clamping if it is necessary
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
                input_image[n] = data;
            }
            n++;
        }
        return;
    }
    else if (mu_minus_mu_times_sigma_divided_by_contrast < 0.0 && contrast > 0.0)
    {
        // in this case we're adding the precalculated value because the value
        // is negative and the subtraction of a negative value is the same as
        // the addition of the absolute That's why we're calculating the
        // absolute and putting that into the xmm-register.
        uint16_t mu_minus_mu_times_sigma_divided_by_contrast_16 =
            -1 * (mu_minus_mu_times_sigma_divided_by_contrast) >= 65535
                ? 65535
                : round (-1 * (mu_minus_mu_times_sigma_divided_by_contrast));
        mu_minus_mu_times_sigma_divided_by_contrast_vec =
            _mm_set1_epi16 (mu_minus_mu_times_sigma_divided_by_contrast_16);

        if (size >= 16)
        {
            while (n < size - 15)
            {
                // Load 16 8 bit unsigned integers and convert them to 4 float
                // xmm register where every register has 4 floats
                __m128i data = _mm_loadu_si128 ((__m128i*) &input_image[n]);

                // Converting the data in unsigned 16 bit values (because of the
                // zeros).
                __m128i data_lo = _mm_unpacklo_epi8 (data, zero_vec);
                __m128i data_hi = _mm_unpackhi_epi8 (data, zero_vec);

                // This is the step that is different in the code Blocks
                // In the case of  contrast > 0 and
                // mu_minus_mu_times_sigma_divided_by_contrast < 0 were adding
                // the mu_minus_mu_times_sigma_divided_by_contrast to the pixel
                // value because q - (-rest) = q + rest
                data_lo = _mm_adds_epu16 (data_lo, mu_minus_mu_times_sigma_divided_by_contrast_vec);
                data_hi = _mm_adds_epu16 (data_hi, mu_minus_mu_times_sigma_divided_by_contrast_vec);

                // The high multiplication shows us if there are overflows that
                // we have to clamp, because an 8.8 integer multiplied with an 8
                // bit integer can lead into an overflow
                __m128i data_lo_hi = _mm_mulhi_epu16 (data_lo, k_divided_by_sigma_vec);
                __m128i data_hi_hi = _mm_mulhi_epu16 (data_hi, k_divided_by_sigma_vec);

                // We're calculating the product of k/sigma times the adjusted
                // pixel value with a mul low and proofing after that, that if
                // an overflow happened, to value is clamped to 255.
                data_lo    = _mm_mullo_epi16 (data_lo, k_divided_by_sigma_vec);
                data_lo_hi = _mm_cmpgt_epi16 (data_lo_hi, zero_vec);
                data_lo_hi = _mm_and_si128 (bitmask, data_lo_hi);

                data_hi    = _mm_mullo_epi16 (data_hi, k_divided_by_sigma_vec);
                data_hi_hi = _mm_cmpgt_epi16 (data_hi_hi, zero_vec);
                data_hi_hi = _mm_and_si128 (bitmask, data_hi_hi);

                // Here we're catching the overflow if it happens
                data_lo = _mm_srli_epi16 (data_lo, 8);
                data_lo = _mm_or_si128 (data_lo, data_lo_hi);

                data_hi = _mm_srli_epi16 (data_hi, 8);
                data_hi = _mm_or_si128 (data_hi, data_hi_hi);

                // Combining the data back to 16 8 bit unsigned integer
                data = _mm_packus_epi16 (data_lo, data_hi);

                _mm_storeu_si128 ((__m128i*) &input_image[n], data);
                n += 16;
            }
        }
        // Calculating the rest of the pixels that aren't enough for using SIMD
        while (n < size)
        {
            // Using the formula for calculations
            float data =
                k_divided_by_sigma * input_image[n] + one_minus_k_divided_by_sigma_times_mu;
            // Clamping if it is necessary
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
                input_image[n] = data;
            }
            n++;
        }
        return;
    }

    // Negative case
    float mu_plus_mu_times_sigma_divided_by_contrast = mu - (sigma / contrast) * mu;
    __m128i mu_plus_mu_times_sigma_divided_by_contrast_vec;
    // In this case we're subtracting the pixel value from the precomputed one
    // (have a look at the formula in Projektbericht.md)
    uint16_t mu_plus_mu_times_sigma_divided_by_contrast_16 =
        mu_plus_mu_times_sigma_divided_by_contrast >= 65535
            ? 65535
            : round (mu_plus_mu_times_sigma_divided_by_contrast);
    mu_plus_mu_times_sigma_divided_by_contrast_vec =
        _mm_set1_epi16 (mu_plus_mu_times_sigma_divided_by_contrast_16);

    if (size >= 16)
    {
        while (n < size - 15)
        {
            // Load 16 8 bit unsigned integers and convert them to 4 float xmm
            // register where every register has 4 floats
            __m128i data = _mm_loadu_si128 ((__m128i*) &input_image[n]);

            // Converting the data in unsigned 16 bit values (because of the
            // zeros).
            __m128i data_lo = _mm_unpacklo_epi8 (data, zero_vec);
            __m128i data_hi = _mm_unpackhi_epi8 (data, zero_vec);

            // This is the step that is different in the code Blocks
            // In the case of  contrast < 0 we multiply everything with -1 so
            // that we can subtract this time in the opposite direction
            data_lo = _mm_subs_epu16 (mu_plus_mu_times_sigma_divided_by_contrast_vec, data_lo);
            data_hi = _mm_subs_epu16 (mu_plus_mu_times_sigma_divided_by_contrast_vec, data_hi);

            // The high multiplication shows us if there are overflows that we
            // have to clamp, because an 8.8 integer multiplied with an 8 bit
            // integer can lead into an overflow
            __m128i data_lo_hi = _mm_mulhi_epu16 (data_lo, k_divided_by_sigma_vec);
            __m128i data_hi_hi = _mm_mulhi_epu16 (data_hi, k_divided_by_sigma_vec);

            // We're calculating the product of k/sigma times the adjusted pixel
            // value with a mul low and proofing after that, that if an overflow
            // happened, to value is clamped to 255.
            data_lo    = _mm_mullo_epi16 (data_lo, k_divided_by_sigma_vec);
            data_lo_hi = _mm_cmpgt_epi16 (data_lo_hi, zero_vec);
            data_lo_hi = _mm_and_si128 (bitmask, data_lo_hi);

            data_hi    = _mm_mullo_epi16 (data_hi, k_divided_by_sigma_vec);
            data_hi_hi = _mm_cmpgt_epi16 (data_hi_hi, zero_vec);
            data_hi_hi = _mm_and_si128 (bitmask, data_hi_hi);
            // here we're catching the overflow if it happens
            data_lo = _mm_srli_epi16 (data_lo, 8);
            data_lo = _mm_or_si128 (data_lo, data_lo_hi);

            data_hi = _mm_srli_epi16 (data_hi, 8);
            data_hi = _mm_or_si128 (data_hi, data_hi_hi);

            // combining the data back to 16 8 bit unsigned integer
            data = _mm_packus_epi16 (data_lo, data_hi);

            _mm_storeu_si128 ((__m128i*) &input_image[n], data);
            n += 16;
        }
        // calculating the rest of the pixels that aren't enough for using SIMD
        while (n < size)
        {
            // using the formula for calculations
            float data =
                k_divided_by_sigma * input_image[n] + one_minus_k_divided_by_sigma_times_mu;
            // clamping if it is necessary
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
                input_image[n] = data;
            }
            n++;
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
void brightness_contrast_V4 (const uint8_t* img,
                             size_t width,
                             size_t height,
                             float a,
                             float b,
                             float c,
                             int16_t brightness,
                             float contrast,
                             uint8_t* input_image)
{
    // Declarations
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
            // a xmm register filled with the brightness as 8 bit unsigned value
            __m128i brightness_vec = _mm_set1_epi8 (brightness);
            if (size >= 16)
            {
                while (n < size - 15)
                {
                    _mm_storeu_si128 ((__m128i*) &input_image[n], brightness_vec);
                    n += 16;
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
            // using a xmm register full of zeros for the clamped negative
            // brightness
            __m128i brightness_vec = _mm_setzero_si128 ();
            if (size >= 16)
            {
                while (n < size - 15)
                {
                    _mm_storeu_si128 ((__m128i*) &input_image[n], brightness_vec);
                    n += 16;
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

    // We're reading 3 times 16 8 bit unsigned values in 3 different
    // xmm registers. The first value is r, the second g, and the third is b of
    // the first pixel. After that the next triple is loaded and so on. For
    // doing the calculations with these values we need a different order. We
    // need 16 bit unsigned integers, and we need a,b,c of one pixel at the same
    // position in this 3 xmm-registers. That's why we're using these bitmasks,
    // that are rearranging the values as specified. The system is a bit
    // complicated, because we want to make sure that we do not occupy too many
    // registers, which would lead to more slow memory access
    __m128i mask_ac_11 = _mm_set_epi8 (-1, -1, -1, -1, -1, 15, -1, 12, -1, 9, -1, 6, -1, 3, -1, 0);
    __m128i mask_a_12 = _mm_set_epi8 (-1, 5, -1, 2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i mask_a_21 =
        _mm_set_epi8 (-1, 8, -1, 11, -1, 14, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m128i mask_ba_11 = _mm_set_epi8 (-1, -1, -1, -1, -1, -1, -1, 13, -1, 10, -1, 7, -1, 4, -1, 1);
    __m128i mask_b_12  = _mm_set_epi8 (-1, 6, -1, 3, -1, 0, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i mask_b_21 =
        _mm_set_epi8 (-1, 9, -1, 12, -1, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);

    __m128i mask_cb_11 = _mm_set_epi8 (-1, -1, -1, -1, -1, -1, -1, 14, -1, 11, -1, 8, -1, 5, -1, 2);
    __m128i mask_c_12  = _mm_set_epi8 (-1, 7, -1, 4, -1, 1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i mask_c_21 =
        _mm_set_epi8 (-1, 10, -1, 13, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1);
    __m128i mask_c_22 = _mm_set_epi8 (-1, -1, -1, -1, -1, 0, -1, 15, -1, 12, -1, 9, -1, 6, -1, 3);

    // these are 16 bit xmm register, that has a, b or c as values for weighting
    // the values
    __m128i a_vec = _mm_set1_epi16 (a_dis);
    __m128i b_vec = _mm_set1_epi16 (b_dis);
    __m128i c_vec = _mm_set1_epi16 (c_dis);

    // this register has the purpose to bring the calculated values in the right
    // order again. That is necessary, because our xmm registers are not
    // ordered, corresponding to the order the values were loaded, because this
    // way we have less overhead with masks
    __m128i bit_mask_2 = _mm_set_epi8 (12, 11, 10, 9, 8, 13, 14, 15, 7, 6, 5, 4, 3, 2, 1, 0);

    // In this case distinction we check, if the brightness is positive or
    // negative. By this we only have to save the absolute
    // of the values and then either add or subtract. That gives us one bit more
    // for information because of the missing sign bit.
    if (brightness > 0)
    {
        // In this case we're adding
        // That is the precomputed xmm-resister with brightness, that is shifted
        // to the first 8 bit of the 16 bit integer. That makes the automated
        // clamping easier.
        __m128i brightness_vec = _mm_set1_epi16 (256 * brightness);

        // The following code-block is in a very similar way two times in this
        // code, because in that way we can exclude an if statement from loop,
        // what gives us better performance. The first time we're calculating
        // with a positiv brightness, what needs an addition from pixel value
        // and brightness. In the other case the brightness is negativ, what
        // needs a subtraction of the brightness from the pixel value
        if (size >= 16)
        {
            while (n < size - 15)
            {
                // reading the data in blocks of 16 8 bit integern
                __m128i load_rgb_1 = _mm_loadu_si128 ((__m128i*) &img[i]);
                __m128i load_rgb_2 = _mm_loadu_si128 ((__m128i*) &img[i + 16]);
                __m128i load_rgb_3 = _mm_loadu_si128 ((__m128i*) &img[i + 32]);

                // Converting the data in 16 bit xmm register that only have
                // red, green, or blue values. The values which belong to each
                // other are at the same position of the xmm register, but they
                // are not ordered like the data is in the beginning, because
                // were saving overhead with that.
                __m128i store_a_1 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_ac_11),
                                                  _mm_shuffle_epi8 (load_rgb_2, mask_a_12));
                __m128i store_a_2 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_a_21),
                                                  _mm_shuffle_epi8 (load_rgb_3, mask_ba_11));
                __m128i store_b_1 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_ba_11),
                                                  _mm_shuffle_epi8 (load_rgb_2, mask_b_12));
                __m128i store_b_2 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_b_21),
                                                  _mm_shuffle_epi8 (load_rgb_3, mask_cb_11));
                __m128i store_c_1 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_cb_11),
                                                  _mm_shuffle_epi8 (load_rgb_2, mask_c_12));
                __m128i store_c_2 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_c_21),
                                                  _mm_shuffle_epi8 (load_rgb_3, mask_c_22));

                // Multiplying the rgb values with the normalized values of
                // a,b,c. That's the reason why we chose the 16 bit values
                // because the product of two 8 bit values has a maximum of 16
                // bits.
                store_a_1 = _mm_mullo_epi16 (store_a_1, a_vec);
                store_a_2 = _mm_mullo_epi16 (store_a_2, a_vec);
                store_b_1 = _mm_mullo_epi16 (store_b_1, b_vec);
                store_b_2 = _mm_mullo_epi16 (store_b_2, b_vec);
                store_c_1 = _mm_mullo_epi16 (store_c_1, c_vec);
                store_c_2 = _mm_mullo_epi16 (store_c_2, c_vec);

                // Now we can add the values r, g and b, that are weighted in a
                // way that the sum is a value between 0 and 255
                __m128i store_1 = _mm_adds_epu16 (store_a_1, _mm_adds_epu16 (store_b_1, store_c_1));
                __m128i store_2 = _mm_adds_epu16 (store_a_2, _mm_adds_epu16 (store_b_2, store_c_2));

                // Adjusting brightness by adding the positive brightness to the
                // pixel values. The operation does automate clamping
                store_1 = _mm_adds_epu16 (store_1, brightness_vec);
                store_2 = _mm_adds_epu16 (store_2, brightness_vec);

                // We're shifting the 8 bits of information that are standing
                // between bit 8 and 15 to bit 0 to 7 for multiplying them.
                store_1 = _mm_srli_epi16 (store_1, 8);
                store_2 = _mm_srli_epi16 (store_2, 8);

                // Calculating sum of the squares of all pixels to later
                // calculate the standard deviation
                __m128i square_1 = _mm_mullo_epi16 (store_1, store_1);
                __m128i square_2 = _mm_mullo_epi16 (store_2, store_2);

                // Adding the 16 bit values of the squares by casting them into
                // 32 bit value.
                __m128i square_12 = _mm_unpacklo_epi16 (square_1, _mm_setzero_si128 ());
                square_1          = _mm_unpackhi_epi16 (square_1, _mm_setzero_si128 ());
                __m128i square_22 = _mm_unpacklo_epi16 (square_2, _mm_setzero_si128 ());
                square_2          = _mm_unpackhi_epi16 (square_2, _mm_setzero_si128 ());
                square_1          = _mm_add_epi32 (square_1, square_12);
                square_2          = _mm_add_epi32 (square_2, square_22);
                __m128i sum_vec   = _mm_hadd_epi32 (square_2, square_1);
                sum_vec           = _mm_hadd_epi32 (sum_vec, sum_vec);
                sum_vec           = _mm_hadd_epi32 (sum_vec, sum_vec);
                sum_of_squares += _mm_cvtsi128_si32 (sum_vec);

                // Calculating sum of all pixels to later calculate the
                // average/standard deviation
                sum_vec = _mm_hadds_epi16 (store_1, store_2);
                sum_vec = _mm_hadds_epi16 (sum_vec, sum_vec);
                sum_vec = _mm_hadds_epi16 (sum_vec, sum_vec);
                sum_vec = _mm_hadds_epi16 (sum_vec, sum_vec);

                sum += _mm_extract_epi16 (sum_vec, 0);

                // Convert everything back to one xmm register with 8 bit values
                __m128i result = _mm_packus_epi16 (store_1, store_2);

                // Reordering the values, so that they have the original order
                result = _mm_shuffle_epi8 (result, bit_mask_2);

                _mm_storeu_si128 ((__m128i*) &input_image[n], result);

                n += 16;
                i += 48;
            }
        }
        // Calculating the rest, that is not possible to parallelize
        while (n < size)
        {
            uint8_t data =
                (round (a) * img[i] + round (b) * img[i + 1] + round (c) * img[i + 2]) / 255;

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
        // xmm-resister with the absolute of the brightness, that is shifted to
        // the first 8 bit of the 16 bit integer. That makes the automated
        // clamping easier.
        __m128i brightness_vec = _mm_set1_epi16 (256 * -brightness);

        // This block is the duplicate of the one above, with the little
        // difference, that this one subtracts the absolute of the brightness
        // and do not add it.
        if (size >= 16)
        {
            while (n < size - 15)
            {
                // reading the data in blocks of 16 8 bit integer
                __m128i load_rgb_1 = _mm_loadu_si128 ((__m128i*) &img[i]);
                __m128i load_rgb_2 = _mm_loadu_si128 ((__m128i*) &img[i + 16]);
                __m128i load_rgb_3 = _mm_loadu_si128 ((__m128i*) &img[i + 32]);

                // Converting the data in 16 bit xmm register that only have
                // red, green, or blue values. The values which belong to each
                // other are at the same position of the xmm register, but they
                // are not ordered like the data is in the beginning, because
                // were saving overhead with that.
                __m128i store_a_1 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_ac_11),
                                                  _mm_shuffle_epi8 (load_rgb_2, mask_a_12));
                __m128i store_a_2 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_a_21),
                                                  _mm_shuffle_epi8 (load_rgb_3, mask_ba_11));
                __m128i store_b_1 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_ba_11),
                                                  _mm_shuffle_epi8 (load_rgb_2, mask_b_12));
                __m128i store_b_2 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_b_21),
                                                  _mm_shuffle_epi8 (load_rgb_3, mask_cb_11));
                __m128i store_c_1 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_1, mask_cb_11),
                                                  _mm_shuffle_epi8 (load_rgb_2, mask_c_12));
                __m128i store_c_2 = _mm_or_si128 (_mm_shuffle_epi8 (load_rgb_2, mask_c_21),
                                                  _mm_shuffle_epi8 (load_rgb_3, mask_c_22));

                // Multiplying the rgb values with the normalized values of
                // a,b,c. That's the reason why we chose the 16 bit values
                // because the product of two 8 bit values has a maximum of 16
                // bits.
                store_a_1 = _mm_mullo_epi16 (store_a_1, a_vec);
                store_a_2 = _mm_mullo_epi16 (store_a_2, a_vec);
                store_b_1 = _mm_mullo_epi16 (store_b_1, b_vec);
                store_b_2 = _mm_mullo_epi16 (store_b_2, b_vec);
                store_c_1 = _mm_mullo_epi16 (store_c_1, c_vec);
                store_c_2 = _mm_mullo_epi16 (store_c_2, c_vec);

                // Now we can add the values r, g and b, that are weighted in a
                // way that the sum is a value between 0 and 255
                __m128i store_1 = _mm_adds_epu16 (store_a_1, _mm_adds_epu16 (store_b_1, store_c_1));
                __m128i store_2 = _mm_adds_epu16 (store_a_2, _mm_adds_epu16 (store_b_2, store_c_2));

                // Adjusting brightness by subtracting the positive brightness
                // to the pixel values. The operation does automate clamping
                store_1 = _mm_subs_epu16 (store_1, brightness_vec);
                store_2 = _mm_subs_epu16 (store_2, brightness_vec);

                // We're shifting the 8 bit of information that are standing
                // between bit 8 and 15 to bit 0 to 7 for multiplying them.
                store_1 = _mm_srli_epi16 (store_1, 8);
                store_2 = _mm_srli_epi16 (store_2, 8);

                // Calculating sum of the squares of all pixels to later
                // calculate the standard deviation
                __m128i square_1 = _mm_mullo_epi16 (store_1, store_1);
                __m128i square_2 = _mm_mullo_epi16 (store_2, store_2);

                // Adding the 16 bit values of the squares by casting them into
                // 32 bit value.
                __m128i square_12 = _mm_unpacklo_epi16 (square_1, _mm_setzero_si128 ());
                square_1          = _mm_unpackhi_epi16 (square_1, _mm_setzero_si128 ());
                __m128i square_22 = _mm_unpacklo_epi16 (square_2, _mm_setzero_si128 ());
                square_2          = _mm_unpackhi_epi16 (square_2, _mm_setzero_si128 ());
                square_1          = _mm_add_epi32 (square_1, square_12);
                square_2          = _mm_add_epi32 (square_2, square_22);
                __m128i sum_vec   = _mm_hadd_epi32 (square_2, square_1);
                sum_vec           = _mm_hadd_epi32 (sum_vec, sum_vec);
                sum_vec           = _mm_hadd_epi32 (sum_vec, sum_vec);
                sum_of_squares += _mm_cvtsi128_si32 (sum_vec);

                // Calculating sum of all pixels to later calculate the
                // average/standard deviation
                sum_vec = _mm_hadds_epi16 (store_1, store_2);
                sum_vec = _mm_hadds_epi16 (sum_vec, sum_vec);
                sum_vec = _mm_hadds_epi16 (sum_vec, sum_vec);
                sum_vec = _mm_hadds_epi16 (sum_vec, sum_vec);

                sum += _mm_extract_epi16 (sum_vec, 0);

                // Convert everything back to one xmm register with 8 bit values
                __m128i result = _mm_packus_epi16 (store_1, store_2);

                // Reordering the values, so that they have the original order
                result = _mm_shuffle_epi8 (result, bit_mask_2);

                _mm_storeu_si128 ((__m128i*) &input_image[n], result);

                n += 16;
                i += 48;
            }
        }
        // Calculating the rest, that is not possible to parallelize
        while (n < size)
        {
            uint8_t data =
                (round (a) * img[i] + round (b) * img[i + 1] + round (c) * img[i + 2]) / 255;

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

    adjust_contrast (input_image, width, height, contrast, sigma, mu);
}

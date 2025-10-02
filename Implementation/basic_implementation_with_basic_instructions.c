#include "math.h" // only for isinf() and isnan()

#include <stdint.h>
#include <stdio.h>

/**
 * @brief computing the square_root using the Heron-Algorithm
 *
 * @param element Value of which the square root is computed
 */
static double square_root (double element)
{
    if (element == 0)
    {
        return 0;
    }
    double x_1 = element / 2.0;
    double x_2 = element + 1.0; // something which isn't one
    while (x_1 != x_2)
    {
        x_1 = x_2;
        x_2 = 0.5 * (x_1 + element / x_1);
    }
    return x_2;
}
/**
 * @brief Rounding function for doubles in range [0, 255]
 *
 * @param in Value in [0, 255] which is rounded
 */
static uint8_t round_double_to_uint8_t (double in)
{
    if ((in - (uint8_t) in) >= 0.5)
    {
        return (uint8_t) (in + 1);
    }
    return (uint8_t) in;
}

/**
 * @brief convert the RGB values of *img to greyscale in *greyscale_result
 *
 * @param img A pointer to the colored pixeldata
 * @param img_size The number of pixels
 * @param a The weight for the red value
 * @param b The weight for the green value
 * @param c The weight for the blue value
 * @param greyscale_result A pointer to the greyscaled pixeldata
 */
static void to_greyscale (const uint8_t* img,
                          size_t img_size,
                          float a,
                          float b,
                          float c,
                          uint8_t* greyscale_result)
{
    // cast the float inputs to double to prevent overflow
    double sum_of_weights = ((double) a) + ((double) b) + ((double) c);

    size_t pixel_arr_size = (img_size * 3);

    int n = 0;

    // compute greyscale values
    for (size_t i = 0; i < pixel_arr_size; i += 3)
    {
        greyscale_result[n++] = round_double_to_uint8_t (
            ((double) a * img[i] + (double) b * img[i + 1] + (double) c * img[i + 2])
            / sum_of_weights);
    }
}
/**
 * @brief Adjust the brightness of the input image
 *
 * @param input_image A pointer to the pixeldata
 * @param img_size The number of pixels
 * @param brightness The brightness value by which the pixels are adjusted
 */
static void
adjust_brightness_basic_instructions (uint8_t* input_image, size_t img_size, int16_t brightness)
{
    // Adjust the brightness of the input image
    for (size_t i = 0; i < img_size; i++)
    {
        int16_t unclamped = (int16_t) input_image[i] + brightness;
        if (unclamped < 0)
            input_image[i] = 0;
        else if (unclamped > 255)
            input_image[i] = 255;
        else
            input_image[i] = unclamped;
    }
}
/**
 * @brief compute the standard deviation needed for the contrast adjustment
 *
 * @param input_image A pointer to the pixeldata
 * @param size The number of pixels
 * @param average The average over all pixels
 */
//
static double
compute_standard_deviation (const uint8_t* input_image, size_t img_size, double average)
{
    double sum = 0;

    for (size_t i = 0; i < img_size; i++)
    {
        sum += (input_image[i] - average) * (input_image[i] - average);
    }

    double sigma_squared = sum / (double) (img_size);

    return square_root (sigma_squared);
}
/**
 * @brief compute the average needed for contrast adjustment
 *
 * @param input_image A pointer to the pixeldata
 * @param size The number of pixels
 */
static double compute_average (const uint8_t* input_image, size_t img_size)
{
    uint64_t sum_over_pixel_values = 0;

    for (size_t i = 0; i < img_size; i++)
    {
        sum_over_pixel_values += input_image[i];
    }

    return (double) sum_over_pixel_values / ((double) (img_size));
}
/**
 * @brief  This method does the contrast adjustment.
 *
 * @param input_image A pointer to the pixeldata
 * @param size The number of pixels
 * @param contrast The given constant that is used for the contrast adjustment
 */
static void adjust_contrast (uint8_t* input_image, size_t img_size, float k)
{
    double average            = compute_average (input_image, img_size);
    double standard_deviation = compute_standard_deviation (input_image, img_size, average);

    double k_div_sigma = k / (double) standard_deviation;

    // If the standard deviation comes near to zero, k_div_sigma results in
    //  infinity (if k != 0) or in NaN (if k = 0) In this case every pixels
    //  value of the output image is set to the average of all pixels in the
    //  input image
    if (isinf (k_div_sigma) || isnan (k_div_sigma))
    {
        uint8_t rounded_average = round_double_to_uint8_t (average);
        for (size_t i = 0; i < img_size; ++i)
        {
            input_image[i] = rounded_average;
        }
        return;
    }

    // Precompute (1- (k/sigma)*mu with normalizing values with 1/256 to prevent
    // double overflow
    double k_div_sigma_normalized = k_div_sigma / 256;
    double one_minus_k_div_sigma_times_average_normalized =
        ((1 / (double) 256) - k_div_sigma_normalized) * average;

    for (size_t i = 0; i < img_size; i++)
    {
        // compute, denormalize and round data, unclamped==inf is caught by
        // clamping
        double unclamped = (input_image[i] * k_div_sigma_normalized
                            + one_minus_k_div_sigma_times_average_normalized)
                           * 256;
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
            input_image[i] = round_double_to_uint8_t (unclamped);
        }
    }
}

/**
 * @brief  This method is greyscaling a ppm image and adjusting the brightness
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
void brightness_contrast_V2 (const uint8_t* img,
                             size_t width,
                             size_t height,
                             float a,
                             float b,
                             float c,
                             int16_t brightness,
                             float contrast,
                             uint8_t* result)
{
    size_t img_size = width * height;
    // catch a,b,c equals zero to prevent division by zero in to_greyscale()
    if (a == 0 && b == 0 && c == 0)
    {
        if (brightness < 0)
            brightness = 0;
        for (size_t i = 0; i < img_size; ++i)
        {
            result[i] = brightness;
        }
        return;
    }

    to_greyscale (img, img_size, a, b, c, result);

    // skip brightness adjust if brightness equals zero
    if (brightness != 0)
    {
        adjust_brightness_basic_instructions (result, img_size, brightness);
    }

    adjust_contrast (result, img_size, contrast);
}

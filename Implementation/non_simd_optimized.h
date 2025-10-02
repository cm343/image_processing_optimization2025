#ifndef NON_SIMD_OPTIMIZED_H
#define NON_SIMD_OPTIMIZED_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>

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
                             uint8_t* result);

#endif // NON_SIMD_OPTIMIZED_H
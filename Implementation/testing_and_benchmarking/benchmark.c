#include "benchmark.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Executes the function fun of type brightness_contrast_implementation (defined
// in brightness_contrast_implementation_signature.h)  with the given parameters
// one time and returns the execution time in milliseconds
double measure_execution_time_single (brightness_contrast_implementation fun,
                                      const uint8_t* img,
                                      size_t width,
                                      size_t height,
                                      float a,
                                      float b,
                                      float c,
                                      int16_t brightness,
                                      float contrast,
                                      uint8_t* result)
{
    struct timespec start;
    struct timespec end;
    clock_gettime (CLOCK_MONOTONIC, &start);

    fun (img, width, height, a, b, c, brightness, contrast, result);

    clock_gettime (CLOCK_MONOTONIC, &end);

    return ((end.tv_sec - start.tv_sec) * 1e3 + 1e-6 * (end.tv_nsec - start.tv_nsec));
}

// Executes the function fun of type brightness_contrast_implementation (defined
// in brightness_contrast_implementation_signature.h) with the given parameters
// n times and returns the total execution time in milliseconds
double measure_execution_time_multiple_total (uint32_t n,
                                              brightness_contrast_implementation fun,
                                              const uint8_t* img,
                                              size_t width,
                                              size_t height,
                                              float a,
                                              float b,
                                              float c,
                                              int16_t brightness,
                                              float contrast,
                                              uint8_t* result)
{
    struct timespec start;
    struct timespec end;
    clock_gettime (CLOCK_MONOTONIC, &start);

    for (; n > 0; n--)
    {
        fun (img, width, height, a, b, c, brightness, contrast, result);
    }

    clock_gettime (CLOCK_MONOTONIC, &end);

    return ((double) (end.tv_sec - start.tv_sec) * 1e3
            + (double) (end.tv_nsec - start.tv_nsec) / 1e6);
}

// Executes the function fun of type brightness_contrast_implementation (defined
// in brightness_contrast_implementation_signature.h) with the given parameters
// n times and returns the average execution time per execution in milliseconds
double measure_execution_time_multiple_average (uint32_t n,
                                                brightness_contrast_implementation fun,
                                                const uint8_t* img,
                                                size_t width,
                                                size_t height,
                                                float a,
                                                float b,
                                                float c,
                                                int16_t brightness,
                                                float contrast,
                                                uint8_t* result)
{
    return measure_execution_time_multiple_total (
               n, fun, img, width, height, a, b, c, brightness, contrast, result)
           / n;
}

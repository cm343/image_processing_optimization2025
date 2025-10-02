#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "../brightness_contrast_implementation_signature.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// functions useful for benchmarking, that abstract the usage of CLOCK_MONOTONIC
// and clock_gettime() for benchmarking functions of signature
// brightness_contrast_implementation (defined in
// brightness_contrast_implementation_signature.h)

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
                                      uint8_t* result);

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
                                              uint8_t* result);

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
                                                uint8_t* result);

#endif // BENCHMARK_H

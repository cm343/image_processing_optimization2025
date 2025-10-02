#ifndef BENCHMARK_H
#define BENCHMARK_H

#include "brightness_contrast_implementation_signature.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

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

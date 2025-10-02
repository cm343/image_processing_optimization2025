
#ifndef IO_BENCHMARK_H
#define IO_BENCHMARK_H

#include "../brightness_contrast_implementation_signature.h"
#include "../process_ppm.h"
#include "benchmark.h"
#include "testing.h"

#include <stdio.h>
#include <string.h>

// benchmarks the given function fun of type brightness_contrast_implementation
// and benchmarks it with every file in files, containing the
// filenames. Input parameters for the benchmarks are equal for every file. The
// resulting average execution time over a minimum of min_iterations for every file is written
// to a file in .csv format with name benchmark_result_file.
// The actual number of iterations in computed, so that between the timestamps of the measurement
// are at least 2 seconds.
int benchmark_file_list_automatic_iter (char** files,
                                        uint32_t n_files,
                                        uint16_t min_iterations,
                                        char* benchmark_result_file,
                                        brightness_contrast_implementation fun,
                                        float a,
                                        float b,
                                        float c,
                                        int16_t brightness,
                                        float contrast);

#endif


#include "../avx_instruction.h"
#include "../basic_implementation.h"
#include "../basic_implementation_with_basic_instructions.h"
#include "../non_simd_optimized.h"
#include "../sse_instruction_optimized_float.h"
#include "../sse_instruction_optimized_integer.h"
#include "io_benchmark.h"
#include "testing.h"

// main method to execute benchmarks using functions from io_benchmark.h
int main ()
{
    srand (0);

    uint16_t file_number = 30;

    uint16_t min_iterations = 100;

    // params
    float a            = 798.834;
    float b            = 459.294;
    float c            = 1023.08871;
    int16_t brightness = 20;
    float contrast     = 124.898;

    char** files = generate_random_benchmark_dataset (10,
                                                      10000,
                                                      300,
                                                      300,
                                                      0,
                                                      255,
                                                      "./testing_and_benchmarking/"
                                                      "generated_benchmark_inputs/",
                                                      "_bench.ppm",
                                                      file_number);

    // dummy
    benchmark_file_list_automatic_iter (files,
                                        file_number,
                                        min_iterations,
                                        "./testing_and_benchmarking/benchmark_results/development/"
                                        "bench_basic_implementation_with_basic_instructions_dummy."
                                        "csv",
                                        brightness_contrast_V2,
                                        a,
                                        b,
                                        c,
                                        brightness,
                                        contrast);

    // basic implementation
    benchmark_file_list_automatic_iter (files,
                                        file_number,
                                        min_iterations,
                                        "./testing_and_benchmarking/benchmark_results/development/"
                                        "bench_basic_implementation.csv",
                                        brightness_contrast_V1,
                                        a,
                                        b,
                                        c,
                                        brightness,
                                        contrast);

    // non simd optimized
    benchmark_file_list_automatic_iter (files,
                                        file_number,
                                        min_iterations,
                                        "./testing_and_benchmarking/benchmark_results/development/"
                                        "bench_non_simd_optimized.csv",
                                        brightness_contrast_V3,
                                        a,
                                        b,
                                        c,
                                        brightness,
                                        contrast);

    // simd
    benchmark_file_list_automatic_iter (files,
                                        file_number,
                                        min_iterations,
                                        "./testing_and_benchmarking/benchmark_results/development/"
                                        "bench_sse_instruction_optimized_float.csv",
                                        brightness_contrast,
                                        a,
                                        b,
                                        c,
                                        brightness,
                                        contrast);

    benchmark_file_list_automatic_iter (files,
                                        file_number,
                                        min_iterations,
                                        "./testing_and_benchmarking/benchmark_results/development/"
                                        "bench_sse_instruction_optimized_integer.csv",
                                        brightness_contrast_V4,
                                        a,
                                        b,
                                        c,
                                        brightness,
                                        contrast);

    benchmark_file_list_automatic_iter (files,
                                        file_number,
                                        min_iterations,
                                        "./testing_and_benchmarking/benchmark_results/development/"
                                        "bench_avx_instruction.csv",
                                        brightness_contrast_V5,
                                        a,
                                        b,
                                        c,
                                        brightness,
                                        contrast);

    // basic instructions
    benchmark_file_list_automatic_iter (files,
                                        file_number,
                                        min_iterations,
                                        "./testing_and_benchmarking/benchmark_results/development/"
                                        "bench_basic_implementation_with_basic_instructions.csv",
                                        brightness_contrast_V2,
                                        a,
                                        b,
                                        c,
                                        brightness,
                                        contrast);

    free_string_array (files, file_number);
}
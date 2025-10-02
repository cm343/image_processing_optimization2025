#include "../avx_instruction.h"
#include "../brightness_contrast_implementation_signature.h"
#include "../non_simd_optimized.h"
#include "../sse_instruction_optimized_integer.h"
#include "testing.h"

int main ()
{
    char file_list_filename[] = "./testing_and_benchmarking/dataset_images_paths.txt";
    uint16_t filename_length  = 20;

    char file_list_prefix[] = "./testing_and_benchmarking/coco_dataset_val_2017_ppms/";
    char result_filename[] =
        "./testing_and_benchmarking/deviation_distribution_results/development/"
        "dev_base_vs_integer_sse.csv";
    uint64_t max_size = 10000000;
    uint32_t samples  = 100;

    compute_deviation_distribution_file_list (brightness_contrast_V3,
                                              brightness_contrast_V4,
                                              file_list_filename,
                                              filename_length,
                                              file_list_prefix,
                                              max_size,
                                              samples,
                                              result_filename);
}

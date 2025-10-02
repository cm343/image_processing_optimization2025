#include "../avx_instruction.h"
#include "../basic_implementation.h"
#include "../basic_implementation_with_basic_instructions.h"
#include "../non_simd_optimized.h"
#include "../process_ppm.h"
#include "../sse_instruction_optimized_float.h"
#include "../sse_instruction_optimized_integer.h"
#include "float.h"
#include "testing.h"

// main function for executing test using the testing functions in testing.c
// decide between manual testing and automated testing by setting the automatic
// variable
int main ()
{
    srand (0);

    // creating image array
    uint32_t images_size = 48;
    const char* files[]  = {
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000000139.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000000285.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000000632.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000000724.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000000776.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000000785.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000000802.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000000872.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000000885.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001000.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001268.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001296.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001353.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001425.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001490.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001503.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001584.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001675.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001761.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001818.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000001993.ppm",
        "./testing_and_benchmarking/coco_dataset_val_2017_ppms/"
         "000000002006.ppm",
        "testing_and_benchmarking/generated_test_inputs/0_0_0_zero.ppm",
        "testing_and_benchmarking/generated_test_inputs/0_0_50_darker.ppm",
        "testing_and_benchmarking/generated_test_inputs/0_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/0_200_255_brighter.ppm",
        "testing_and_benchmarking/generated_test_inputs/0_255_255_white.ppm",
        "testing_and_benchmarking/generated_test_inputs/1_0_0_zero.ppm",
        "testing_and_benchmarking/generated_test_inputs/1_0_50_darker.ppm",
        "testing_and_benchmarking/generated_test_inputs/1_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/1_200_255_brighter.ppm",
        "testing_and_benchmarking/generated_test_inputs/1_255_255_white.ppm",
        "testing_and_benchmarking/generated_test_inputs/2_0_50_darker.ppm",
        "testing_and_benchmarking/generated_test_inputs/2_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/2_200_255_brighter.ppm",
        "testing_and_benchmarking/generated_test_inputs/3_0_50_darker.ppm",
        "testing_and_benchmarking/generated_test_inputs/3_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/3_200_255_brighter.ppm",
        "testing_and_benchmarking/generated_test_inputs/4_0_50_darker.ppm",
        "testing_and_benchmarking/generated_test_inputs/4_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/4_200_255_brighter.ppm",
        "testing_and_benchmarking/generated_test_inputs/5_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/6_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/7_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/8_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/9_0_255random.ppm",
        "testing_and_benchmarking/generated_test_inputs/"
         "0_0_255_main_fewblack_few_white.ppm",
        "testing_and_benchmarking/generated_test_inputs/"
         "1_0_255_main_fewblack_few_white.ppm",
    };

    uint8_t** images = malloc (sizeof (uint8_t*) * images_size);
    size_t* widths   = malloc (sizeof (size_t) * images_size);
    size_t* heights  = malloc (sizeof (size_t) * images_size);

    // pictures of different size
    char** f = (generate_random_benchmark_dataset (1,
                                                   2000,
                                                   1,
                                                   2000,
                                                   0,
                                                   255,
                                                   "testing_and_benchmarking/"
                                                   "generated_test_inputs/",
                                                   "random.ppm",
                                                   10));

    // generate pictures with specific properties and ignore the returns, but
    // saving them for freeing them later
    char** a = (generate_random_benchmark_dataset (1,
                                                   1000,
                                                   1,
                                                   1000,
                                                   0,
                                                   50,
                                                   "testing_and_benchmarking/"
                                                   "generated_test_inputs/",
                                                   "_darker.ppm",
                                                   5));

    char** b = (generate_random_benchmark_dataset (1,
                                                   1000,
                                                   1,
                                                   1000,
                                                   200,
                                                   255,
                                                   "testing_and_benchmarking/"
                                                   "generated_test_inputs/",
                                                   "_brighter.ppm",
                                                   5));

    char** c = (generate_random_benchmark_dataset (500,
                                                   1000,
                                                   500,
                                                   1000,
                                                   0,
                                                   0,
                                                   "testing_and_benchmarking/"
                                                   "generated_test_inputs/",
                                                   "_zero.ppm",
                                                   2));

    char** d = (generate_random_benchmark_dataset (500,
                                                   1000,
                                                   100,
                                                   1000,
                                                   255,
                                                   255,
                                                   "testing_and_benchmarking/"
                                                   "generated_test_inputs/",
                                                   "_white.ppm",
                                                   2));

    char** e = (generate_main_with_few (500,
                                        1000,
                                        500,
                                        1000,
                                        0,
                                        255,
                                        0.001,
                                        "testing_and_benchmarking/"
                                        "generated_test_inputs/",
                                        "black_few_white.ppm",
                                        2));

    // load specified images from disk
    uint64_t max_size = 0;
    for (uint32_t i = 0; i < images_size; ++i)
    {
        Image* img = read_single_ppm (files[i]);
        if (img == NULL)
        {
            fprintf (stderr, "file %d could not be loaded\n", i);
            continue;
        }
        images[i]  = img->pixel_liste;
        widths[i]  = img->width;
        heights[i] = img->height;
        if (img->height * img->width > max_size)
            max_size = img->width * img->height;
        free (img);
    }

    // init arrays for other params
    int n               = 5;
    float as[5]         = {FLT_MAX, FLT_MIN, 0, 5.3789392, 3454354389.847983};
    float bs[5]         = {FLT_MAX, FLT_MIN, 0, 5.3789392, 3454354389.847983};
    float cs[5]         = {FLT_MAX, FLT_MIN, 0, 5.3789392, 3454354389.847983};
    int16_t brights[5]  = {0, -255, 255, 128, -128};
    float constrasts[6] = {0, 34.4389275492122, -64.34345213, -255, 255, -128};

    // test the specified implementations
    compare_implementations_on_multiple_testcases (brightness_contrast_V3,
                                                   brightness_contrast_V2,
                                                   0,
                                                   images,
                                                   widths,
                                                   heights,
                                                   max_size,
                                                   images_size,
                                                   as,
                                                   n,
                                                   bs,
                                                   n,
                                                   cs,
                                                   n,
                                                   brights,
                                                   n,
                                                   constrasts,
                                                   6);

    // free memory
    for (uint32_t i = 0; i < images_size; ++i)
    {
        free (images[i]);
    }
    free (images);
    free (heights);
    free (widths);
    free_string_array (a, 5);
    free_string_array (b, 5);
    free_string_array (c, 2);
    free_string_array (d, 2);
    free_string_array (e, 2);
    free_string_array (f, 10);
}
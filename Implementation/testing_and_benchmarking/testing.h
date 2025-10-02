#ifndef TESTING_H
#define TESTING_H

#include "../brightness_contrast_implementation_signature.h"
#include "../process_ppm.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

void generate_random_valid_image_array (uint8_t* res, size_t res_size, uint8_t min, uint8_t max);

// generate a random positive float
float generate_random_positive_float ();
float generate_random_float_minus255_255 ();
int16_t generate_random_int16t_between_minus255_255 ();

// Generate a set of files with sizes between width_min*height_min to width_max*height_max, which
// contain mainly color values with value main_val, and a few color values few_val This is an edge
// case, which may provoke differences between the results of different implementations. It returns
// an array containing the paths to the generated files
char** generate_main_with_few (uint32_t width_min,
                               uint32_t width_max,
                               uint32_t height_min,
                               uint32_t height_max,
                               uint8_t main_val,
                               uint8_t few_val,
                               float percentage,
                               char* res_prefix,
                               char* res_postfix,
                               uint16_t image_count);

// Generate a set of files with sizes between width_min*height_min to width_max*height_max, which
// contain random color values between pixel_min and pixel_max Returns array with paths to th
// generated files
char** generate_random_benchmark_dataset (uint32_t width_min,
                                          uint32_t width_max,
                                          uint32_t height_min,
                                          uint32_t height_max,
                                          uint8_t pixel_min,
                                          uint8_t pixel_max,
                                          char* res_prefix,
                                          char* res_postfix,
                                          uint16_t image_count);
// free the memory of a char** array
void free_string_array (char** arr, uint32_t arr_size);

// reading first image of a .ppm file
Image* read_single_ppm (const char* filename);

// free a Image struct
void free_image (Image* img);

// Function that compares two implementations of brightness contrast (signature
// defined in brightness_contrast_implementation_signature.h) by executing the
// function compare_implementations_on_single_case with all possible
// combinations of the parameter values specified in the input parameters For
// every parameter the function gets an array which contains the inputs the
// brightness contrast implementations should be tested with. This function is
// primarily for debugging implementations, by comparing them to simpler and bug
// free implementations ( e.g. the basic implementation)
void compare_implementations_on_multiple_testcases (
    brightness_contrast_implementation implementation_one,
    brightness_contrast_implementation implementation_two,
    int16_t treshold,
    uint8_t** images,
    size_t* widths,
    size_t* heights,
    size_t max_size,
    size_t images_size,
    float* as,
    size_t as_size,
    float* bs,
    size_t bs_size,
    float* cs,
    size_t cs_size,
    int16_t* brightnesses,
    size_t brightnesses_size,
    float* contrasts,
    size_t contrasts_size);

// Function to compute a deviation distribution over all files, that are
// specified in the file file_names_file. Every file is tested with sample_size
// many random input constellations for the other parameters. The resulting
// deviation is written to .csv file.
void compute_deviation_distribution_file_list (
    brightness_contrast_implementation reference_implementation,
    brightness_contrast_implementation deviating_implementation,
    char* file_names_file,
    uint8_t filename_length,
    char* prefix,
    size_t max_size,
    uint32_t sample_size,
    char* deviation_array_out_file);
#endif // TESTING_H



#include "../brightness_contrast_implementation_signature.h"
#include "../process_ppm.h"

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define RANDOM_DATASET_SEED 0
#define RANDOM_DEVIATION_SEED 0

// Method using rand() from stdlib.h to generate an array of random uint8 values
// starting at res with size res_size
void generate_random_valid_image_array (uint8_t* res, size_t res_size, uint8_t min, uint8_t max)
{
    for (uint64_t i = 0; i < res_size; ++i)
    {
        res[i] = min + rand () % (max - min + 1);
    }
}

void generate_same_value_array (uint8_t* res, size_t res_size, uint8_t val)
{
    for (uint64_t i = 0; i < res_size; ++i)
    {
        res[i] = val;
    }
}

// generate a random positive float
float generate_random_positive_float ()
{
    float res;
    do
    {
        res = rand () * exp2 (127 - rand () % 256);
    } while (isinf (res) || isnan (res));
    return res;
}

float generate_random_float_minus255_255 ()
{
    return 255 - (510 * (rand () / (float) RAND_MAX));
}

int16_t generate_random_int16t_between_minus255_255 ()
{
    return (255) - (rand () % 510);
}

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
                               uint16_t image_count)
{
    Image* res_image = malloc (sizeof (Image));
    if (res_image == NULL)
    {
        perror ("Memory for res_image could not be allocated");
        return NULL;
    }

    res_image->pixel_liste = malloc (width_max * height_max * 3);
    if (res_image->pixel_liste == NULL)
    {
        free (res_image);
        perror ("Memory for res_image->pixel_liste could not be allocated");
        return NULL;
    }

    char** file_name_list = malloc (image_count * sizeof (char*));
    if (file_name_list == NULL)
    {
        perror ("Memory for file_name list could not be allocated");
        free (res_image->pixel_liste);
        free (res_image);
        return NULL;
    }

    float width_step_size  = 0;
    float height_step_size = 0;

    // check to prevent deviation by zero
    if (image_count > 1)
    {
        width_step_size  = (width_max - width_min) / ((float) image_count - 1);
        height_step_size = (height_max - height_min) / ((float) image_count - 1);
    }

    // generate
    for (int i = 0; i < image_count; ++i)
    {
        res_image->width  = width_min + width_step_size * i;
        res_image->height = height_min + height_step_size * i;

        generate_same_value_array (res_image->pixel_liste,
                                   (uint64_t) res_image->width * (uint64_t) res_image->height * 3,
                                   main_val);

        for (uint32_t j = 0;
             j
             < percentage * 0.01 * ((uint64_t) res_image->width * (uint64_t) res_image->height * 3);
             j++)
        {
            res_image
                ->pixel_liste[rand ()
                              % ((uint64_t) res_image->width * (uint64_t) res_image->height * 3)] =
                few_val;
        }

        // allocate memory for file name
        uint16_t num_length = 25; // uint16t i has maximum of 10 characters, pixel_min and pixel_max
                                  // maximum of 3 characters, 4 underscores, 7 letters: 10 + 3 + 3
                                  // +2 +7 = 25
        char* outname =
            malloc ((strlen (res_postfix) + strlen (res_prefix) + num_length + 1) * sizeof (char));
        if (outname == NULL)
        {
            perror ("Outname memory could not be allocated");
            return NULL;
        }
        char num[num_length];

        // generate filename
        sprintf (num, "%d_%d_%d_main_few", i, main_val, few_val);
        strcpy (outname, res_prefix);
        strcat (outname, num);
        strcat (outname, res_postfix);
        write_ppm (outname, res_image);
        file_name_list[i] = outname;
    }

    free (res_image->pixel_liste);
    free (res_image);
    return file_name_list;
}

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
                                          uint16_t image_count)
{
    Image* res_image = malloc (sizeof (Image));
    if (res_image == NULL)
    {
        fprintf (stderr, "memory for res_image could not be allocated");
        return NULL;
    }

    res_image->pixel_liste = malloc (width_max * height_max * 3);
    if (res_image->pixel_liste == NULL)
    {
        free (res_image);
        perror ("Memory for res_image->pixel_liste could not be allocated");
        return NULL;
    }

    char** file_name_list = malloc (image_count * sizeof (char*));
    if (file_name_list == NULL)
    {
        perror ("Memory for file_name list could not be allocated");
        return NULL;
    }

    float width_step_size  = 0;
    float height_step_size = 0;

    if (image_count > 1)
    {
        width_step_size  = (width_max - width_min) / ((float) image_count - 1);
        height_step_size = (height_max - height_min) / ((float) image_count - 1);
    }

    for (int i = 0; i < image_count; ++i)
    {
        res_image->width  = width_min + width_step_size * i;
        res_image->height = height_min + height_step_size * i;
        if (pixel_max == pixel_min)
        {
            generate_same_value_array (
                res_image->pixel_liste,
                (uint64_t) res_image->width * (uint64_t) res_image->height * 3,
                pixel_min);
        }
        else
        {
            generate_random_valid_image_array (
                res_image->pixel_liste,
                (uint64_t) res_image->width * (uint64_t) res_image->height * 3,
                pixel_min,
                pixel_max);
        }

        // allocate memory for file name
        uint16_t num_length = 18; // uint16t i hat maximal 10 Stellen, pixel_min und pixel_max
                                  // maximal 3 stellen, 2 Unterstriche 10 + 3 + 3 +2
        char* outname = malloc ((strlen (res_postfix) + strlen (res_prefix) + num_length + 1)
                                * sizeof (char)); // maximale Stringlänge + NULL Byte
        if (outname == NULL)
        {
            perror ("Outname memory could not be allocated");
            free (res_image->pixel_liste);
            free (res_image);
            free (file_name_list);
            return NULL;
        }

        char num[num_length];

        // generate filename the generated image is written to
        sprintf (num, "%d_%d_%d", i, pixel_min, pixel_max);
        strcpy (outname, res_prefix);
        strcat (outname, num);
        strcat (outname, res_postfix);
        write_ppm (outname, res_image);
        file_name_list[i] = outname;
    }
    free (res_image->pixel_liste);
    free (res_image);
    return file_name_list;
}

// free the memory of a char** array
void free_string_array (char** arr, uint32_t arr_size)
{
    for (uint32_t i = 0; i < arr_size; ++i)
    {
        free (arr[i]);
    }
    free (arr);
}

// reading first image of a .ppm file
Image* read_single_ppm (const char* filename)
{
    FILE* file_stream = fopen (filename, "r");
    if (file_stream == NULL)
    {
        return NULL;
    }
    return new_image (file_stream);
}

// free a Image struct
void free_image (Image* img)
{
    free (img->pixel_liste);
    free (img);
};

// function that executes two implementations of brightness contrast (signature
// defined in brightness_contrast_implementation_signature.h) and compares the
// results pixel by pixel in case that the results differ it prints some
// information on how the pictures differ
static void
compare_implementations_on_single_case (brightness_contrast_implementation implementation_one,
                                        brightness_contrast_implementation implementation_two,
                                        uint8_t* result_one,
                                        uint8_t* result_two,
                                        int16_t treshold,
                                        uint8_t* image,
                                        int image_id,
                                        size_t width,
                                        size_t height,
                                        float a,
                                        float b,
                                        float c,
                                        int16_t brightness,
                                        float contrast)
{
    implementation_one (image, width, height, a, b, c, brightness, contrast, result_one);
    implementation_two (image, width, height, a, b, c, brightness, contrast, result_two);

    uint8_t correct                = 1;
    uint64_t sum                   = 0;
    uint64_t differing_pixel_count = 0;
    int16_t max_dif                = 0;
    for (uint64_t i = 0; i < width * height; ++i)
    {
        if (abs (result_two[i] - result_one[i]) > (treshold))
        {
            correct = 0;
            differing_pixel_count += 1;
            int dif = result_two[i] - result_one[i];
            sum += abs (dif);
            if (abs (dif) > abs (max_dif))
                max_dif = dif;
        }
    }

    double average_over_faults = sum / (double) (differing_pixel_count);
    double average             = sum / (double) (width * height);

    if (correct == 0)
    {
        // write results to files if they aren't equal to each other
        // set a gdb breakpoint in this if statement, to open debugger when correct
        // == 0
        write_pgm ("implementation_one_result.pgm", result_one, width, height, 0);
        write_pgm ("implementation_two_result.pgm", result_two, width, height, 0);

        // print error information
        printf (
            "Results: maximum difference:  %d\t, average difference over differing "
            "pixels: %f\t, average over all pixels: %f\t, absolut differing "
            "pixels: %lu\t, differing pixels relative to size: %f\t on image id %d "
            "with inputs height: %zu, width: %zu, a: %f, b: %f, c: "
            "%f, brightness: %d, contrast: %f.\n ",
            max_dif,
            average_over_faults,
            average,
            differing_pixel_count,
            differing_pixel_count / (double) (width * height),
            image_id,
            height,
            width,
            a,
            b,
            c,
            brightness,
            contrast);

        // rerun implementations to debug step by step with debugger, if you add a
        // debugger breakpoint here
        implementation_one (image, width, height, a, b, c, brightness, contrast, result_one);
        implementation_two (image, width, height, a, b, c, brightness, contrast, result_two);
    }
}

// Function that compares two implementations of brightness contrast (signature
// defined in brightness_contrast_implementation_signature.h) by executing the
// function compare_implementations_on_single_case with all possible
// combinations of the parameter values specified in the input parameters. For
// every parameter the function gets an array which contains the inputs the
// brightness contrast implementations should be tested with. This function is
// primarily for debugging implementations, by comparing them to simpler and bug
// free implementations ( e.g. the basic implementation). The threshold parameter specifies
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
    size_t contrasts_size)
{
    uint8_t* result_one = malloc (max_size);
    if (result_one == NULL)
    {
        perror ("Memory for result of implementation one could not be allocated");
        return;
    }
    uint8_t* result_two = malloc (max_size);
    if (result_two == NULL)
    {
        perror ("Memory for result of implementation two could not be allocated");
        free (result_one);
        return;
    }

    if (treshold < 0)
        treshold = 0;

    // test every possible combination of the specified inputs
    for (size_t image_index = 0; image_index < images_size; ++image_index)
    {
        printf ("%lu\n", image_index);
        for (size_t a_index = 0; a_index < as_size; ++a_index)
        {
            for (size_t b_index = 0; b_index < bs_size; ++b_index)
            {
                for (size_t c_index = 0; c_index < cs_size; ++c_index)
                {
                    for (size_t brightness_index = 0; brightness_index < brightnesses_size;
                         ++brightness_index)
                    {
                        for (size_t contrasts_index = 0; contrasts_index < contrasts_size;
                             ++contrasts_index)
                        {
                            compare_implementations_on_single_case (implementation_one,
                                                                    implementation_two,
                                                                    result_one,
                                                                    result_two,
                                                                    treshold,
                                                                    images[image_index],
                                                                    image_index,
                                                                    widths[image_index],
                                                                    heights[image_index],
                                                                    as[a_index],
                                                                    bs[b_index],
                                                                    cs[c_index],
                                                                    brightnesses[brightness_index],
                                                                    contrasts[contrasts_index]);
                        }
                    }
                }
            }
        }
    }

    free (result_one);
    free (result_two);
}

// Function that executes two implementations of brightness contrast with
// specified parameters, and compares them pixel by pixel. The deviation array,
// passed as pointer, contains a counter for every possible pixel deviation
// [-255, 255]. A counter is increased, when this deviation occurs in a pixel.
// In this way it calculates a deviation distribution.
static void test_implementations_against_single_deviation (
    brightness_contrast_implementation reference_implementation,
    brightness_contrast_implementation deviating_implementation,
    uint64_t* deviation_array,
    uint8_t* reference_result,
    uint8_t* deviating_result,
    uint8_t* image,
    size_t width,
    size_t height,
    float a,
    float b,
    float c,
    int16_t brightness,
    float contrast)
{
    // execute implementation
    reference_implementation (
        image, width, height, a, b, c, brightness, contrast, reference_result);
    deviating_implementation (
        image, width, height, a, b, c, brightness, contrast, deviating_result);

    for (uint64_t i = 0; i < width * height; ++i)
    {
        int16_t deviation = deviating_result[i] - reference_result[i];
        deviation_array[deviation + 255] += 1;
    }
}

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
    char* deviation_array_out_file)
{
    uint8_t* image_array_in;
    uint8_t* reference_result;
    uint8_t* deviating_result;
    uint64_t* deviation_array;
    FILE* file_name_file_stream;
    FILE* file;

    image_array_in = malloc (sizeof (uint8_t) * max_size * 3);
    if (image_array_in == NULL)
    {
        perror ("Memory for input image could not be allocated");
        return;
    }

    reference_result = malloc (sizeof (uint8_t) * max_size);
    if (reference_result == NULL)
    {
        free (image_array_in);
        perror ("Memory for reference results could not be allocated");
        return;
    }
    deviating_result = malloc (sizeof (uint8_t) * max_size);
    if (deviating_result == NULL)
    {
        free (image_array_in);
        free (reference_result);
        perror ("Memory for deviating results could not be allocated");
        return;
    }

    deviation_array = calloc ((511), sizeof (uint64_t));
    if (deviation_array == NULL)
    {
        free (image_array_in);
        free (reference_result);
        free (deviating_result);
        perror ("Memory for deviation array could not be allocated");
        return;
    }

    file_name_file_stream = fopen (file_names_file, "r");
    if (file_name_file_stream == NULL)
    {
        free (image_array_in);
        free (reference_result);
        free (deviating_result);
        free (deviation_array);
        perror ("File with filenames could not be opened");
        return;
    }

    Image* img = NULL;
    char filename_line[filename_length];
    char path[filename_length + strlen (prefix)];
    uint64_t sum = 0;
    srand (RANDOM_DEVIATION_SEED);

    while (fgets (filename_line, sizeof (filename_line), file_name_file_stream))
    {
        // construct path and read ppm file
        filename_line[strlen (filename_line) - 1] = '\0';
        strcpy (path, prefix);
        strcat (path, filename_line);
        img = read_single_ppm (path);
        printf ("Processed File: %s\n", path);

        if (img == NULL)
        {
            fprintf (stderr, "Failed to open file %s: %s\n", path, strerror (errno));
            continue;
        }

        for (uint32_t i = 0; i < sample_size; ++i)
        {
            float a            = generate_random_positive_float ();
            float b            = generate_random_positive_float ();
            float c            = generate_random_positive_float ();
            int16_t brightness = generate_random_int16t_between_minus255_255 ();
            float contrast     = generate_random_float_minus255_255 ();

            sum += img->width * img->height;

            test_implementations_against_single_deviation (reference_implementation,
                                                           deviating_implementation,
                                                           deviation_array,
                                                           reference_result,
                                                           deviating_result,
                                                           img->pixel_liste,
                                                           img->width,
                                                           img->height,
                                                           a,
                                                           b,
                                                           c,
                                                           brightness,
                                                           contrast);
        }
    }

    file = fopen (deviation_array_out_file, "w");
    if (file == NULL)
    {
        free (image_array_in);
        free (reference_result);
        free (deviating_result);
        free (deviation_array);
        perror ("File for writing deviation distribution could not be opened");
        return;
    }
    fprintf (file, "%lu\n", sum);
    for (int i = 0; i < 511; ++i)
    {
        fprintf (file, "%d,%lu\n", i - 255, deviation_array[i]);
    }

    free (reference_result);
    free (deviating_result);
    free (deviation_array);
    if (img != NULL)
    {
        free (img->pixel_liste);
        free (img);
    }
    free (image_array_in);
    fclose (file);
}
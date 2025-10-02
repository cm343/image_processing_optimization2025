// Standardlib Imports
#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Implementierungs Imports
#include "avx_instruction.h"
#include "basic_implementation.h"
#include "basic_implementation_with_basic_instructions.h"
#include "non_simd_optimized.h"
#include "sse_instruction_optimized_float.h"
#include "sse_instruction_optimized_integer.h"

// Helper Function Imports
#include "benchmark.h"
#include "brightness_contrast_implementation_signature.h"
#include "common.h"
#include "process_ppm.h"

int main (int argc, char* argv[])
{
    const uint8_t highest_version = 5;
    uint8_t version               = 0;

    int option_index = 0;

    float contrast = 0.;

    uint16_t brightness = 0;

    // Base Values for a, b,c choosen after weighting according to luminance
    float a = 0.299;
    float b = 0.587;
    float c = 0.114;

    uint32_t repetitions = 1;

    char* outputfile_name = NULL;

    int8_t outputfile_name_exist = 0;

    int8_t time_tracking = 0;

    brightness_contrast_implementation func = brightness_contrast_V1;

    int tmp;

    const struct option long_options[] = {
        {"coeffs", required_argument, 0, 'c'},
        {"brightness", required_argument, 0, 'b'},
        {"contrast", required_argument, 0, 'k'},
        {"help", no_argument, 0, 'h'},
    };

    int32_t condition  = 0;
    uint32_t countArgs = 0;
    while ((condition = getopt_long (argc, argv, "V:B::ho:", long_options, &option_index)) != -1)
    {
        switch (condition)
        {
            case 'V':

                // We check if version is set argument is set, only contains
                // digits and if it is a valid version If no version is set we
                // use the predefined value (version = 0)

                if (optarg)
                {
                    if (is_only_digits (optarg))
                    {
                        int tmp_version = strtol (optarg, NULL, 10);
                        if (tmp_version <= highest_version || tmp_version >= 1)
                        {
                            version = tmp_version;
                        }
                    }
                    else
                    {
                        fprintf (stderr,
                                 "You entered a version that contained "
                                 "letters. Our versions are digit only!");
                        if (outputfile_name_exist)
                        {
                            free (outputfile_name);
                        }
                        return EXIT_FAILURE;
                    }
                }
                break;

            case 'B':

                // We check if B's argument is set, only contains digits and if
                // it is a valid repetion number If no argument for B is set we
                // use the predefined value (B = 1)

                time_tracking = 1;

                if (optarg)
                {
                    if (is_only_digits (optarg))
                    {
                        repetitions = strtol (optarg, NULL, 10);

                        if (repetitions <= 0)
                        {
                            fprintf (stderr,
                                     "Error: Repetition count must be greater "
                                     "than 0.\n");
                            if (outputfile_name_exist)
                            {
                                free (outputfile_name);
                            }
                            return EXIT_FAILURE;
                        }
                    }
                    else
                    {
                        fprintf (stderr,
                                 "You entered a invalid argument for Option "
                                 "-B. It contained letters.");
                        if (outputfile_name_exist)
                        {
                            free (outputfile_name);
                        }
                        return EXIT_FAILURE;
                    }
                }
                break;

            case 'c':

                // We check if coeffs argument is set, only contains digits,
                // dots or minus and if it is a valid version If no coeffs is
                // set we use the predefined value (a = 0.299, b = 0.587, c =
                // 0.114)

                if (optarg)
                {
                    int commas = count_commas (optarg);
                    if (commas != 2)
                    {
                        fprintf (stderr,
                                 "Es wurde ein nicht valide Anzahl an "
                                 "Argumenten für die Option --coeffs "
                                 "angegeben: %d\n",
                                 commas);
                        if (outputfile_name_exist)
                        {
                            free (outputfile_name);
                        }
                        return EXIT_FAILURE;
                    }

                    char* token;

                    // a
                    token = strtok (optarg, ",");

                    if (is_only_digit_or_minus_or_dot (token))
                        a = strtof (token, NULL);
                    else
                    {
                        if (outputfile_name_exist)
                        {
                            free (outputfile_name);
                        }
                        fprintf (stderr,
                                 "Your value for a is not valid. It contained "
                                 "non numeric characters: %s\n",
                                 token);
                        return EXIT_FAILURE;
                    }

                    // b
                    token = strtok (NULL, ",");

                    if (is_only_digit_or_minus_or_dot (token))
                        b = strtof (token, NULL);
                    else
                    {
                        if (outputfile_name_exist)
                        {
                            free (outputfile_name);
                        }
                        fprintf (stderr,
                                 "\nYour value for b is not valid. It "
                                 "contained non numeric characters: %s\n",
                                 token);
                        return EXIT_FAILURE;
                    }

                    // c
                    token = strtok (NULL, ",");

                    if (is_only_digit_or_minus_or_dot (token))
                        c = strtof (token, NULL);
                    else
                    {
                        if (outputfile_name_exist)
                        {
                            free (outputfile_name);
                        }
                        fprintf (stderr,
                                 "\nYour value for c is not valid. It "
                                 "contained non numeric characters: %s\n",
                                 token);
                        return EXIT_FAILURE;
                    }

                    if (a < 0 || b < 0 || c < 0)
                    {
                        if (outputfile_name_exist)
                        {
                            free (outputfile_name);
                        }
                        fprintf (stderr,
                                 "One of the parameters a, b or c was negative "
                                 "which isn't defined for our implementation. "
                                 "Please choose the values for a, b, c >= 0");
                        return EXIT_FAILURE;
                    }
                }
                break;

            case 'b':

                // We check if brightness argument is set, only contains digits
                // or minus and if it is a valid brightness value If no
                // brightness is set we use the predefined value (brightness =
                // 0)

                if (optarg)
                {
                    if (is_only_digit_or_minus (optarg))
                    {
                        int pre_check_brightness = strtol (optarg, NULL, 10);

                        if (pre_check_brightness <= 255 && pre_check_brightness >= -255)
                        {
                            brightness = pre_check_brightness;
                        }
                        else
                        {
                            fprintf (stderr,
                                     "Your brithness was not inside the "
                                     "interval. It was either larger than 255 "
                                     "or less than -255\n");
                            if (outputfile_name_exist)
                            {
                                free (outputfile_name);
                            }
                            return EXIT_FAILURE;
                        }
                    }
                }
                break;

            case 'k':

                // We check if contrast argument is set, only contains digits,
                // dots or minus and if it is a valid contrast If no contrast is
                // set we use the predefined value (contrast)

                if (optarg)
                {
                    if (is_only_digit_or_minus_or_dot (optarg))
                    {
                        int pre_check_contrast = strtof (optarg, NULL);
                        if (pre_check_contrast <= 255.0 && (pre_check_contrast >= -255.0))
                        {
                            contrast = pre_check_contrast;
                        }
                        else
                        {
                            fprintf (stderr,
                                     "Your contrast was not inside the "
                                     "interval. It was either larger than 255 "
                                     "or less than -255\n");
                            if (outputfile_name_exist)
                            {
                                free (outputfile_name);
                            }
                            return EXIT_FAILURE;
                        }
                    }
                }

                break;

            case 'o':

                // We check if outputname argument is set and if it ends with
                // .pgm If it not ends with .pgm we change the outputname so it
                // ends with .pgm If no coeffs is set we expect that no output
                // is wanted and do not produce an outputfile

                if (optarg)
                {
                    if (outputfile_name)
                    {
                        free (outputfile_name);
                    }
                    char* dot    = strrchr (optarg, '.');
                    uint32_t len = strlen (optarg);

                    // Check if outputfilename is contains file extension and if
                    // it is pgm
                    if (!dot || strcmp (dot, ".pgm") != 0)
                    {
                        outputfile_name = malloc (len + 5);
                        if (!outputfile_name)
                        {
                            fprintf (stderr,
                                     "Error: Memory allocation of outputfile "
                                     "name failed.\n");
                            return EXIT_FAILURE;
                        }

                        if (dot)
                        {
                            *dot = '\0';
                        }
                        snprintf (outputfile_name, len + 5, "%s.pgm", optarg);
                    }
                    else
                    {
                        outputfile_name = malloc (len + 2);
                        if (!outputfile_name)
                        {
                            fprintf (stderr,
                                     "Error: Memory allocation of outputfile "
                                     "name failed.\n");
                            return EXIT_FAILURE;
                        }

                        strcpy (outputfile_name, optarg);
                    }

                    outputfile_name_exist = 1;
                }
                break;
            case 'h':

                print_helpmessage ();
                if (outputfile_name_exist)
                {
                    free (outputfile_name);
                }
                return EXIT_SUCCESS;

            default:
                fprintf (stderr,
                         "This option isn't supported. Please use -h or --help "
                         "to see the available options.");
                if (outputfile_name_exist)
                {
                    free (outputfile_name);
                }
                return EXIT_FAILURE;
        }
        countArgs++;
    }

    // get positional Arguments
    char* inputfile_name = NULL;
    if (optind < argc)
    {
        inputfile_name = argv[optind];
    }

    if (inputfile_name == NULL)
    {
        fprintf (stderr, "There was no inputfile submitted to the program.\n");
        if (outputfile_name_exist)
        {
            free (outputfile_name);
        }
        return EXIT_FAILURE;
    }

    print_params (version, repetitions, a, b, c, brightness, contrast, inputfile_name);
    if (outputfile_name_exist)
    {
        printf ("Outputfile Name: %s\n", outputfile_name);
    }

    FILE* input_image = NULL;

    errno       = 0;
    input_image = fopen (inputfile_name, "rb");

    // fopen returns NULL if a error occured (e.g. FileNotFound)
    if (input_image == NULL || errno != 0)
    {
        fprintf (stderr, "An error file opening: %s\n", strerror (errno));
        if (outputfile_name_exist)
        {
            free (outputfile_name);
        }
        return EXIT_FAILURE;
    }

    switch (version)
    {
        case 0:
            func = brightness_contrast_V1;
            break;
        case 1:
            func = brightness_contrast_V2;
            break;
        case 2:
            func = brightness_contrast_V3;
            break;
        case 3:
            func = brightness_contrast_V4;
            break;
        case 4:
            if (is_avx_supported ())
            {
                func = brightness_contrast_V5;
            }
            else
            {
                fprintf (stderr,
                         "AVX2 is not supported on that system. We therefore "
                         "use SSE-Version (V4). ");
                func = brightness_contrast_V4;
            }

            break;
        default:
            fprintf (stderr,
                     "Die Angegebene Version passt zu keiner der vorhandenen "
                     "Funktionen!!!\n");
            if (outputfile_name_exist)
            {
                free (outputfile_name);
            }
            fclose (input_image);
            return EXIT_FAILURE;
    }

    double total_average_execution_time = 0;
    uint8_t not_first                   = 0;
    uint64_t img_count                  = 0;

    // We go through the complete file to ensure that if there are multiple
    // valid images in one file that we catch all.

    while (!feof (input_image))
    {
        errno = 0;

        Image* img = new_image (input_image);

        if (img == NULL || errno != 0)
        {
            if (errno != 0)
                fprintf (stderr, "Error during file read: %s\n", strerror (errno));
            if (outputfile_name_exist)
            {
                free (outputfile_name);
            }
            fclose (input_image);
            return EXIT_FAILURE;
        }

        if (img)
        {
            img_count++;

            printf ("\n%ld.Image\n", img_count);
            printf ("Max Val: %d\n", img->max_val);
            printf ("height: %ld\n", img->height);
            printf ("width: %ld\n", img->width);

            uint8_t* result = NULL;

            result = malloc (img->height * img->width);

            if (!(result))
            {
                fprintf (stderr, "Error allocating memory for the Result Image\n");
                if (outputfile_name_exist)
                {
                    free (outputfile_name);
                }
                free (img->pixel_liste);
                free (img);
                fclose (input_image);
                return EXIT_FAILURE;
            }

            if (repetitions > 1)
            {
                total_average_execution_time +=
                    measure_execution_time_multiple_total (repetitions,
                                                           func,
                                                           img->pixel_liste,
                                                           img->width,
                                                           img->height,
                                                           a,
                                                           b,
                                                           c,
                                                           brightness,
                                                           contrast,
                                                           result);
            }
            else
            {
                total_average_execution_time += measure_execution_time_single (func,
                                                                               img->pixel_liste,
                                                                               img->width,
                                                                               img->height,
                                                                               a,
                                                                               b,
                                                                               c,
                                                                               brightness,
                                                                               contrast,
                                                                               result);
            }

            if (outputfile_name_exist)
            {
                errno = 0;

                if (not_first)
                {
                    write_pgm (outputfile_name, result, img->width, img->height, 1);
                }
                else
                {
                    not_first++;
                    write_pgm (outputfile_name, result, img->width, img->height, 0);
                }
                if (errno != 0)
                {
                    fprintf (stderr, "Error during file write: %s\n", strerror (errno));
                    if (outputfile_name_exist)
                    {
                        free (outputfile_name);
                    }
                    free (img->pixel_liste);
                    free (img);
                    free (result);
                    fclose (input_image);
                    return EXIT_FAILURE;
                }
            }

            free (img->pixel_liste);
            free (img);
            free (result);
        }
        else
        {
            fprintf (stderr, "Error during execution: Invalid Format\n");
            fclose (input_image);
            if (outputfile_name_exist)
            {
                free (outputfile_name);
            }
            return EXIT_FAILURE;
        }

        // Check if there is more data in the file
        tmp           = fgetc (input_image);
        int extraData = fgetc (input_image);
        if (extraData != EOF)
        {
            ungetc (extraData, input_image);
            ungetc (tmp, input_image);

            // Check if there are other images in the file
            char next_magic_number[3];
            if (fscanf (input_image, "%2s", next_magic_number) == 1
                && strcmp (next_magic_number, "P6") == 0)
            {
                // Another picture was found in the file
                ungetc (next_magic_number[1], input_image);
                ungetc (next_magic_number[0], input_image);
                continue;
            }
            else
            {
                fprintf (stderr,
                         "Invalid Format because image has more Pixel than "
                         "specified in the header.\n");
                fclose (input_image);
                if (outputfile_name_exist)
                {
                    free (outputfile_name);
                }
                return EXIT_FAILURE;
            }
        }
        else
        {
            break;
        }
    }

    fclose (input_image);

    if (outputfile_name_exist)
    {
        free (outputfile_name);
    }

    if (time_tracking)
    {
        total_average_execution_time = total_average_execution_time / (img_count * repetitions);
        printf ("\nEXECUTION TIME: %f\n\n", total_average_execution_time);
    }

    return EXIT_SUCCESS;
}
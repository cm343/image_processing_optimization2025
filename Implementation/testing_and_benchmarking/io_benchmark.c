#include "../brightness_contrast_implementation_signature.h"
#include "../process_ppm.h"
#include "benchmark.h"
#include "errno.h"
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
void benchmark_file_list_automatic_iter (char** files,
                                         uint32_t n_files,
                                         uint16_t min_iterations,
                                         char* benchmark_result_file,
                                         brightness_contrast_implementation fun,
                                         float a,
                                         float b,
                                         float c,
                                         int16_t brightness,
                                         float contrast)
{
    FILE* file = fopen (benchmark_result_file, "w");
    if (file == NULL)
    {
        perror ("Unable to open result file: ");
        return;
    }

    for (uint32_t i = 0; i < n_files; ++i)
    {
        // read current image from file
        Image* in = read_single_ppm (files[i]);
        if (in == NULL)
        {
            fprintf (stderr, "Unable to read file with index %d: %s\n", i, strerror (errno));
            fprintf (file, "-1,-1,-1,-1\n");
            continue;
        }

        // allocate memory for the result image
        uint8_t* res = malloc (in->height * in->width);
        if (res == NULL)
        {
            fprintf (stderr,
                     "Memory for result image %d could not be allocated: %s\n",
                     i,
                     strerror (errno));
            continue;
        }

        // pre measurement to compute iterations needed
        double pre_duration = measure_execution_time_multiple_average (
            2, fun, in->pixel_liste, in->width, in->height, a, b, c, brightness, contrast, res);

        uint32_t iterations = 2e3 / pre_duration; // guarantee that the estimated runtime between
                                                  // the measurement points is 2 seconds
        if (iterations < min_iterations)
            iterations = min_iterations; // guarantee a minimum of min_iterations
                                         // iterations per file

        // execute benchmark for current file
        double duration = measure_execution_time_multiple_average (iterations,
                                                                   fun,
                                                                   in->pixel_liste,
                                                                   in->width,
                                                                   in->height,
                                                                   a,
                                                                   b,
                                                                   c,
                                                                   brightness,
                                                                   contrast,
                                                                   res);

        double total_time = iterations * duration; //  that check validity by ensuring runtime
                                                   //  between the
                                                   // timestamps is high enough

        printf ("Benchmarked file %s with duration %f and %d iterations\n",
                files[i],
                duration,
                iterations);

        // write benchmark result to result file in .csv format
        fprintf (file, "%lu,%f,%d,%f\n", in->height * in->width, duration, iterations, total_time);

        free (res);
        free (in->pixel_liste);
        free (in);
    }

    fclose (file);
}
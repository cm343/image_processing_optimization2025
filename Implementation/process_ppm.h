#ifndef PROCESS_PPM_H
#define PROCESS_PPM_H

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    uint64_t width, height;
    uint32_t max_val;
    uint8_t* pixel_liste;
} Image;

void write_ppm (const char* filename, const Image* img);

void write_pgm (const char* filename,
                const uint8_t* img,
                size_t width,
                size_t height,
                uint8_t append);

Image* new_image (FILE* file);

#endif
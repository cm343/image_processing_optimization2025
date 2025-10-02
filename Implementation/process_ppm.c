#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    size_t width, height;
    uint32_t max_val;
    uint8_t* pixel_liste;
} Image;

/**
 * @brief Skips all lines in a given file that start with '#'
 *
 * @param input_file_ptr A file pointer from which to read the image data.
 */
void skip_comments (FILE* input_file_ptr)
{
    int ch;
    while ((ch = fgetc (input_file_ptr)) != EOF)
    {
        if (ch == '#')
        {
            while ((ch = fgetc (input_file_ptr)) != '\n' && ch != EOF)
                ;
        }
        else if (!isspace (ch))
        {
            ungetc (ch, input_file_ptr);
            break;
        }
    }
}

/**
 * @brief Creates a new Image structure from a given file.
 *
 * @param file A file pointer from which to read the image data.
 * @return A pointer to the newly created Image structure.
 */
Image* new_image (FILE* file)
{
    // check if it is an P6 ppm image
    char magic_num[3] = {0};

    if (fscanf (file, "%2s", magic_num) != 1)
    {
        fprintf (stderr, "Error reading magic number.\n");
        return NULL;
    }

    if (magic_num[0] != 'P' || magic_num[1] != '6')
    {
        fprintf (stderr, "Invalid PPM Format. Required Format is 24bpp PPM (P6).\n");
        return NULL;
    }

    // Because there could be comments in the header we need to skip them
    skip_comments (file);

    // read the Meta Data from header
    int64_t tmp_width, tmp_height;
    size_t width, height;
    int32_t max_val_tmp;
    uint32_t max_val;
    if (fscanf (file, "%ld %ld", &tmp_width, &tmp_height) != 2)
    {
        if (errno != 0)
        {
            fprintf (stderr, "Error during width and height read: %s\n", strerror (errno));
        }
        else
        {
            fprintf (stderr, "Error during width and height read. Invalid Format.\n");
        }
        return NULL;
    }

    int tmp;
    if (!isspace ((tmp = fgetc (file))))
    { //!='\n'){
        fprintf (stderr,
                 "Error: Invalid Format. No whitespace after height or decimal "
                 "height.\n");
        return NULL;
    }
    else
    {
        ungetc (tmp, file);
    }

    if (tmp_width <= 0)
    {
        fprintf (stderr, "Not supported width. Width and height have to be >= 1\n");
        return NULL;
    }
    else if (tmp_height <= 0)
    {
        fprintf (stderr, "Not supported height. Width and height have to be >= 1\n");
        return NULL;
    }
    else
    {
        width  = (size_t) tmp_width;
        height = (size_t) tmp_height;
    }

    // There could also be comments after the with and height line
    skip_comments (file);

    if (fscanf (file, "%d", &max_val_tmp) != 1)
    {
        fprintf (stderr, "Error reading max value\n");
        return NULL;
    }

    if (max_val_tmp <= 0)
    {
        fprintf (stderr,
                 "Not supported Max Value.  Max Value have to be 1 <= Max "
                 "Value <= 255\n");
        return NULL;
    }
    else if (max_val_tmp > 255)
    {
        fprintf (stderr,
                 "Not supported Max Value. Max Value have to be 1 <= Max Value "
                 "<= 255\n");
        return NULL;
    }
    else
    {
        max_val = (uint32_t) max_val_tmp;
    }

    // checks if after the max_value comes a whitespace --> checking format
    if (!isspace ((tmp = fgetc (file))))
    {
        fprintf (stderr, "Error: No whitespace after Max Value\n");
        return NULL;
    }
    else
    {
        ungetc (tmp, file);
    }

    // creates a new Image struct
    Image* img = malloc (sizeof (Image));
    if (!(img))
    {
        fprintf (stderr, "Error allocating memory for the Image Object\n");
        return NULL;
    }

    img->width   = width;
    img->height  = height;
    img->max_val = max_val;

    uint64_t number_of_pixels = 0;
    // uint8_t tmp = 3;

    if (__builtin_umull_overflow (width, height, &number_of_pixels))
    {
        fprintf (stderr,
                 "Picture height and width would cause an overflow. Invalid "
                 "Value Combination.\n");
        free (img);
        return NULL;
    }
    if (__builtin_umull_overflow (number_of_pixels, 3, &number_of_pixels))
    {
        fprintf (stderr,
                 "Picture height and width would cause an overflow. Invalid "
                 "Value Combination.\n");
        free (img);
        return NULL;
    }

    // hier muss noch ein overflow check hin
    img->pixel_liste = malloc (number_of_pixels);

    if (!(img->pixel_liste))
    {
        fprintf (stderr, "Error allocating memory for the pixels\n");
        free (img);
        return NULL;
    }

    size_t index = 0;

    fgetc (file); // absorb '\n'

    if (max_val <= 255)
    {
        uint8_t color_val;

        while (!feof (file) && index < number_of_pixels)
        {
            if (fread (&color_val, sizeof (uint8_t), 1, file) != 1)
            {
                fprintf (stderr, "Error during reading occured: fread\n");
                fprintf (stderr,
                         "Probably not enough pixels in data section of "
                         "ppm.\n");
                free (img->pixel_liste);
                free (img);
                return NULL;
            }

            if (max_val != 255)
            {
                // overflow check
                uint8_t tmp = (uint8_t) round ((color_val / max_val) * 255.);
                color_val   = tmp;
            }

            if (color_val > max_val)
            {
                fprintf (stderr,
                         "Error while reading. Color Value was higher than max "
                         "Value.\n");
                free (img->pixel_liste);
                free (img);
                return NULL;
            }

            img->pixel_liste[index++] = color_val;
        }
    }

    if (index < (number_of_pixels - 1))
    {
        fprintf (stderr, "Error while reading. Not enough Pixel to read.\n");
        free (img->pixel_liste);
        free (img);
        return NULL;
    }

    return img;
}

/**
 * @brief Writes an image in PPM format to a file.
 *
 * @param filename The name of the file to write the image to.
 * @param img A pointer to the Image structure containing image data.
 */
void write_ppm (const char* filename, const Image* img, uint8_t append)
{
    FILE* output_file_ptr;
    if (append)
    {
        output_file_ptr = fopen (filename, "ab");
    }
    else
    {
        output_file_ptr = fopen (filename, "wb");
    }

    if (!output_file_ptr)
    {
        perror ("Unable to open file\n");
        return;
    }

    fprintf (output_file_ptr,
             "P6\n%ld %ld\n255\n",
             img->width,
             img->height); // es kann sein das hier noch ein zeilenumbruch
                           // hinmuss

    fwrite (img->pixel_liste, sizeof (uint8_t), (img->width) * (img->height) * 3, output_file_ptr);

    fclose (output_file_ptr);
}

/**
 * @brief Writes image data in PGM format to a file.
 *
 * @param filename The name of the file to write the image data to.
 * @param img A pointer to the image data in uint8_t format.
 * @param width The width of the image.
 * @param height The height of the image.
 * @param append A flag to indicate if the data should be appended to the file
 * (non-zero value) or not (zero value).
 */
void write_pgm (const char* filename,
                const uint8_t* img,
                size_t width,
                size_t height,
                uint8_t append)
{
    errno = 0;
    FILE* output_file_ptr;
    if (append)
    {
        output_file_ptr = fopen (filename, "ab");
    }
    else
    {
        output_file_ptr = fopen (filename, "wb");
    }

    if (output_file_ptr == NULL)
    {
        fprintf (stderr, "An error file opening: %s\n", strerror (errno));
        return;
    }

    fprintf (output_file_ptr, "P5\n%ld %ld\n255\n", width, height);

    fwrite (img, sizeof (uint8_t), width * height, output_file_ptr);

    fclose (output_file_ptr);
}

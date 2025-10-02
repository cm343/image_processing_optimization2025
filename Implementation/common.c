#include <cpuid.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Checks if AVX (Advanced Vector Extensions) is supported on the current
 * machine.
 *
 * @return 1 if AVX is supported, 0 otherwise.
 *
 * @see
 * https://android.googlesource.com/platform/prebuilts/gcc/linux-x86/host/x86_64-linux-glibc2.7-4.6/+/02075080d51c371ae87b9898bf84a085e436ee27/lib/gcc/x86_64-linux/4.6.x-google/include/cpuid.h
 * @see
 * https://en.wikipedia.org/wiki/CPUID#EAX=1:_Processor_Info_and_Feature_Bits
 */
int is_avx_supported ()
{
    unsigned eax = 0, ebx = 0, ecx = 0, edx = 0;
    uint32_t avx_bit_mask = (1u << 28);
    if (__get_cpuid (1, &eax, &ebx, &ecx, &edx))
    {
        if (ecx & avx_bit_mask)
        {
            return 1;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        fprintf (stderr,
                 "__get_cpuid not supported. Therefor no check for AVX "
                 "possible.\n");
        return 0;
    }
}

/**
 * @brief Checks if a string contains only digits.
 *
 * @param str The string to check.
 * @return 1 if the string contains only digits, 0 otherwise.
 */
int is_only_digits (const char* str)
{
    /*
        Checks if an abitrary String contains any non numeric characters.
    */
    while (*str != '\0')
    {
        if (!isdigit ((unsigned char) *str))
        {
            return 0;
        }
        str++;
    }
    return 1;
}

/**
 * @brief Checks if a string contains only digits or dots.
 *
 * @param str The string to check.
 * @return 1 if the string contains only digits or dots, 0 otherwise.
 */
int is_only_digit_or_minus (const char* str)
{
    while (*str != '\0')
    {
        unsigned char tmp = (unsigned char) *str;
        if (!isdigit (tmp) && tmp != '-')
        {
            return 0;
        }
        str++;
    }
    return 1;
}

/**
 * @brief Checks if a string contains only digits or minus signs.
 *
 * @param str The string to check.
 * @return 1 if the string contains only digits or minus signs, 0 otherwise.
 */
int is_only_digit_or_dot (const char* str)
{
    uint32_t dot_count = 0;
    while (*str != '\0')
    {
        unsigned char tmp = (unsigned char) *str;
        if ((!isdigit (tmp) && tmp != '.') || dot_count > 1)
        {
            return 0;
        }
        if (tmp == '.')
            dot_count++;
        str++;
    }
    return 1;
}

/**
 * @brief Checks if a string contains only digits, minus sign, or dot.
 *
 * @param str The string to check.
 * @return 1 if the string contains only digits, minus signs, or dots, 0
 * otherwise.
 */
int is_only_digit_or_minus_or_dot (const char* str)
{
    uint32_t dot_count   = 0;
    uint32_t minus_count = 0;
    while (*str != '\0')
    {
        unsigned char tmp = (unsigned char) *str;
        if ((!isdigit (tmp) && tmp != '-' && tmp != '.') || dot_count > 1 || minus_count)
        {
            return 0;
        }
        if (tmp == '.')
            dot_count++;
        if (tmp == '-')
            minus_count++;
        str++;
    }
    return 1;
}

/**
 * @brief Counts the number of commas in a string.
 *
 * @param str The string to check.
 * @return The number of commas in the string.
 */
int count_commas (const char* str)
{
    int count = 0;
    while (*str != '\0')
    {
        if (*str == ',')
        {
            count++;
        }
        str++;
    }
    return count;
}

/**
 * @brief Prints the help message for the program.
 */
void print_helpmessage ()
{
    char helpmessage[] =
        "Usage: programmname [OPTION...] <input_file>\n"
        "\tPerforms a specified image processing operation on the input file "
        "and saves the result in an output file.\n\n"
        "Options:\n"
        "\t-V <Number>              \tSelects the implementation to use. "
        "Default is 0 (main implementation).\n"

        "\t-B[<Number>]             \tMeasures and outputs the runtime of the "
        "specified implementation. Optionally, the number of repetitions can "
        "be specified.\n"

        "\t-o <Filename>            \tSpecifies the filename of the output "
        "file.\n"

        "\t--coeffs <a>,<b>,<c>     \tDetermines the coefficients for "
        "grayscale conversion. If not set, default values are used.\n"

        "\t--brightness <Number>    \tSets the brightness. The value must be "
        "in the range [-255, 255].\n"

        "\t--contrast <Number>      \tSets the contrast. The value must be in "
        "the range [-255, 255].\n"

        "\t-h, --help               \tDisplays this help message and exits the "
        "program.\n\n"
        "Positional Argument:\n"
        "\t<input_file>               \tSpecifies the filename of the input "
        "file.\n\n"

        "Examples:\n"
        "\tprogrammname -V 0 -B10 --coeffs 0.3,0.59,0.11 --brightness 50 "
        "--contrast 100 -o output.pgm input.ppm\n"
        "\tprogrammname --help\n";

    printf ("%s", helpmessage);
}

/**
 * @brief Prints the parameters used by the program.
 *
 * @param version The version of the implementation being used.
 * @param standard_repetitions The number of standard repetitions for the
 * operation.
 * @param a The coefficient 'a' for grayscale conversion.
 * @param b The coefficient 'b' for grayscale conversion.
 * @param c The coefficient 'c' for grayscale conversion.
 * @param brightness The brightness adjustment value.
 * @param contrast The contrast adjustment value.
 * @param inputfile_name The name of the input file.
 */
void print_params (uint8_t version,
                   uint32_t standard_repetitions,
                   float a,
                   float b,
                   float c,
                   uint16_t brightness,
                   float contrast,
                   char* inputfile_name)
{
    printf ("\nIMPLEMENTATION STARTED WITH THE FOLLOWING PARAMETERS:\n");
    printf ("version: %d\n", version);
    printf ("B: %d\n", standard_repetitions);
    printf ("Inputfilename: %s\n", inputfile_name);
    printf ("a: %f\n", a);
    printf ("b: %f\n", b);
    printf ("c: %f\n", c);
    printf ("brightness: %d\n", brightness);
    printf ("contrast: %f\n", contrast);
}
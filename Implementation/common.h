#ifndef COMMON_H
#define COMMON_H

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

int is_only_digits (const char* str);

int is_only_digit_or_dot (const char* str);

int is_only_digit_or_minus (const char* str);

int is_only_digit_or_minus_or_dot (const char* str);

int count_commas (const char* str);

int is_avx_supported ();

void print_helpmessage ();

void print_params (uint8_t version,
                   uint32_t standard_repetitions,
                   float a,
                   float b,
                   float c,
                   uint16_t brightness,
                   float contrast,
                   char* inputfile_name);

#endif
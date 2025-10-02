#ifndef BRIGHTNESS_CONTRAST_IMPLEMENTATION_SIGNATURE_H
#define BRIGHTNESS_CONTRAST_IMPLEMENTATION_SIGNATURE_H

#include <stdint.h>
#include <stdlib.h>

// type to define a signature for the implementations of brightness_contrast
typedef void (*brightness_contrast_implementation) (const uint8_t* img,
                                                    size_t width,
                                                    size_t height,
                                                    float a,
                                                    float b,
                                                    float c,
                                                    int16_t brightness,
                                                    float contrast,
                                                    uint8_t* result);

#endif // BRIGHTNESS_CONTRAST_IMPLEMENTATION_SIGNATURE_H

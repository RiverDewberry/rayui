#ifndef MKIMG_H
#define MKIMG_H

#include <stdint.h>

//makes a png file from a buffer of uint8 for each color and an image width and height
void makePngFromBitmap(
    uint8_t *red,
    uint8_t *green,
    uint8_t *blue,
    uint32_t w,
    uint32_t h,
    char *path
);

#endif

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

#define get_index(im, x, y, c) ((x) + ((im.w) * (y)) + ((im.w) * (im.h) * (c)))
#define get_inbound_index(a, min, max) (((a) > max) ? max : (((a) < min) ? min : (a)))
#define in_bounds(im, x, y, c) ((x) < im.w && (y) < im.h && (c) < im.c)

float get_pixel(image im, int x, int y, int c)
{
    return im.data[get_index(im, get_inbound_index(x, 0, im.w - 1), get_inbound_index(y, 0, im.h - 1)
            , get_inbound_index(c, 0, im.c - 1))];
}

void set_pixel(image im, int x, int y, int c, float v)
{
    if (!in_bounds(im, x, y, c)) return;
    im.data[get_index(im, x, y, c)] = v;
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    memcpy(copy.data, im.data, im.w * im.h * im.c * sizeof(float));
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
    float weights[] = { 0.299, 0.587, 0.114 };

    for (int c = 0; c < im.c; c++) {
        for (int h = 0; h < im.h; h++) {
            for (int w = 0; w < im.w; w++) {
                gray.data[get_index(gray, w, h, 0)] += weights[c] * im.data[get_index(im, w, h, c)];
            }
        }
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    // TODO Fill this in
}

void clamp_image(image im)
{
    // TODO Fill this in
}


// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    // TODO Fill this in
}

void hsv_to_rgb(image im)
{
    // TODO Fill this in
}

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

#define get_index(im, x, y, c) ((x) + ((im).w * (y)) + ((im).w * (im).h * (c)))
#define get_inbound(a, min, max) (((a) > max) ? max : (((a) < min) ? min : (a)))
#define in_bounds(im, x, y, c) ((x) < (im).w && (y) < (im).h && (c) < (im).c)

float get_pixel(image im, int x, int y, int c)
{
    return im.data[get_index(im, get_inbound(x, 0, im.w - 1), get_inbound(y, 0, im.h - 1)
            , get_inbound(c, 0, im.c - 1))];
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
    int image_index = 0;

    for (int c = 0; c < im.c; c++) {
        int gray_index = 0;

        for (int h = 0; h < im.h; h++) {
            for (int w = 0; w < im.w; w++) {
                gray.data[gray_index++] += weights[c] * im.data[image_index++];
            }
        }
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    if (!in_bounds(im, 0, 0, c)) return;

    int index = get_index(im, 0, 0, c);
    for (int h = 0; h < im.h; h++) {
        for (int w = 0; w < im.w; w++) {
            im.data[index++] += v;
        }
    }
}

void scale_image(image im, int c, float v)
{
    if (!in_bounds(im, 0, 0, c)) return;

    int index = get_index(im, 0, 0, c);
    for (int h = 0; h < im.h; h++) {
        for (int w = 0; w < im.w; w++) {
            im.data[index++] *= v;
        }
    }
}

void clamp_image(image im)
{
    int index = 0;
    for (int c = 0; c < im.c; c++) {
        for (int h = 0; h < im.h; h++) {
            for (int w = 0; w < im.w; w++) {
                im.data[index] = get_inbound(im.data[index], 0, 1);
                index++;
            }
        }
    }
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

float calc_hue(float C, float V, float red, float green, float blue) {
    float hue = 0;

    if (V == red) {
        hue = (green - blue) / C;
    } else if (V == green) {
        hue = (blue - red) / C + 2;
    } else if (V == blue) {
        hue = (red - green) / C + 4;
    }

    hue /= 6;
    if (hue < 0) {
        return hue + 1;
    }
    return hue;
}

void rgb_to_hsv(image im)
{
    for (int h = 0; h < im.h; h++) {
        for (int w = 0; w < im.w; w++) {
            int redIndex = get_index(im, w, h, 0);
            int greenIndex = get_index(im, w, h, 1);
            int blueIndex = get_index(im, w, h, 2);

            float red = im.data[redIndex];
            float green = im.data[greenIndex];
            float blue = im.data[blueIndex];

            float min = three_way_min(red, green, blue);
            float V = three_way_max(red, green, blue);
            float C = V - min;
            float S = V != 0 ? C / V : 0;
            float H = C != 0 ? calc_hue(C, V, red, green, blue) : 0;

            im.data[blueIndex] = V;
            im.data[greenIndex] = S;
            im.data[redIndex] = H;
        }
    }
}

float color(int n, float hue, float value, float saturation) {
    float k = n + hue * 6;
    while (k > 6) {
        k -= 6;
    }
    float min = three_way_min(k, 4 - k, 1);
    return value - value * saturation * (min > 0 ? min : 0);
}

void hsv_to_rgb(image im)
{
    for (int h = 0; h < im.h; h++) {
        for (int w = 0; w < im.w; w++) {
            int hueIndex = get_index(im, w, h, 0);
            int saturationIndex = get_index(im, w, h, 1);
            int valueIndex = get_index(im, w, h, 2);

            float hue = im.data[hueIndex];
            float saturation = im.data[saturationIndex];
            float value = im.data[valueIndex];

            float red = color(5, hue, value, saturation);
            float green = color(3, hue, value, saturation);
            float blue = color(1, hue, value, saturation);

            im.data[hueIndex] = red;
            im.data[saturationIndex] = green;
            im.data[valueIndex] = blue;
        }
    }
}

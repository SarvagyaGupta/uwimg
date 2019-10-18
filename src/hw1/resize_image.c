#include <math.h>
#include "image.h"
#include <assert.h>

#define get_index(im, x, y, c) ((x) + ((im).w * (y)) + ((im).w * (im).h * (c)))

int find_closest_int(float, int);

float nn_interpolate(image im, float x, float y, int c)
{
    int x_closest = find_closest_int(x, im.w - 1);
    int y_closest = find_closest_int(y, im.h - 1);
    return im.data[get_index(im, x_closest, y_closest, c)];
}

int find_closest_int(float f, int max) {
    if (f <= 0) {
        return 0;
    } else if (f >= max) {
        return max;
    }

    int int_part = (int) f;
    int val = int_part + (f - int_part < 0.5 ? 0 : 1);

    return val > max ? max : val;
}

image nn_resize(image im, int w, int h)
{
    image new_image = make_image(w, h, im.c);

    float x_factor = 1. * im.w / w;
    float x_shift = x_factor / 2.0 - 0.5;

    float y_factor = 1. * im.h / h;
    float y_shift = y_factor / 2.0 - 0.5;

    for (int c = 0; c < im.c; c++) {
        for (int i = 0; i < h; i++) {
            for (int j = 0; j < w; j++) {
                float x = x_factor * j + x_shift;
                float y = y_factor * i + y_shift;
                new_image.data[get_index(new_image, j, i, c)] = nn_interpolate(im, x, y, c);
            }
        }
    }
    return new_image;
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    // TODO
    return 0;
}

image bilinear_resize(image im, int w, int h)
{
    // TODO
    return make_image(1,1,1);
}


#include <math.h>
#include "image.h"
#include <assert.h>

#define func(im, x, y, c, nn) ((nn) ? nn_interpolate((im), (x), (y), (c)) : bilinear_interpolate((im), (x), (y), (c)))

image resize(image, int, int, int);

int find_closest_int(float, int);
float get_contribution(image, int, int, float, float, int);

float nn_interpolate(image im, float x, float y, int c)
{
    int x_closest = find_closest_int(x, im.w - 1);
    int y_closest = find_closest_int(y, im.h - 1);
    return get_pixel(im, x_closest, y_closest, c);
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
    return resize(im, w, h, 1);
}

float bilinear_interpolate(image im, float x, float y, int c)
{
    int x_int = (int) x;
    float x_dec = x - x_int;
    int y_int = (int) y;
    float y_dec = y - y_int;

    return get_contribution(im, x_int, y_int, 1 - x_dec, 1 - y_dec, c)
            + get_contribution(im, x_int + 1, y_int, x_dec, 1 - y_dec, c)
            + get_contribution(im, x_int, y_int + 1, 1 - x_dec, y_dec, c)
            + get_contribution(im, x_int + 1, y_int + 1, x_dec, y_dec, c);
}

float get_contribution(image im, int x, int y, float dx, float dy, int c) {
    return dx * dy * get_pixel(im, x, y, c);
}

image bilinear_resize(image im, int w, int h)
{
    return resize(im, w, h, 0);
}

image resize(image im, int w, int h, int nn) {
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

                set_pixel(new_image, j, i, c, func(im, x, y, c, nn));
            }
        }
    }
    return new_image;
}


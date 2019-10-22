#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

float get_convolved_value(image, image, int, int, int);
float get_gaussian_value(int, int, float);
image superimpose_image(image, image, int);

void l1_normalize(image im)
{
    float sum = 0;
    for (int c = 0; c < im.c; c++) {
        for (int h = 0; h < im.h; h++) {
            for (int w = 0; w < im.w; w++) {
                sum += get_pixel(im, w, h, c);
            }
        }
    }

    for (int c = 0; c < im.c; c++) {
        if (sum != 0) {
            scale_image(im, c, 1.0 / sum);
        } else {
            scale_image(im, c, 0);
        }
    }
}

image make_box_filter(int w)
{
    image filter = make_image(w, w, 1);
    for (int y = 0; y < w; y++) {
        for (int x = 0; x < w; x++) {
            set_pixel(filter, x, y, 0, 1.);
        }
    }
    l1_normalize(filter);
    return filter;
}

image convolve_image(image im, image filter, int preserve)
{
    image filtered_image = copy_image(im);
    for (int c = 0; c < im.c; c++) {
        for (int h = 0; h < im.h; h++) {
            for (int w = 0; w < im.w; w++) {
                set_pixel(filtered_image, w, h, c, get_convolved_value(im, filter, w, h, c));
            }
        }
    }

    if (!preserve) {
        image merged_filtered_image = make_image(filtered_image.w, filtered_image.h, 1);
        for (int c = 0; c < filtered_image.c; c++) {
            for (int h = 0; h < filtered_image.h; h++) {
                for (int w = 0; w < filtered_image.w; w++) {
                    float val = get_pixel(merged_filtered_image, w, h, 0) + get_pixel(filtered_image, w, h, c);
                    set_pixel(merged_filtered_image, w, h, 0, val);
                }
            }
        }
        return merged_filtered_image;
    }

    return filtered_image;
}

float get_convolved_value(image im, image filter, int x, int y, int c) {
    int shift_x = filter.w / 2;
    int shift_y = filter.h / 2;
    int channel = (im.c == filter.c) ? c : 0;

    float sum = 0;
    for (int h = 0; h < filter.h; h++) {
        for (int w = 0; w < filter.w; w++) {
            sum += get_pixel(im, x - shift_x + w, y - shift_y + h, c) * get_pixel(filter, w, h, channel);
        }
    }
    return sum;
}

image make_highpass_filter()
{
    image filter = make_image(3, 3, 1);

    set_pixel(filter, 0, 1, 0, -1);
    set_pixel(filter, 1, 0, 0, -1);
    set_pixel(filter, 1, 1, 0, 4);
    set_pixel(filter, 1, 2, 0, -1);
    set_pixel(filter, 2, 1, 0, -1);

    return filter;
}

image make_sharpen_filter()
{
    image filter = make_image(3, 3, 1);

    set_pixel(filter, 0, 1, 0, -1);
    set_pixel(filter, 1, 0, 0, -1);
    set_pixel(filter, 1, 1, 0, 5);
    set_pixel(filter, 1, 2, 0, -1);
    set_pixel(filter, 2, 1, 0, -1);

    return filter;
}

image make_emboss_filter()
{
    image filter = make_image(3, 3, 1);

    set_pixel(filter, 0, 0, 0, -2);
    set_pixel(filter, 0, 1, 0, -1);
    set_pixel(filter, 1, 0, 0, -1);
    set_pixel(filter, 1, 1, 0, 1);
    set_pixel(filter, 1, 2, 0, 1);
    set_pixel(filter, 2, 1, 0, 1);
    set_pixel(filter, 2, 2, 0, 2);

    return filter;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: TODO

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: TODO

image make_gaussian_filter(float sigma)
{
    int sigma_ = ceil(sigma * 6);
    int size = sigma_ + (sigma_ % 2 == 0 ? 1 : 0);
    int half_size = size / 2;

    image filter = make_image(size, size, 1);

    for (int y = -half_size; y <= half_size; y++) {
        for (int x = -half_size; x <= half_size; x++) {
            set_pixel(filter, half_size + x, half_size + y, 0, get_gaussian_value(x, y, sigma));
        }
    }

    l1_normalize(filter);
    return filter;
}

float get_gaussian_value(int x, int y, float sigma) {
    double expo = exp(-((pow(x, 2) + pow(y, 2)) / (2 * pow(sigma, 2))));
    float scale = TWOPI * pow(sigma, 2);
    return expo / scale;
}

image add_image(image a, image b)
{
    return superimpose_image(a, b, 1);
}

image sub_image(image a, image b)
{
    return superimpose_image(a, b, -1);
}

image superimpose_image(image a, image b, int factor) {
    assert(a.c == b.c && a.h == b.h && a.w == b.w);

    image res_image = make_image(a.w, a.h, a.c);
    for (int c = 0; c < b.c; c++) {
        for (int y = 0; y < b.h; y++) {
            for (int x = 0; x < b.w; x++) {
                set_pixel(res_image, x, y, c, get_pixel(a, x, y, c) + factor * get_pixel(b, x, y, c));
            }
        }
    }
    return res_image;
}

image make_gx_filter()
{
    image filter = make_image(3, 3, 1);

    set_pixel(filter, 0, 0, 0, -1);
    set_pixel(filter, 0, 2, 0, 1);
    set_pixel(filter, 1, 0, 0, -2);
    set_pixel(filter, 1, 2, 0, 2);
    set_pixel(filter, 2, 0, 0, -1);
    set_pixel(filter, 2, 2, 0, 1);

    return filter;
}

image make_gy_filter()
{
    image filter = make_image(3, 3, 1);

    set_pixel(filter, 0, 0, 0, -1);
    set_pixel(filter, 0, 1, 0, -2);
    set_pixel(filter, 0, 2, 0, -1);
    set_pixel(filter, 2, 0, 0, 1);
    set_pixel(filter, 2, 1, 0, 2);
    set_pixel(filter, 2, 2, 0, 1);

    return filter;
}

void feature_normalize(image im)
{
    for (int c = 0; c < im.c; c++) {
        float min = get_pixel(im, 0, 0, c);
        float max = min;

        for (int y = 0; y < im.h; y++) {
            for (int x = 0; x < im.w; x++) {
                float val = get_pixel(im, x, y, c);
                min = min > val ? val : min;
                max = max < val ? val : max;
            }
        }

        float range = max - min;
        for (int y = 0; y < im.h; y++) {
            for (int x = 0; x < im.w; x++) {
                if (range == 0) {
                    set_pixel(im, x, y, c, 0);
                } else {
                    set_pixel(im, x, y, c, (get_pixel(im, x, y, c) - min) / range);
                }
            }
        }
    }
}

image *sobel_image(image im)
{
    image gradient_x = convolve_image(im, make_gx_filter(), 0);
    image gradient_y = convolve_image(im, make_gy_filter(), 0);

    image* sobel_images = calloc(2, sizeof(image));
    sobel_images[0] = make_image(im.w, im.h, 1);
    sobel_images[1] = make_image(im.w, im.h, 1);

    for (int y = 0; y < im.h; y++) {
        for (int x = 0; x < im.w; x++) {
            float gradient_x_val = get_pixel(gradient_x, x, y, 0);
            float gradient_y_val = get_pixel(gradient_y, x, y, 0);
            set_pixel(sobel_images[0], x, y, 0, sqrt(pow(gradient_x_val, 2) + pow(gradient_y_val, 2)));
            set_pixel(sobel_images[1], x, y, 0, atan2(gradient_x_val, gradient_y_val));
        }
    }

    return sobel_images;
}

image colorize_sobel(image im)
{
    // TODO
    return make_image(1,1,1);
}

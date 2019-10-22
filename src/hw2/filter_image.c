#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853

float get_convolved_value(image, image, int, int, int);

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
        scale_image(im, c, 1.0 / sum);
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
                    float val = get_pixel(merged_filtered_image, w, h, 1) + get_pixel(filtered_image, c, h, w);
                    set_pixel(merged_filtered_image, w, h, 1, val);
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
    // TODO
    return make_image(1,1,1);
}

image make_sharpen_filter()
{
    // TODO
    return make_image(1,1,1);
}

image make_emboss_filter()
{
    // TODO
    return make_image(1,1,1);
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer: TODO

// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: TODO

image make_gaussian_filter(float sigma)
{
    // TODO
    return make_image(1,1,1);
}

image add_image(image a, image b)
{
    // TODO
    return make_image(1,1,1);
}

image sub_image(image a, image b)
{
    // TODO
    return make_image(1,1,1);
}

image make_gx_filter()
{
    // TODO
    return make_image(1,1,1);
}

image make_gy_filter()
{
    // TODO
    return make_image(1,1,1);
}

void feature_normalize(image im)
{
    // TODO
}

image *sobel_image(image im)
{
    // TODO
    return calloc(2, sizeof(image));
}

image colorize_sobel(image im)
{
    // TODO
    return make_image(1,1,1);
}

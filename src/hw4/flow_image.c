#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#include "matrix.h"

image* image_gradients(image);

// Draws a line on an image with color corresponding to the direction of line
// image im: image to draw line on
// float x, y: starting point of line
// float dx, dy: vector corresponding to line angle and magnitude
void draw_line(image im, float x, float y, float dx, float dy)
{
    assert(im.c == 3);
    float angle = 6*(atan2(dy, dx) / TWOPI + .5);
    int index = floor(angle);
    float f = angle - index;
    float r, g, b;
    if(index == 0){
        r = 1; g = f; b = 0;
    } else if(index == 1){
        r = 1-f; g = 1; b = 0;
    } else if(index == 2){
        r = 0; g = 1; b = f;
    } else if(index == 3){
        r = 0; g = 1-f; b = 1;
    } else if(index == 4){
        r = f; g = 0; b = 1;
    } else {
        r = 1; g = 0; b = 1-f;
    }
    float i;
    float d = sqrt(dx*dx + dy*dy);
    for(i = 0; i < d; i += 1){
        int xi = x + dx*i/d;
        int yi = y + dy*i/d;
        set_pixel(im, xi, yi, 0, r);
        set_pixel(im, xi, yi, 1, g);
        set_pixel(im, xi, yi, 2, b);
    }
}

// Make an integral image or summed area table from an image
// image im: image to process
// returns: image I such that I[x,y] = sum{i<=x, j<=y}(im[i,j])
image make_integral_image(image im)
{
    image integ = make_image(im.w, im.h, im.c);
    for (int c = 0; c < im.c; c++) {
        for (int h = 0; h < im.h; h++) {
            for (int w = 0; w < im.w; w++) {
                float val = get_pixel(im, w, h, c);
                if (w > 0) {
                    val += get_pixel(integ, w - 1, h, c);
                }
                if (h > 0) {
                    val += get_pixel(integ, w, h - 1, c);
                }
                if (h > 0 && w > 0) {
                    val -= get_pixel(integ, w - 1, h - 1, c);
                }
                set_pixel(integ, w, h, c, val);
            }
        }
    }
    return integ;
}

// Apply a box filter to an image using an integral image for speed
// image im: image to smooth
// int s: window size for box filter
// returns: smoothed image
image box_filter_image(image im, int s)
{
    image integ = make_integral_image(im);
    image S = make_image(im.w, im.h, im.c);

    int half = s / 2;
    for (int c = 0; c < im.c; c++) {
        for (int h = 0; h < im.h; h++) {
            for (int w = 0; w < im.w; w++) {
                float val = 0;

                if (w - half >= 0 && h - half >= 0) {
                    val += get_pixel(integ, w - half, h - half, c);
                }

                if (w + half < integ.w && h - half >= 0) {
                    val -= get_pixel(integ, w + half, h - half, c);
                }

                if (w - half >= 0 && h + half < integ.h) {
                    val -= get_pixel(integ, w - half, h + half, c);
                }

                if (w + half < integ.w && h + half < integ.h) {
                    val += get_pixel(integ, w + half, h + half, c);
                }

                set_pixel(S, w, h, c, val / (s * s));
            }
        }
    }

    free_image(integ);
    return S;
}

// Calculate the time-structure matrix of an image pair.
// image im: the input image.
// image prev: the previous image in sequence.
// int s: window size for smoothing.
// returns: structure matrix. 1st channel is Ix^2, 2nd channel is Iy^2,
//          3rd channel is IxIy, 4th channel is IxIt, 5th channel is IyIt.
image time_structure_matrix(image im, image prev, int s)
{
    int converted = 0;
    if (im.c == 3) {
        converted = 1;
        im = rgb_to_grayscale(im);
        prev = rgb_to_grayscale(prev);
    }

    image* gradients_im = image_gradients(im);
    image S = make_image(im.w, im.h, 5);

    for (int h = 0; h < im.h; h++) {
        for (int w = 0; w < im.w; w++) {
            float Ix = get_pixel(gradients_im[0], w, h, 0);
            float Iy = get_pixel(gradients_im[1], w, h, 0);
            float It = get_pixel(prev, w, h, 0) - get_pixel(im, w, h, 0);

            set_pixel(S, w, h, 0, Ix * Ix);
            set_pixel(S, w, h, 1, Iy * Iy);
            set_pixel(S, w, h, 2, Ix * Iy);
            set_pixel(S, w, h, 3, Ix * It);
            set_pixel(S, w, h, 4, Iy * It);
        }
    }

    image smooth_image = box_filter_image(S, s);
    free_image(S);
    free_image(gradients_im[0]);
    free_image(gradients_im[1]);

    if (converted) {
        free_image(im); free_image(prev);
    }
    return smooth_image;
}

image* image_gradients(image im) {
    image gradient_x = make_gx_filter();
    image gradient_y = make_gy_filter();

    image* gradients = calloc(2, sizeof(image));

    gradients[0] = convolve_image(im, gradient_x, 0);
    gradients[1] = convolve_image(im, gradient_y, 0);

    free_image(gradient_x);
    free_image(gradient_y);

    return gradients;
}

// Calculate the velocity given a structure image
// image S: time-structure image
// int stride: only calculate subset of pixels for speed
image velocity_image(image S, int stride)
{
    image v = make_image(S.w/stride, S.h/stride, 3);
    int i, j;
    matrix M = make_matrix(2, 2);
    matrix b = make_matrix(2, 1);
    for(j = (stride-1)/2; j < S.h; j += stride){
        for(i = (stride-1)/2; i < S.w; i += stride){
            float Ixx = S.data[i + S.w*j + 0*S.w*S.h];
            float Iyy = S.data[i + S.w*j + 1*S.w*S.h];
            float Ixy = S.data[i + S.w*j + 2*S.w*S.h];
            float Ixt = S.data[i + S.w*j + 3*S.w*S.h];
            float Iyt = S.data[i + S.w*j + 4*S.w*S.h];

            M.data[0][0] = Ixx; M.data[0][1] = M.data[1][0] = Ixy; M.data[1][1] = Iyy;
            matrix transpose = transpose_matrix(M);
            matrix square = matrix_mult_matrix(transpose, M);
            matrix inverse = matrix_invert(square);

            if (inverse.cols != 0 && inverse.rows != 0) {
                b.data[0][0] = Ixt; b.data[1][0] = Iyt;

                matrix first = matrix_mult_matrix(inverse, transpose);
                matrix solved = matrix_mult_matrix(first, b);

                set_pixel(v, i/stride, j/stride, 0, solved.data[0][0]);
                set_pixel(v, i/stride, j/stride, 1, solved.data[1][0]);

                free_matrix(first);
                free_matrix(solved);
            }

            free_matrix(transpose);
            free_matrix(square);
            free_matrix(inverse);
        }
    }

    free_matrix(M);
    free_matrix(b);

    return v;
}

// Draw lines on an image given the velocity
// image im: image to draw on
// image v: velocity of each pixel
// float scale: scalar to multiply velocity by for drawing
void draw_flow(image im, image v, float scale)
{
    int stride = im.w / v.w;
    int i,j;
    for (j = (stride-1)/2; j < im.h; j += stride) {
        for (i = (stride-1)/2; i < im.w; i += stride) {
            float dx = scale*get_pixel(v, i/stride, j/stride, 0);
            float dy = scale*get_pixel(v, i/stride, j/stride, 1);
            if(fabs(dx) > im.w) dx = 0;
            if(fabs(dy) > im.h) dy = 0;
            draw_line(im, i, j, dx, dy);
        }
    }
}


// Constrain the absolute value of each image pixel
// image im: image to constrain
// float v: each pixel will be in range [-v, v]
void constrain_image(image im, float v)
{
    int i;
    for(i = 0; i < im.w*im.h*im.c; ++i){
        if (im.data[i] < -v) im.data[i] = -v;
        if (im.data[i] >  v) im.data[i] =  v;
    }
}

// Calculate the optical flow between two images
// image im: current image
// image prev: previous image
// int smooth: amount to smooth structure matrix by
// int stride: downsampling for velocity matrix
// returns: velocity matrix
image optical_flow_images(image im, image prev, int smooth, int stride)
{
    image S = time_structure_matrix(im, prev, smooth);   
    image v = velocity_image(S, stride);
    constrain_image(v, 6);
    image vs = smooth_image(v, 2);
    free_image(v);
    free_image(S);
    return vs;
}

// Run optical flow demo on webcam
// int smooth: amount to smooth structure matrix by
// int stride: downsampling for velocity matrix
// int div: downsampling factor for images from webcam
void optical_flow_webcam(int smooth, int stride, int div)
{
#ifdef OPENCV
    void * cap;
    cap = open_video_stream(0, 0, 1280, 720, 30);
    image prev = get_image_from_stream(cap);
    image prev_c = nn_resize(prev, prev.w/div, prev.h/div);
    image im = get_image_from_stream(cap);
    image im_c = nn_resize(im, im.w/div, im.h/div);
    while(im.data){
        image copy = copy_image(im);
        image v = optical_flow_images(im_c, prev_c, smooth, stride);
        draw_flow(copy, v, smooth*div);
        int key = show_image(copy, "flow", 5);
        free_image(v);
        free_image(copy);
        free_image(prev);
        free_image(prev_c);
        prev = im;
        prev_c = im_c;
        if(key != -1) {
            key = key % 256;
            printf("%d\n", key);
            if (key == 27) break;
        }
        im = get_image_from_stream(cap);
        im_c = nn_resize(im, im.w/div, im.h/div);
    }
#else
    fprintf(stderr, "Must compile with OpenCV\n");
#endif
}

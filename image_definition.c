#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "image_definition.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

DefImage defimg_load(const char *path)
{
    DefImage img = {0};
    int channels;
    img.data = stbi_load(path, &img.width, &img.height, &channels, 1 /* force grayscale */);
    if (!img.data)
        fprintf(stderr, "error: cannot load '%s': %s\n", path, stbi_failure_reason());
    return img;
}

void defimg_free(DefImage *img)
{
    stbi_image_free(img->data);
    img->data = NULL;
    img->width = img->height = 0;
}

/* Clamp-bordered pixel accessor */
static inline double px(const DefImage *img, int x, int y)
{
    if (x < 0) x = 0;
    else if (x >= img->width)  x = img->width  - 1;
    if (y < 0) y = 0;
    else if (y >= img->height) y = img->height - 1;
    return (double)img->data[y * img->width + x];
}

/*
 * Laplacian variance
 *
 * Applies the 8-neighbour Laplacian kernel and returns the variance of the
 * response map.  Sharp images have strong, spread-out edge responses giving
 * high variance; blurry images have near-zero responses throughout.
 *
 * Kernel:
 *   -1 -1 -1
 *   -1  8 -1
 *   -1 -1 -1
 */
double defimg_laplacian_variance(const DefImage *img)
{
    if (!img->data || img->width < 3 || img->height < 3)
        return 0.0;

    double sum = 0.0, sum2 = 0.0;
    long n = 0;

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            double v =
                  8.0 * px(img, x,   y)
                - px(img, x-1, y-1) - px(img, x, y-1) - px(img, x+1, y-1)
                - px(img, x-1, y)                      - px(img, x+1, y)
                - px(img, x-1, y+1) - px(img, x, y+1) - px(img, x+1, y+1);
            sum  += v;
            sum2 += v * v;
            n++;
        }
    }

    double mean = sum / n;
    return sum2 / n - mean * mean;   /* Var(X) = E[X²] - E[X]² */
}

/*
 * Tenengrad (Sobel gradient energy)
 *
 * Computes the mean squared gradient magnitude using Sobel operators.  This is
 * the classic autofocus criterion: well-focused images have large, dense
 * gradients at edges.
 *
 * Sobel Gx:       Sobel Gy:
 *  -1  0  1        -1 -2 -1
 *  -2  0  2         0  0  0
 *  -1  0  1         1  2  1
 */
double defimg_tenengrad(const DefImage *img)
{
    if (!img->data || img->width < 3 || img->height < 3)
        return 0.0;

    double sum = 0.0;
    long n = 0;

    for (int y = 1; y < img->height - 1; y++) {
        for (int x = 1; x < img->width - 1; x++) {
            double gx =
                (px(img, x+1, y-1) + 2.0*px(img, x+1, y) + px(img, x+1, y+1))
              - (px(img, x-1, y-1) + 2.0*px(img, x-1, y) + px(img, x-1, y+1));
            double gy =
                (px(img, x-1, y+1) + 2.0*px(img, x, y+1) + px(img, x+1, y+1))
              - (px(img, x-1, y-1) + 2.0*px(img, x, y-1) + px(img, x+1, y-1));
            sum += gx * gx + gy * gy;
            n++;
        }
    }

    return sum / n;
}

/*
 * Brenner's focus measure
 *
 * Sums the squared difference between each pixel and the pixel two steps to
 * its right.  Simple, fast, and effective for images with horizontal edges.
 */
double defimg_brenner(const DefImage *img)
{
    if (!img->data || img->width < 3)
        return 0.0;

    double sum = 0.0;
    long n = 0;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width - 2; x++) {
            double d = px(img, x + 2, y) - px(img, x, y);
            sum += d * d;
            n++;
        }
    }

    return sum / n;
}

/*
 * Normalized variance
 *
 * Variance of pixel intensities divided by the mean intensity.  Normalising by
 * the mean makes this less sensitive to overall brightness changes than raw
 * variance, while still reflecting local contrast and detail richness.
 */
double defimg_normalized_variance(const DefImage *img)
{
    if (!img->data)
        return 0.0;

    long n = (long)img->width * img->height;
    double sum = 0.0, sum2 = 0.0;

    for (long i = 0; i < n; i++) {
        double v = (double)img->data[i];
        sum  += v;
        sum2 += v * v;
    }

    double mean = sum / n;
    if (mean < 1e-9)
        return 0.0;

    double variance = sum2 / n - mean * mean;
    return variance / mean;
}

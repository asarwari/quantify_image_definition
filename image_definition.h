#ifndef IMAGE_DEFINITION_H
#define IMAGE_DEFINITION_H

#include <stdint.h>

typedef struct {
    uint8_t *data;
    int width, height;
} DefImage;

/* Load image from path as grayscale. Returns zeroed struct on failure. */
DefImage defimg_load(const char *path);
void defimg_free(DefImage *img);

/*
 * Sharpness / definition metrics — higher values mean a sharper, better-defined
 * image in all cases.
 *
 * laplacian_variance : variance of the Laplacian response (edge energy spread)
 * tenengrad          : mean squared Sobel gradient magnitude
 * brenner            : mean squared 2-pixel horizontal difference
 * normalized_variance: pixel intensity variance divided by mean intensity
 */
double defimg_laplacian_variance(const DefImage *img);
double defimg_tenengrad(const DefImage *img);
double defimg_brenner(const DefImage *img);
double defimg_normalized_variance(const DefImage *img);

#endif /* IMAGE_DEFINITION_H */

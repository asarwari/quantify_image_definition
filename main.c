#include "image_definition.h"
#include <stdio.h>
#include <string.h>

static void print_separator(int ncols)
{
    for (int i = 0; i < ncols; i++)
        putchar('-');
    putchar('\n');
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr,
            "Usage: %s <image> [image ...]\n\n"
            "Supported formats: JPEG, PNG, BMP, TGA, GIF, PSD, HDR, PIC, PNM\n"
            "All metrics are higher-is-sharper.\n",
            argv[0]);
        return 1;
    }

    const int col_file   = 38;
    const int col_metric = 16;
    const int total = col_file + col_metric * 4 + 1;

    printf("%-*s %*s %*s %*s %*s\n",
        col_file,  "File",
        col_metric, "Lap.Variance",
        col_metric, "Tenengrad",
        col_metric, "Brenner",
        col_metric, "Norm.Variance");
    print_separator(total);

    int failures = 0;
    for (int i = 1; i < argc; i++) {
        DefImage img = defimg_load(argv[i]);
        if (!img.data) {
            failures++;
            continue;
        }

        double lv = defimg_laplacian_variance(&img);
        double tg = defimg_tenengrad(&img);
        double br = defimg_brenner(&img);
        double nv = defimg_normalized_variance(&img);

        printf("%-*s %*.4f %*.4f %*.4f %*.4f\n",
            col_file,  argv[i],
            col_metric, lv,
            col_metric, tg,
            col_metric, br,
            col_metric, nv);

        defimg_free(&img);
    }

    return failures > 0 ? 1 : 0;
}

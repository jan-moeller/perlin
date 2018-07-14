#include <iostream>
#include <cassert>

#include "point.h"
#include "vector.h"
#include "perlin_noise_generator.h"
#include "fractal_noise_generator.h"

#include "png.h"

int main()
{
    point2d_f p{1.337f, 42.f};

    vector<float, 2> v1{42.f, 13.f};
    vector<float, 2> v2{1.f, 2.f};

    std::cout << "v1: " << v1 << std::endl;
    std::cout << "v2: " << v2 << std::endl;
    std::cout << std::endl;
    std::cout << "v1 + v2: " << (v1 + v2) << std::endl;
    std::cout << "v1 - v2: " << v1 - v2 << std::endl;
    std::cout << "-v1: " << -v1 << std::endl;
    std::cout << "v1 * 2: " << v1 * 2 << std::endl;
    std::cout << "v1 dot v2: " << dot(v1, v2) << std::endl;
    std::cout << "|v1|: " << magnitude(v1) << std::endl;
    std::cout << "v1/|v1|: " << normalized(v1) << std::endl;

    std::cout << "p.convert_to<int>(): " << p.convert_to<int>() << std::endl;

    n_choose_k(7, 3);

    fractal_noise_generator<perlin_noise_generator<2>, 5> gen{42};
    std::cout << gen.at(point2d_f{0.2f, 0.2f}) << std::endl;
    std::cout << gen.at(point2d_f{1.2f, 0.2f}) << std::endl;
    float showcells = 4;

    auto filename = "out.png";
    char* title = "noise";
    int width = 512;
    int height = 512;

    int code = 0;
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

    // Open file for writing (binary mode)
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file %s for writing\n", filename);
        code = 1;
        goto finalise;
    }

    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fprintf(stderr, "Could not allocate write struct\n");
        code = 1;
        goto finalise;
    }

    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fprintf(stderr, "Could not allocate info struct\n");
        code = 1;
        goto finalise;
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error during png creation\n");
        code = 1;
        goto finalise;
    }

    png_init_io(png_ptr, fp);

    // Write header (8 bit colour depth)
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Set title
    if (title != NULL) {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = title;
        png_set_text(png_ptr, info_ptr, &title_text, 1);
    }

    png_write_info(png_ptr, info_ptr);

    // Allocate memory for one row (1 bytes per pixel - Grayscale)
    row = (png_bytep) malloc(width * sizeof(png_byte));

    // Write image data
    int x, y;
    for (y=0 ; y<height ; y++) {
        png_byte min = 255;
        png_byte max = 0;
        for (x=0 ; x<width ; x++) {
            float xPos = (float)x/(width / showcells);
            float yPos = (float)y /(height / showcells);
            auto val = gen.at(point2d_f{xPos, yPos});
            row[x] = png_byte((val + 1) * 128);

            // Mark grid corners
            if (((std::ceil(xPos) - xPos) < 0.000001f) && ((std::ceil(yPos) - yPos) < 0.000001f))
                row[x] = 255;

            if (row[x] < min)
                min = row[x];
            if (row[x] > max)
                max = row[x];
        }
        png_write_row(png_ptr, row);
    }

    // End write
    png_write_end(png_ptr, NULL);

    finalise:
    if (fp != NULL) fclose(fp);
    if (info_ptr != NULL) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL) png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    if (row != NULL) free(row);

    return code;

    return 0;
}
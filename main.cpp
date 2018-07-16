#include <iostream>
#include <string>

#include <png.h>

#include "point.h"
#include "vector.h"
#include "perlin_noise_generator.h"
#include "fractal_noise_generator.h"
#include "seamless_noise_generator_2d.h"

template<class Gen>
int generate_png(Gen& gen, std::string const& filename, int width, int height, int showcells, std::function<point<typename Gen::result_t, Gen::dimensions>(typename Gen::result_t, typename Gen::result_t)> mapPoint)
{
    char const* title = filename.c_str();
    FILE* fp = nullptr;
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;

    auto finalize = [&]()
    {
        if (fp != nullptr) fclose(fp);
        if (info_ptr != nullptr) png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
        if (png_ptr != nullptr) png_destroy_write_struct(&png_ptr, (png_infopp) nullptr);
    };

    // Open file for writing (binary mode)
    fp = fopen(filename.c_str(), "wb");
    if (fp == nullptr)
    {
        fprintf(stderr, "Could not open file %s for writing\n", filename.c_str());
        finalize();
        return 1;
    }

    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr)
    {
        fprintf(stderr, "Could not allocate write struct\n");
        finalize();
        return 1;
    }

    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == nullptr)
    {
        fprintf(stderr, "Could not allocate info struct\n");
        finalize();
        return 1;
    }

    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "Error during png creation\n");
        finalize();
        return 1;
    }

    png_init_io(png_ptr, fp);

    // Write header (8 bit grayscale depth)
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Set title
    if (title != nullptr)
    {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = const_cast<char*>("Title");
        title_text.text = const_cast<char*>(title);
        png_set_text(png_ptr, info_ptr, &title_text, 1);
    }

    png_write_info(png_ptr, info_ptr);

    // Allocate memory for one row (1 bytes per pixel - Grayscale)
    std::vector<png_byte> row;
    row.resize(width);

    // Write image data
    int x, y;
    for (y = 0; y < height; y++)
    {
//        std::cout << std::endl;
        for (x = 0; x < width; x++)
        {
            float xPos = (float) x / (width / (float) showcells);
            float yPos = (float) y / (height / (float) showcells);
            auto val = std::clamp(gen.at(mapPoint(xPos, yPos)), -1.f, 1.f);
            row[x] = png_byte((val + 1) / 2.f * 255);

//            std::cout << val << ", ";

            // Mark grid corners
//            if (((std::ceil(xPos) - xPos) < 0.000001f) && ((std::ceil(yPos) - yPos) < 0.000001f))
//                row[x] = 255;
        }
        png_write_row(png_ptr, row.data());
    }

    // End write
    png_write_end(png_ptr, nullptr);
    finalize();

    return 0;
}

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
    std::cout << "p.floor<int>(): " << p.floor<int>() << std::endl;
    std::cout << "p.ceil<int>(): " << p.ceil<int>() << std::endl;

    n_choose_k(7, 3);

    bool const enable2d = true;
    bool const enable3d = false;
    bool const enable4d = false;
    bool const enableSeamless = true;

    constexpr int const showcells = 4;
    int const width = 512;
    int const height = 512;

    if (enable2d)
    {
        // 2d
        using Gen = perlin_noise_generator<2>;
        Gen gen{42};
        generate_png(gen, "2d.png", width, height, showcells, [](float x, float y) {return point2d_f{x, y};});
    }

    if (enable3d)
    {
        // 3d
        using Gen = perlin_noise_generator<3>;
        Gen gen{42};
        for (int i = 0; i < 10; ++i)
        {
            generate_png(gen, "3d_" + std::to_string(i) + ".png", width, height, showcells, [i](float x, float y) {return point3d_f{x, y, i/8.f};});
        }
    }

    if (enable4d)
    {
        // 4d
        using Gen = perlin_noise_generator<4>;
        Gen gen{42};
        for (int i = 0; i < 10; ++i)
        {
            generate_png(gen, "4d_" + std::to_string(i) + ".png", width, height, showcells, [i](float x, float y) {return point4d_f{x, y, i/8.f, i/8.f};});
        }
    }

    if (enableSeamless)
    {
        // seamless
        using Gen = seamless_noise_generator_2d<fractal_noise_generator<perlin_noise_generator<4>>, showcells, showcells>;
        Gen gen{42};
        generate_png(gen, "seamless.png", width, height, showcells, [](float x, float y) {return point2d_f{x, y};});
    }

    return 0;
}
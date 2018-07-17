#include <iostream>

#include <png.h>
#include <map>

#include "point.h"
#include "vector.h"
#include "perlin_noise_generator.h"
#include "fractal_noise_generator.h"
#include "seamless_noise_generator_2d.h"

using namespace noise;

template<class Gen>
int generate_png(Gen& gen, std::string const& filename, int width, int height, int cellsX, int cellsY, std::function<point<typename Gen::result_t, Gen::dimensions>(typename Gen::result_t, typename Gen::result_t)> mapPoint)
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
            float xPos = (float) x / (width / (float) cellsX);
            float yPos = (float) y / (height / (float) cellsY);
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

template<class Gen>
int generate_world_png(Gen& gen, std::string const& filename, int width, int height, int cellsX, int cellsY, std::function<point<typename Gen::result_t, Gen::dimensions>(typename Gen::result_t, typename Gen::result_t)> mapPoint)
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

    // Write header (8 bit rgb depth)
    png_set_IHDR(png_ptr, info_ptr, width, height,
                 8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
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

    // Allocate memory for one row (3 bytes per pixel - RGB)
    std::vector<png_byte> row;
    row.resize(width * 3);

    // Color scale
    using Color = std::array<png_byte, 3>;
    std::map<float, Color> colors{
            { 0.5f,  {255, 255, 255}}, // snow
            { 0.35f, {150, 150, 160}}, // mountains
            { 0.25f, { 60, 130,  30}}, // forest
            { 0.15f, {120, 190,  90}}, // grass
            { 0.1f,  {229, 221,   0}}, // shore
            {-1.f,   {  0,   0, 255}}, // water
    };

    // Write image data
    int x, y;
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            float xPos = (float) x / (width / (float) cellsX);
            float yPos = (float) y / (height / (float) cellsY);
            auto val = std::clamp(gen.at(mapPoint(xPos, yPos)), -1.f, 1.f);
            auto colorIter = colors.lower_bound(val);
            if (colorIter == colors.begin())
            {
                std::cerr << "No color for " << val << std::endl;
                continue;
            }
            --colorIter;
            auto color = colorIter->second;
            row[(x*3)+0] = color[0];
            row[(x*3)+1] = color[1];
            row[(x*3)+2] = color[2];
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
    bool const enable2d = false;
    bool const enable3d = false;
    bool const enable4d = false;
    bool const enableSeamless = true;
    bool const enableWorld = true;

    constexpr int const cellsX = 6;
    constexpr int const cellsY = 4;
    int const width = 512 + 256;
    int const height = 512;
    std::uint_fast32_t seed = 10;
    constexpr int smoothness = 2;
    constexpr int octaves = 50;

    if (enable2d)
    {
        // 2d
        using Gen = perlin_noise_generator<2, smoothness>;
        Gen gen{seed};
        generate_png(gen, "2d.png", width, height, cellsX, cellsY, [](float x, float y) {return point2d_f{x, y};});
    }

    if (enable3d)
    {
        // 3d
        using Gen = perlin_noise_generator<3, smoothness>;
        Gen gen{seed};
        for (int i = 0; i < 10; ++i)
        {
            generate_png(gen, "3d_" + std::to_string(i) + ".png", width, height, cellsX, cellsY, [i](float x, float y) {return point3d_f{x, y, i/8.f};});
        }
    }

    if (enable4d)
    {
        // 4d
        using Gen = perlin_noise_generator<4, smoothness>;
        Gen gen{seed};
        for (int i = 0; i < 10; ++i)
        {
            generate_png(gen, "4d_" + std::to_string(i) + ".png", width, height, cellsX, cellsY, [i](float x, float y) {return point4d_f{x, y, i/8.f, i/8.f};});
        }
    }

    if (enableSeamless)
    {
        // seamless
        using Gen = seamless_noise_generator_2d<fractal_noise_generator<perlin_noise_generator<4, smoothness>, octaves>, cellsX, cellsY>;
        Gen gen{seed};
        generate_png(gen, std::to_string(seed) + "_seamless.png", width, height, cellsX, cellsY, [](float x, float y) {return point2d_f{x, y};});
    }

    if (enableWorld)
    {
        // seamless world generation
        using Gen = seamless_noise_generator_2d<fractal_noise_generator<perlin_noise_generator<4, smoothness>, octaves>, cellsX, cellsY>;
        Gen gen{seed};
        generate_world_png(gen, std::to_string(seed) + "_world.png", width, height, cellsX, cellsY, [](float x, float y) {return point2d_f{x, y};});
    }

    return 0;
}
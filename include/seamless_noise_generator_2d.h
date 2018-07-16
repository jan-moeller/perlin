/**********************************************************
 * @file   seamless_noise_generator_2d.h
 * @author jan
 * @date   7/14/18
 * ********************************************************
 * @brief
 * @details
 **********************************************************/
#ifndef PERLINNOISE_SEAMLESS_NOISE_GENERATOR_2D_H
#define PERLINNOISE_SEAMLESS_NOISE_GENERATOR_2D_H

#include <random>

#include "point.h"

template<class Gen,
        typename Gen::grid_coord_t Width,
        typename Gen::grid_coord_t Height>
class seamless_noise_generator_2d
{
public:
    static_assert(Gen::dimensions == 4, "Must use a 4D generator");
    static_assert(Width > 0, "Width must be positive");
    static_assert(Height > 0, "Height must be positive");

    using result_t = typename Gen::result_t;
    using grid_coord_t = typename Gen::grid_coord_t;

    static constexpr const int dimensions = 2;

    explicit seamless_noise_generator_2d(std::uint_fast32_t seed = std::random_device()()) noexcept
            : m_noiseGen(seed)
    {
    }

    constexpr result_t at(point<result_t, dimensions> const& p) const noexcept
    {
        constexpr result_t const pi = 3.1415926535897932384626433832795028841971;
        constexpr result_t const two_pi = 2 * pi;

        auto s = p[0] / static_cast<result_t>(Width);
        auto t = p[1] / static_cast<result_t>(Height);

        auto multiplier_x = static_cast<result_t>(Width) / two_pi;
        auto multiplier_y = static_cast<result_t>(Height) / two_pi;
        auto nx = std::cos(s * two_pi) * multiplier_x;
        auto ny = std::cos(t * two_pi) * multiplier_y;
        auto nz = std::sin(s * two_pi) * multiplier_x;
        auto nw = std::sin(t * two_pi) * multiplier_y;

        return m_noiseGen.at(point<result_t, 4>{nx, ny, nz, nw});
    }

private:
    Gen m_noiseGen;
};

#endif //PERLINNOISE_SEAMLESS_NOISE_GENERATOR_2D_H

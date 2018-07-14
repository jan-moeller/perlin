/**********************************************************
 * @file   fractal_noise_generator.h
 * @author jan
 * @date   7/14/18
 * ********************************************************
 * @brief
 * @details
 **********************************************************/
#ifndef PERLINNOISE_FRACTAL_NOISE_GENERATOR_H
#define PERLINNOISE_FRACTAL_NOISE_GENERATOR_H

#include <random>
#include <algorithm>

#include "point.h"

template<class Gen, int Octaves = 3>
class fractal_noise_generator
{
public:
    static_assert(Octaves > 0, "Must have at least one octave");

    static constexpr const int dimensions = Gen::dimensions;

    using result_t = typename Gen::result_t;

    explicit fractal_noise_generator(std::uint_fast32_t seed = std::random_device()()) noexcept
            : m_noiseGen(seed)
    {
    }

    constexpr result_t at(point<result_t, dimensions> const& p) const noexcept
    {
        result_t result = 0;
        for (int i = 1; i <= Octaves; ++i)
            result += m_noiseGen.at(pointAtOctave(p, i)) / static_cast<result_t>(i);

        return std::clamp(result, static_cast<result_t>(-1), static_cast<result_t>(1));
    }

private:
    constexpr point<result_t, dimensions> pointAtOctave(point<result_t, dimensions> p, int octave) const noexcept
    {
        for (auto& e : p)
            e *= octave;
        return p;
    }

    Gen m_noiseGen;
};

#endif //PERLINNOISE_FRACTAL_NOISE_GENERATOR_H

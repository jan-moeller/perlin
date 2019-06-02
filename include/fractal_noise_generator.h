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
#include <ratio>

#include "point.h"
#include "math.h"

namespace noise
{
    template<class T, class Ratio = std::ratio<6, 10>>
    class linear_decay
    {
    private:
        constexpr static T s_slope = static_cast<T>(-Ratio::num) / static_cast<T>(Ratio::den);
    public:
        constexpr T operator()(int i) const noexcept
        {
            return s_slope * static_cast<T>(i) + 1;
        }
    };

    template<class T, class Ratio = std::ratio<6, 10>>
    using linear_growth = linear_decay<T, std::ratio_multiply<Ratio, std::ratio<-1>>>;

    template<class T>
    class hyperbolic_decay
    {
    public:
        constexpr T operator()(int i) const noexcept
        {
            return 1 / static_cast<T>(i + 1);
        }
    };

    template<class T, class Ratio = std::ratio<6, 10>>
    class exponential_decay
    {
    private:
        static_assert(std::ratio_greater_v<Ratio, std::ratio<0, 1>>, "B must be greater than zero");

        constexpr static T s_base = static_cast<T>(Ratio::num) / static_cast<T>(Ratio::den);
    public:
        constexpr T operator()(int i) const noexcept
        {
            return powi(s_base, i);
        }
    };

    template<class T, class Ratio = std::ratio<2, 1>>
    using exponential_growth = exponential_decay<T, Ratio>;

    template<class T, class Ratio = std::ratio<3, 1>>
    class polynomial_decay
    {
    private:
        constexpr static T s_exp = static_cast<T>(-Ratio::num) / static_cast<T>(Ratio::den);
    public:
        constexpr T operator()(int i) const noexcept
        {
            return std::pow(i + 1, s_exp);
        }
    };

    template<class T,  class Ratio = std::ratio<3, 1>>
    using polynomial_growth = polynomial_decay<T, std::ratio_multiply<Ratio, std::ratio<-1>>>;

    template<class T, class Ratio = std::ratio<3, 1>>
    class gaussian_decay
    {
    private:
        constexpr static T s_stddev = static_powi<2>(static_cast<T>(-Ratio::num) / static_cast<T>(Ratio::den));
    public:
        constexpr T operator()(int i) const noexcept
        {
            return std::pow(constants<T>::e, -static_powi<2>(i) / (2 * s_stddev));
        }
    };

    /**
     * Generates fractal noise in arbitrary dimensions.
     *
     * @details Fractal noise is generated by summing multiple coherent noise functions at increasing frequencies but
     *          with decreasing weight.
     *
     *          Octave frequency increases according to the given frequency function with every step, weight decreases
     *          according to the given weighting function. To keep the resulting value within range [-1,1], the
     *          summation result is smoothed using a smoothstep function of configurable order.
     *
     * @tparam Gen          A coherent noise generator such as perlin_noise_generator
     * @tparam Octaves      Number of noise functions to add
     * @tparam WeightFun    Weighting function
     * @tparam FrequencyFun Frequency function
     * @tparam Contrast     Order of the smoothstep function to use
     */
    template<class Gen,
            int Octaves = 3,
            class WeightFun = hyperbolic_decay<typename Gen::result_t>,
            class FrequencyFun = linear_growth<typename Gen::result_t>,
            int Contrast = 1>
    class fractal_noise_generator
    {
    public:
        static_assert(Octaves > 0, "Must have at least one octave");

        using result_t = typename Gen::result_t;
        using grid_coord_t = typename Gen::grid_coord_t;

        static constexpr const int dimensions = Gen::dimensions;

        /**
         * @param seed Random seed for noise generation
         */
        explicit fractal_noise_generator(std::uint_fast32_t seed = std::random_device()()) noexcept
                : m_noiseGen(seed)
        {
            std::generate(m_weights.begin(), m_weights.end(),
                          [weightFun = WeightFun(), i = 0]() mutable { return weightFun(i++); });
            std::generate(m_frequencies.begin(), m_frequencies.end(),
                          [freqFun = FrequencyFun(), i = 0]() mutable { return freqFun(i++); });
        }

        /**
         * Evaluate the noise function at a given point.
         *
         * @param p Point of evaluation
         * @return  Noise function value at the specified point
         */
        result_t at(point<result_t, dimensions> const& p) const noexcept
        {
            result_t result = 0;
            for (int i = 0; i < Octaves; ++i)
                result += m_noiseGen.at(pointAtOctave(p, i)) * m_weights[i];

            return smoothstep<Contrast>((result + 1) / 2.f) * 2.f - 1;
        }

    private:
        constexpr point<result_t, dimensions> pointAtOctave(point<result_t, dimensions> p, int octave) const noexcept
        {
            for (auto& e : p)
                e *= m_frequencies[octave];
            return p;
        }

        Gen m_noiseGen;
        std::array<result_t, Octaves> m_weights;
        std::array<result_t, Octaves> m_frequencies;
    };

}

#endif //PERLINNOISE_FRACTAL_NOISE_GENERATOR_H

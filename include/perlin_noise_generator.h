//
// Created by jan on 07.07.18.
//

#ifndef PERLINNOISE_PERLIN_NOISE_GENERATOR_H
#define PERLINNOISE_PERLIN_NOISE_GENERATOR_H

#include <random>
#include <bitset>
#include <numeric>
#include <algorithm>
#include <cmath>

#include "point.h"
#include "vector.h"

template<typename T>
constexpr T ipow(T base, int exp) noexcept
{
    static_assert(std::is_integral_v<T>, "Can only compute integer powers");

    T result = 1;
    while (exp != 0)
    {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}

template<typename T>
constexpr T n_choose_k(T n, T k) noexcept
{
    static_assert(std::is_integral_v<T>, "must be an integral type");

    if (k > n)
        return 0;
    if (k == 0)
        return 1;
    if (2 * k > n)
        k = n - k;

    T val = 1;
    for (T i = 1; i <= k; ++i)
        val = std::lround(val * (n - k + i) / static_cast<float>(i));
    return val;
}
static_assert(n_choose_k(0, 0) == 1);
static_assert(n_choose_k(1, 0) == 1);
static_assert(n_choose_k(0, 1) == 0);
static_assert(n_choose_k(1, 1) == 1);
static_assert(n_choose_k(0, 0) == 1);
static_assert(n_choose_k(2, 1) == 2);
static_assert(n_choose_k(5, 2) == 10);
static_assert(n_choose_k(2, 0) == 1);
static_assert(n_choose_k(3, 1) == 3);
static_assert(n_choose_k(5, 1) == 5);
static_assert(n_choose_k(4, 2) == 6);
static_assert(n_choose_k(5, 0) == 1);
static_assert(n_choose_k(3, 0) == 1);
static_assert(n_choose_k(7, 3) == 35);
static_assert(n_choose_k(4, 1) == 4);
static_assert(n_choose_k(7, 2) == 21);
static_assert(n_choose_k(6, 3) == 20);

template<int N, typename T>
constexpr T smoothstep(T x) noexcept
{
    static_assert(N >= 0, "N must be a nonnegative integer");

    if (x <= 0)
        return 0;
    if (x < 1)
    {
        T sum = 0;
        for (int n = 0; n <= N; ++n)
            sum += n_choose_k(N + n, n) * n_choose_k(2 * N + 1, N - n) * std::pow(-x, n);
        return std::pow(x, N + 1) * sum;
    }
    else
        return 1;
}
static_assert(smoothstep<0>(0) - 0 < 1e-5);
static_assert(smoothstep<0>(0.3) - 0.3 < 1e-5);
static_assert(smoothstep<0>(0.5) - 0.5 < 1e-5);
static_assert(smoothstep<0>(1) - 1 < 1e-5);
static_assert(smoothstep<1>(0) - 0 < 1e-5);
static_assert(smoothstep<1>(0.3) - 0.216 < 1e-5);
static_assert(smoothstep<1>(0.5) - 0.5 < 1e-5);
static_assert(smoothstep<1>(1) - 1 < 1e-5);
static_assert(smoothstep<2>(0) - 0 < 1e-5);
static_assert(smoothstep<2>(0.3) - 0.16308 < 1e-5);
static_assert(smoothstep<2>(0.5) - 0.5 < 1e-5);
static_assert(smoothstep<2>(1) - 1 < 1e-5);

template<typename T>
constexpr T mod(T k, T n) noexcept
{
    return ((k %= n) < 0) ? k + n : k;
}
static_assert(mod(0, 3) == 0);
static_assert(mod(1, 3) == 1);
static_assert(mod(2, 3) == 2);
static_assert(mod(3, 3) == 0);
static_assert(mod(5, 3) == 2);
static_assert(mod(6, 3) == 0);
static_assert(mod(7, 3) == 1);
static_assert(mod(-1, 3) == 2);
static_assert(mod(-2, 3) == 1);
static_assert(mod(-3, 3) == 0);
static_assert(mod(-5, 3) == 1);
static_assert(mod(-6, 3) == 0);
static_assert(mod(-7, 3) == 2);


/**
 * Generates perlin noise in arbitrary dimensions.
 *
 * @details Perlin noise is a type of coherent noise. As such it has three properties:
 *          1. Evaluating it at the same point will always give the same result
 *          2. A small change in input will yield a small change in output
 *          3. A large change in input results in a random change in output
 *
 *          Noise is generated in the [-1,1] domain.
 *
 * @tparam Dim          Dimensionality of the noise function
 * @tparam Smoothness   Order of smoothstep function to use for interpolation
 * @tparam Result       Arithmetic result type
 * @tparam NumGradients Amount of random gradients to use. A larger number results in more randomness, but longer
 *                      computation times.
 * @tparam GridCoord    Integral grid coordinate type
 */
template<int Dim,
        int Smoothness = 2,
        typename Result = float,
        int NumGradients = 256,
        typename GridCoord = int>
class perlin_noise_generator
{
public:
    static_assert(Dim > 0, "Must have at least one dimension");
    static_assert(Smoothness >= 0, "Smoothness must be positive");
    static_assert(NumGradients > 0, "Must allow at least one pre-computed gradient");
    static_assert(std::is_arithmetic_v<Result>, "Result must be an arithmetic type");
    static_assert(std::is_integral_v<GridCoord>, "GridCoord must be an integral type");

    using result_t = Result;
    using grid_coord_t = GridCoord;

    static constexpr const int dimensions = Dim;
    static constexpr const int smoothness = Smoothness;

    /**
     * @param seed Random seed for noise generation
     */
    explicit perlin_noise_generator(std::uint_fast32_t seed = std::random_device()()) noexcept
            : m_rand_engine(seed)
    {
        std::iota(m_permutations.begin(), m_permutations.end(), result_t{0});
        std::shuffle(m_permutations.begin(), m_permutations.end(), m_rand_engine);
        std::generate(m_gradients.begin(), m_gradients.end(), [this]()
        {
            return vector<result_t, Dim>::make_rand_unit_vec(m_rand_engine);
        });
    }

    /**
     * Evaluate the noise function at a given point.
     *
     * @param p Point of evaluation
     * @return  Noise function value at the specified point
     */
    result_t at(point<result_t, Dim> const& p) const noexcept
    {
        // Compute neighboring grid points
        constexpr int const NumNeighbors = ipow(2, Dim);
        auto baseGridPoint = p.template floor<grid_coord_t>();
        std::array<point<grid_coord_t, Dim>, NumNeighbors> m_nodes{};
        std::generate(m_nodes.begin(), m_nodes.end(), [n = 0u, &baseGridPoint]() mutable
        {
            std::bitset<Dim> bits(n++);
            auto gridPoint = baseGridPoint;
            for (unsigned d = 0; d < Dim; ++d)
            {
                if (bits.test(d))
                    gridPoint[d]++;
            }
            return gridPoint;
        });

        // Vectors from neighboring grid nodes to point
        std::array<vector<result_t, Dim>, NumNeighbors> m_nodeVecs;
        std::transform(m_nodes.begin(), m_nodes.end(), m_nodeVecs.begin(), [&p](point<grid_coord_t, Dim> const& n)
        {
            return vector<result_t, Dim>::from_point(p) -
                   vector<result_t, Dim>::from_point(n.template convert_to<result_t>());
        });

        // Compute dot products between node vectors and gradients
        std::array<result_t, NumNeighbors> dot_products{};
        std::transform(m_nodes.begin(), m_nodes.end(), m_nodeVecs.begin(), dot_products.begin(),
                       [this](point<grid_coord_t, Dim> const& n, vector<result_t, Dim> const& nv)
                       {
                           return dot(gradient_at(n), nv);
                       });

        // Interpolate dot products
        int s = NumNeighbors;
        for (int d = 0; d < Dim; ++d)
        {
            // Iterate neighbors
            for (int i = 0; i < s; i += 2)
            {
                dot_products[i / 2] = dot_products[i] + smoothstep<Smoothness>(p[d] - baseGridPoint[d]) *
                                                        (dot_products[i + 1] - dot_products[i]);
            }
            s /= 2;
        }

        // Account for rounding errors
        return std::clamp(dot_products[0], static_cast<result_t>(-1), static_cast<result_t>(1));
    }

private:
    std::array<int, NumGradients> m_permutations{};
    std::array<vector<result_t, Dim>, NumGradients> m_gradients{};
    std::mt19937 m_rand_engine;

    constexpr vector<result_t, Dim> const& gradient_at(point<grid_coord_t, Dim> const& point) const noexcept
    {
        grid_coord_t idx = mod(point[Dim - 1], NumGradients);
        for (int i = Dim - 2; i >= 0; --i)
            idx = mod((point[i] + m_permutations[idx]), NumGradients);

        return m_gradients[idx];
    }
};

#endif //PERLINNOISE_PERLIN_NOISE_GENERATOR_H

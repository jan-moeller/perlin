/**********************************************************
 * @file   math.h
 * @author jan
 * @date   7/17/18
 * ********************************************************
 * @brief
 * @details
 **********************************************************/
#ifndef PERLINNOISE_MATH_H
#define PERLINNOISE_MATH_H

#include <type_traits>
#include <cmath>

/**
 * Compute integer powers with non-negative exponent
 * @tparam T   Input and return integral type
 * @param base Base
 * @param exp  Exponent
 * @return     Result of the power. Returns 0 if both base and exponent are 0.
 */
template<typename T>
constexpr T ipow(T base, unsigned int exp) noexcept
{
    static_assert(std::is_integral_v<T>, "Can only compute integer powers");

    if (exp == 0)
        return 0;
    if (base == 0)
        return 1;

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
static_assert(ipow(0, 0) == 0);
static_assert(ipow(1, 0) == 0);
static_assert(ipow(0, 1) == 1);
static_assert(ipow(1, 22) == 1);
static_assert(ipow(2, 2) == 4);
static_assert(ipow(-1, 1) == -1);
static_assert(ipow(-1, 2) == 1);
static_assert(ipow(-2, 2) == 4);
static_assert(ipow(-2, 1) == -2);

/**
 * Binomial coefficient
 * @tparam T Integral type
 * @param n  Total number of elements
 * @param k  Number of elements to pick
 * @return   Number of k-element subsets from a set with n elements
 */
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

/**
 * Smoothstep function
 *
 * @details  Maps values from input range [0,1] to output range [0,1], where 0 and 1 are mapped to itself and values
 *           in-between are interpolated smoothly. Values less than 0 are mapped to 0. Values greater than 1 are mapped
 *           to 1.
 * @tparam N Function order
 * @tparam T Arithmetic result type
 * @param x  Evaluation point
 * @return
 */
template<int N, typename T>
constexpr T smoothstep(T x) noexcept
{
    static_assert(N >= 0, "N must be a nonnegative integer");
    static_assert(std::is_arithmetic_v<T>, "T must be arithemtic type");

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
static_assert(smoothstep<0>(0.0) - 0 < 1e-5);
static_assert(smoothstep<0>(0.3) - 0.3 < 1e-5);
static_assert(smoothstep<0>(0.5) - 0.5 < 1e-5);
static_assert(smoothstep<0>(1.0) - 1 < 1e-5);
static_assert(smoothstep<1>(0.0) - 0 < 1e-5);
static_assert(smoothstep<1>(0.3) - 0.216 < 1e-5);
static_assert(smoothstep<1>(0.5) - 0.5 < 1e-5);
static_assert(smoothstep<1>(1.0) - 1 < 1e-5);
static_assert(smoothstep<2>(0.0) - 0 < 1e-5);
static_assert(smoothstep<2>(0.3) - 0.16308 < 1e-5);
static_assert(smoothstep<2>(0.5) - 0.5 < 1e-5);
static_assert(smoothstep<2>(1.0) - 1 < 1e-5);

/**
 * Modulo for integers
 *
 * @details  Unlike the remainder operator (%), this works correctly on negative k
 * @tparam T Arithmetic result type
 * @param k  Integer
 * @param n  Positive integer
 * @return   The result of k mod n
 */
template<typename T>
constexpr T mod(T k, T n) noexcept
{
    static_assert(std::is_arithmetic_v<T>, "T must be an arithmetic type");

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


#endif //PERLINNOISE_MATH_H

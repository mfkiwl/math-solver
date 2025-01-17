#ifndef STAN_MATH_PRIM_FUN_ASIN_HPP
#define STAN_MATH_PRIM_FUN_ASIN_HPP

#include <stan/math/prim/core.hpp>
#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/fun/asinh.hpp>
#include <stan/math/prim/fun/copysign.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/prim/fun/i_times.hpp>
#include <stan/math/prim/fun/value_of_rec.hpp>
#include <stan/math/prim/functor/apply_scalar_unary.hpp>
#include <stan/math/prim/functor/apply_vector_unary.hpp>
#include <cmath>
#include <complex>

namespace stan {
namespace math {

/**
 * Return the arc sine of the arithmetic argument.
 *
 * @tparam V `Arithmetic` argument
 * @param[in] x argument
 * @return arc sine of the argument
 */
template <typename T, require_arithmetic_t<T>* = nullptr>
inline auto asin(const T x) {
  return std::asin(x);
}

/**
 * Return the arc sine of the complex argument.
 *
 * @tparam V `complex<Arithmetic> argument
 * @param[in] x argument
 * @return arc sine of the argument
 */
template <typename T, require_complex_bt<std::is_arithmetic, T>* = nullptr>
inline auto asin(const T x) {
  return std::asin(x);
}

/**
 * Structure to wrap `asin()` so it can be vectorized.
 *
 * @tparam T type of argument
 * @param x argument
 * @return Arcsine of x in radians.
 */
struct asin_fun {
  template <typename T>
  static inline auto fun(const T& x) {
    return asin(x);
  }
};

/**
 * Returns the elementwise `asin()` of the input,
 * which may be a scalar or any Stan container of numeric scalars.
 *
 * @tparam Container type of container
 * @param x container
 * @return Arcsine of each variable in the container, in radians.
 */
template <typename Container, require_ad_container_t<Container>* = nullptr>
inline auto asin(const Container& x) {
  return apply_scalar_unary<asin_fun, Container>::apply(x);
}

/**
 * Version of `asin()` that accepts std::vectors, Eigen Matrix/Array objects,
 *  or expressions, and containers of these.
 *
 * @tparam Container Type of x
 * @param x Container
 * @return Arcsine of each variable in the container, in radians.
 */
template <typename Container,
          require_container_bt<std::is_arithmetic, Container>* = nullptr>
inline auto asin(const Container& x) {
  return apply_vector_unary<Container>::apply(
      x, [](const auto& v) { return v.array().asin(); });
}

namespace internal {
/**
 * Return the arc sine of the complex argument.
 *
 * @tparam V value type of argument
 * @param[in] z argument
 * @return arc sine of the argument
 */
template <typename V>
inline std::complex<V> complex_asin(const std::complex<V>& z) {
  auto y_d = asin(value_of_rec(z));
  auto y = neg_i_times(asinh(i_times(z)));
  return copysign(y, y_d);
}
}  // namespace internal

}  // namespace math
}  // namespace stan

#endif

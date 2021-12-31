#ifndef STAN_MATH_PRIM_FUN_INC_BETA_INV_HPP
#define STAN_MATH_PRIM_FUN_INC_BETA_INV_HPP

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/boost_policy.hpp>
#include <boost/math/special_functions/beta.hpp>

namespace stan {
namespace math {

/**
 * The inverse of the normalized incomplete beta function of a, b, with
 * probability p.
 *
 * Used to compute the cumulative density function for the beta
 * distribution.
 *
 * @param a Shape parameter a >= 0; a and b can't both be 0
 * @param b Shape parameter b >= 0
 * @param p Random variate. 0 <= p <= 1
 * @throws if constraints are violated or if any argument is NaN
 * @return The inverse of the normalized incomplete beta function.
 */
inline double inc_beta_inv(double a, double b, double p) {
  check_not_nan("inc_beta", "a", a);
  check_not_nan("inc_beta", "b", b);
  check_not_nan("inc_beta", "p", p);
  return boost::math::ibeta_inv(a, b, p, boost_policy_t<>());
}

}  // namespace math
}  // namespace stan
#endif

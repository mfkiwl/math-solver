#ifndef STAN_MATH_REV_FUNCTOR_HESSIAN_TIMES_VECTOR_HPP
#define STAN_MATH_REV_FUNCTOR_HESSIAN_TIMES_VECTOR_HPP

#include <stan/math/prim/fun/constants.hpp>
#include <stan/math/rev/meta.hpp>
#include <stan/math/rev/core.hpp>
#include <stan/math/prim/fun/Eigen.hpp>
#include <stan/math/rev/functor.hpp>
#include <cmath>

namespace stan {
namespace math {
namespace internal {

/**
 * Calculate the value and the product of the Hessian and the specified
 * vector of the specified function at the specified argument using
 * central finite difference of gradients, automatically setting
 * the stepsize between the function evaluations along a dimension.
 *
 * <p>The functor must implement
 *
 * <code>
 * double operator()(const Eigen::Matrix<stan::math::var, -1, 1>&)
 * </code>
 *
 * <p>For details of the algorithm, see
 * https://justindomke.wordpress.com/2009/01/17/hessian-vector-products/
 *
 * <p>Step size for dimension `i` is set automatically using
 * `stan::math::finite_diff_stepsize(v.normalized()(i))`.
 *
 * 2 gradient calls are needed for the algorithm.
 *
 * @tparam F Type of function
 * @param[in] f Function
 * @param[in] x Argument to function
 * @param[in] v Vector to multiply Hessian with
 * @param[out] fx Function applied to argument
 * @param[out] hvp Product of Hessian and vector at argument
 */
template <typename F>
void finite_diff_hessian_times_vector_auto(const F& f, const Eigen::VectorXd& x,
                                           const Eigen::VectorXd& v, double& fx,
                                           Eigen::VectorXd& hvp) {
  fx = f(x);

  double v_norm = v.norm();
  auto v_normalized = v / v_norm;

  int d = x.size();

  double epsilon = std::cbrt(EPSILON);

  auto v_eps = epsilon * v_normalized;

  double tmp;
  Eigen::VectorXd grad_forward(d);
  gradient(f, x + v_eps, tmp, grad_forward);

  Eigen::VectorXd grad_backward(d);
  gradient(f, x - v_eps, tmp, grad_backward);

  hvp.resize(d);
  hvp = v_norm * (grad_forward - grad_backward) / (2 * epsilon);
}
}  // namespace internal
}  // namespace math
}  // namespace stan
#endif

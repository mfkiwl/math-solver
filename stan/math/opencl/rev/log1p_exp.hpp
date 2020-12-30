#ifndef STAN_MATH_OPENCL_REV_LOG1P_EXP_HPP
#define STAN_MATH_OPENCL_REV_LOG1P_EXP_HPP
#ifdef STAN_OPENCL

#include <stan/math/opencl/kernel_generator.hpp>
#include <stan/math/rev/core.hpp>
#include <stan/math/rev/fun/value_of.hpp>

namespace stan {
namespace math {

/**
 * Returns the elementwise `log1p_exp()` of a var_value<matrix_cl<double>>.
 *
 * @param A argument
 * @return Elementwise `log1p_exp()` of the input.
 */
inline var_value<matrix_cl<double>> log1p_exp(
    const var_value<matrix_cl<double>>& A) {
  var_value<matrix_cl<double>> res = log1p_exp(A.val());

  reverse_pass_callback([A, res]() mutable {
    A.adj() = A.adj() + elt_multiply(res.adj(), inv_logit(A.val()));
  });

  return res;
}

}  // namespace math
}  // namespace stan

#endif
#endif
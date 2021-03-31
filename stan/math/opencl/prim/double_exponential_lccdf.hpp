#ifndef STAN_MATH_OPENCL_PRIM_DOUBLE_EXPONENTIAL_LCCDF_HPP
#define STAN_MATH_OPENCL_PRIM_DOUBLE_EXPONENTIAL_LCCDF_HPP
#ifdef STAN_OPENCL

#include <stan/math/prim/meta.hpp>
#include <stan/math/prim/err.hpp>
#include <stan/math/prim/fun/constants.hpp>
#include <stan/math/prim/fun/elt_divide.hpp>
#include <stan/math/prim/fun/elt_multiply.hpp>
#include <stan/math/opencl/kernel_generator.hpp>
#include <stan/math/prim/functor/operands_and_partials.hpp>

namespace stan {
namespace math {

/** \ingroup opencl
 * Returns the double exponential log complementary cumulative density
 * function. Given containers of matching sizes, returns the log sum of
 * probabilities.
 *
 * @tparam T_y_cl type of scalar outcome
 * @tparam T_loc_cl type of location
 * @tparam T_scale_cl type of scale
 * @param y (Sequence of) scalar(s).
 * @param mu (Sequence of) location(s).
 * @param sigma (Sequence of) scale(s).
 * @return The log of the product of densities.
 */
template <
    typename T_y_cl, typename T_loc_cl, typename T_scale_cl,
    require_all_prim_or_rev_kernel_expression_t<T_y_cl, T_loc_cl,
                                                T_scale_cl>* = nullptr,
    require_any_not_stan_scalar_t<T_y_cl, T_loc_cl, T_scale_cl>* = nullptr>
return_type_t<T_y_cl, T_loc_cl, T_scale_cl> double_exponential_lccdf(
    const T_y_cl& y, const T_loc_cl& mu, const T_scale_cl& sigma) {
  static const char* function = "double_exponential_lccdf(OpenCL)";
  using T_partials_return = partials_return_t<T_y_cl, T_loc_cl, T_scale_cl>;
  using std::isfinite;
  using std::isnan;

  check_consistent_sizes(function, "Random variable", y, "Location parameter",
                         mu, "Scale parameter", sigma);
  const size_t N = max_size(y, mu, sigma);
  if (N == 0) {
    return 0.0;
  }

  const auto& y_col = as_column_vector_or_scalar(y);
  const auto& mu_col = as_column_vector_or_scalar(mu);
  const auto& sigma_col = as_column_vector_or_scalar(sigma);

  const auto& y_val = value_of(y_col);
  const auto& mu_val = value_of(mu_col);
  const auto& sigma_val = value_of(sigma_col);

  auto check_y_not_nan
      = check_cl(function, "Random variable", y_val, "not NaN");
  auto y_not_nan_expr = !isnan(y_val);
  auto check_mu_finite
      = check_cl(function, "Location parameter", mu_val, "finite");
  auto mu_finite_expr = isfinite(mu_val);
  auto check_sigma_positive_finite
      = check_cl(function, "Scale parameter", sigma_val, "positive finite");
  auto sigma_positive_finite_expr = 0 < sigma_val && isfinite(sigma_val);

  auto sigma_inv = elt_divide(1.0, sigma_val);
  auto scaled_diff = elt_multiply(y_val - mu_val, sigma_inv);
  auto cond = y_val < mu_val;
  auto mu_deriv = select(
      cond, elt_divide(sigma_inv, 2.0 * exp(-scaled_diff) - 1.0), sigma_inv);
  auto ccdf_log_expr = colwise_sum(
      select(cond, log1m(0.5 * exp(scaled_diff)), LOG_HALF - scaled_diff));
  auto y_deriv = -mu_deriv;
  auto sigma_deriv = elt_multiply(mu_deriv, scaled_diff);

  matrix_cl<double> ccdf_log_cl;
  matrix_cl<double> mu_deriv_cl;
  matrix_cl<double> y_deriv_cl;
  matrix_cl<double> sigma_deriv_cl;

  results(check_y_not_nan, check_mu_finite, check_sigma_positive_finite,
          ccdf_log_cl, y_deriv_cl, mu_deriv_cl, sigma_deriv_cl)
      = expressions(y_not_nan_expr, mu_finite_expr, sigma_positive_finite_expr,
                    ccdf_log_expr,
                    calc_if<!is_constant<T_y_cl>::value>(y_deriv),
                    calc_if<!is_constant<T_loc_cl>::value>(mu_deriv),
                    calc_if<!is_constant<T_scale_cl>::value>(sigma_deriv));

  T_partials_return ccdf_log = (from_matrix_cl(ccdf_log_cl)).sum();

  operands_and_partials<decltype(y_col), decltype(mu_col), decltype(sigma_col)>
      ops_partials(y_col, mu_col, sigma_col);

  if (!is_constant<T_y_cl>::value) {
    ops_partials.edge1_.partials_ = std::move(y_deriv_cl);
  }
  if (!is_constant<T_loc_cl>::value) {
    ops_partials.edge2_.partials_ = std::move(mu_deriv_cl);
  }
  if (!is_constant<T_scale_cl>::value) {
    ops_partials.edge3_.partials_ = std::move(sigma_deriv_cl);
  }
  return ops_partials.build(ccdf_log);
}

}  // namespace math
}  // namespace stan
#endif
#endif

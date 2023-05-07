#ifndef STAN_MATH_PRIM_PROB_WIENER7_LPDF_HPP
#define STAN_MATH_PRIM_PROB_WIENER7_LPDF_HPP

#include <stan/math/prim/fun.hpp>
#include <stan/math/prim/functor/hcubature.hpp>
#include <stan/math/prim/prob/wiener5_lpdf.hpp>

namespace stan {
namespace math {
namespace internal {

template <typename F, typename... TArgs>
auto wiener7_integrand(const F& integrand_fun, double labstol_wiener5,
                       double lerr_bound, TArgs&&... args) {
  const auto& wiener7_integrand_impl
      = [&](std::vector<double> x, double y, double a, double v, double w,
            double t0, double sv, double sw, double st0, double lerr,
            auto&&... args_int) {
          scalar_seq_view<decltype(x)> x_vec(x);
          double omega = sw ? w + sw * (x_vec[0] - 0.5) : w;
          double t0_ = sw ? (st0 ? t0 + st0 * x_vec[1] : t0)
                          : (st0 ? t0 + st0 * x_vec[0] : t0);
          if (y - t0_ <= 0) {
            return 0.0;
          } else {
            return integrand_fun(t0_, omega, y, a, v, w, t0, sv, sw, st0, lerr,
                                 args_int...);
          }
        };

  double err = labstol_wiener5 - lerr_bound + LOG_TWO + 1;
  const auto& functor = [&](auto&&... int_args) {
    return hcubature(wiener7_integrand_impl, int_args...);
  };
  return estimate_with_err_check<0>(functor, err, std::make_tuple(args...));
}

template <FunType FunTypeEnum>
inline double wiener7_impl(double t0_, double omega, double y, double a,
                           double v, double w, double t0, double sv, double sw,
                           double st0, double lerr) {
  if (FunTypeEnum == FunType::GradT || FunTypeEnum == FunType::GradA
      || FunTypeEnum == FunType::GradW) {
    return wiener5_helper<FunTypeEnum>(y - t0_, a, v, omega, sv, lerr, 1);
  }
  double dens
    = exp(wiener5_helper<FunType::Density>(y - t0_, a, v, omega, sv, lerr));
  if (FunTypeEnum == FunType::Density) {
    return dens;
  } else {
    return wiener5_helper<FunTypeEnum>(y - t0_, a, v, omega, sv) * dens;
  }
}

inline double int_dswddiff(double t0_, double omega, double y, double a,
                           double v, double w, double t0, double sv, double sw,
                           double st0, double lerr, double w_lower,
                           double w_upper, double sw_mean) {
  double fl
    = exp(wiener5_helper<FunType::Density>(y - t0_, a, v, w_lower, sv, lerr));
  double fu
    = exp(wiener5_helper<FunType::Density>(y - t0_, a, v, w_upper, sv, lerr));
  return 0.5 * (fl + fu) / sw_mean;
}
}  // namespace internal

template <bool propto, typename T_y, typename T_a, typename T_t0, typename T_w,
          typename T_v, typename T_sv, typename T_sw, typename T_st0>
inline return_type_t<T_y, T_a, T_t0, T_w, T_v, T_sv, T_sw, T_st0> wiener7_lpdf(
    const T_y& y, const T_a& a, const T_t0& t0, const T_w& w, const T_v& v,
    const T_sv& sv, const T_sw& sw, const T_st0& st0, const double& prec) {
  using T_y_ref = ref_type_t<T_y>;
  using T_a_ref = ref_type_t<T_a>;
  using T_v_ref = ref_type_t<T_v>;
  using T_w_ref = ref_type_t<T_w>;
  using T_t0_ref = ref_type_t<T_t0>;
  using T_sv_ref = ref_type_t<T_sv>;
  using T_sw_ref = ref_type_t<T_sw>;
  using T_st0_ref = ref_type_t<T_st0>;

  const char* function_name = "wiener7_lpdf";
  check_consistent_sizes(function_name, "Random variable", y,
                         "Boundary separation", a, "Drift rate", v,
                         "A-priori bias", w, "Nondecision time", t0,
                         "Inter-trial variability in drift rate", sv,
                         "Inter-trial variability in A-priori bias", sw,
                         "Inter-trial variability in Nondecision time", st0);
  check_consistent_size(function_name, "Random variable", y, 1);
  check_consistent_size(function_name, "Boundary separation", a, 1);
  check_consistent_size(function_name, "Drift rate", v, 1);
  check_consistent_size(function_name, "A-priori bias", w, 1);
  check_consistent_size(function_name, "Nondecision time", t0, 1);
  check_consistent_size(function_name, "Inter-trial variability in drift rate",
                        sv, 1);
  check_consistent_size(function_name,
                        "Inter-trial variability in A-priori bias", sw, 1);
  check_consistent_size(function_name,
                        "Inter-trial variability in Nondecision time", st0, 1);

  T_y_ref y_ref = y;
  T_a_ref a_ref = a;
  T_v_ref v_ref = v;
  T_w_ref w_ref = w;
  T_t0_ref t0_ref = t0;
  T_sv_ref sv_ref = sv;
  T_sw_ref sw_ref = sw;
  T_st0_ref st0_ref = st0;

  check_positive_finite(function_name, "Random variable", value_of(y_ref));
  check_positive_finite(function_name, "Boundary separation", value_of(a_ref));
  check_finite(function_name, "Drift rate", value_of(v_ref));
  check_less(function_name, "A-priori bias", value_of(w_ref), 1);
  check_greater(function_name, "A-priori bias", value_of(w_ref), 0);
  check_nonnegative(function_name, "Nondecision time", value_of(t0_ref));
  check_finite(function_name, "Nondecision time", value_of(t0_ref));
  check_nonnegative(function_name, "Inter-trial variability in drift rate",
                    value_of(sv_ref));
  check_finite(function_name, "Inter-trial variability in drift rate",
               value_of(sv_ref));
  check_bounded(function_name, "Inter-trial variability in A-priori bias",
                value_of(sw_ref), 0, 1);
  check_nonnegative(function_name,
                    "Inter-trial variability in Nondecision time",
                    value_of(st0_ref));
  check_finite(function_name, "Inter-trial variability in Nondecision time",
               value_of(st0_ref));

  if (size_zero(y, a, v, w, t0) || size_zero(sv, sw, st0)) {
    return 0;
  }
  size_t N = max_size(y, a, v, w, t0, sv, sw, st0);
  if (!N) {
    return 0;
  }
  scalar_seq_view<T_y_ref> y_vec(y_ref);
  scalar_seq_view<T_a_ref> a_vec(a_ref);
  scalar_seq_view<T_v_ref> v_vec(v_ref);
  scalar_seq_view<T_w_ref> w_vec(w_ref);
  scalar_seq_view<T_t0_ref> t0_vec(t0_ref);
  scalar_seq_view<T_sv_ref> sv_vec(sv_ref);
  scalar_seq_view<T_sw_ref> sw_vec(sw_ref);
  scalar_seq_view<T_st0_ref> st0_vec(st0_ref);
  size_t N_y_t0 = max_size(y, t0, st0);

  for (size_t i = 0; i < N_y_t0; ++i) {
    if (y_vec[i] <= t0_vec[i]) {
      std::stringstream msg;
      msg << ", but must be greater than nondecision time = " << t0_vec[i];
      std::string msg_str(msg.str());
      throw_domain_error(function_name, "Random variable", y_vec[i], " = ",
                         msg_str.c_str());
    }
  }
  size_t N_beta_sw = max_size(w, sw);
  for (size_t i = 0; i < N_beta_sw; ++i) {
    if (w_vec[i] - .5 * sw_vec[i] <= 0) {
      std::stringstream msg;
      msg << ", but must be smaller than 2*(A-priori bias) = " << 2 * w_vec[i];
      std::string msg_str(msg.str());
      throw_domain_error(function_name,
                         "Inter-trial variability in A-priori bias", sw_vec[i],
                         " = ", msg_str.c_str());
    }
    if (w_vec[i] + .5 * sw_vec[i] >= 1) {
      std::stringstream msg;
      msg << ", but must be smaller than 2*(1-A-priori bias) = "
          << 2 * (1 - w_vec[i]);
      std::string msg_str(msg.str());
      throw_domain_error(function_name,
                         "Inter-trial variability in A-priori bias", sw_vec[i],
                         " = ", msg_str.c_str());
    }
  }
  if (!include_summand<propto, T_y, T_a, T_v, T_w, T_t0, T_sv, T_sw,
                       T_st0>::value) {
    return 0;
  }

  double error_bound_dens = 1e-6;  // precision for density
  double lerror_bound_dens = log(error_bound_dens);
  double error_bound = prec;  // precision for
  // derivatives (controllable by user)
  double lerror_bound = log(error_bound);  // log(alpha)
  double abstol = 0.0;
  double reltol = .9 * error_bound;  // eps_rel(Integration)
  double abstol_wiener5 = 1e-12;     // eps_abs(wiener5)
  double labstol_wiener5 = log(abstol_wiener5);
  // log(eps_abs(wiener5)
  int Meval = 6000;
  double dens = 0.0;
  double ld = 0.0;
  operands_and_partials<T_y_ref, T_a_ref, T_t0_ref, T_w_ref, T_v_ref, T_sv_ref,
                        T_sw_ref, T_st0_ref>
      ops_partials(y_ref, a_ref, t0_ref, w_ref, v_ref, sv_ref, sw_ref, st0_ref);
  static constexpr double LOG_FOUR = LOG_TWO + LOG_TWO;
  static constexpr double LOG_POINT1 = -1;

  // calculate density and partials
  for (size_t i = 0; i < N; i++) {
    const double y_val = y_vec.val(i);
    const double a_val = a_vec.val(i);
    const double v_val = v_vec.val(i);
    const double w_val = w_vec.val(i);
    const double t0_val = t0_vec.val(i);
    const double sv_val = sv_vec.val(i);
    const double sw_val = sw_vec.val(i);
    const double st0_val = st0_vec.val(i);
    const auto params
        = std::make_tuple(y_val, a_val, v_val, w_val, t0_val, sv_val, sw_val,
                          st0_val, labstol_wiener5 - LOG_TWO);
    int dim = (sw_val != 0) + (st0_val != 0);
    check_positive(function_name,
                   "(Inter-trial variability in A-priori bias) + "
                   "(Inter-trial variability in nondecision time)",
                   dim);

    std::vector<double> xmin(dim, 0);
    std::vector<double> xmax(dim, 1);
    if (st0_val) {
      xmax[dim - 1] = fmin(1.0, (y_val - t0_val) / st0_val);
    }

    dens = internal::wiener7_integrand(
        internal::wiener7_impl<internal::FunType::Density>, labstol_wiener5,
        lerror_bound_dens, params, dim, xmin, xmax, Meval, abstol, reltol / 2);
    double log_dens = log(dens);
    ld += log_dens;

    // computation of derivative for t and precision check in order to give
    // the value as deriv_t to edge1 and as -deriv_t to edge5
    double deriv_t_7 = internal::wiener7_integrand(
                           internal::wiener7_impl<internal::FunType::GradT>,
                           labstol_wiener5, lerror_bound + log_dens, params,
                           dim, xmin, xmax, Meval, abstol, reltol / 2)
                       / dens;

    // computation of derivatives and precision checks
    double deriv;
    if (!is_constant_all<T_y>::value) {
      ops_partials.edge1_.partials_[i] = deriv_t_7;
    }
    if (!is_constant_all<T_a>::value) {
      ops_partials.edge2_.partials_[i]
          = internal::wiener7_integrand(
                internal::wiener7_impl<internal::FunType::GradA>,
                labstol_wiener5, lerror_bound + log_dens, params, dim, xmin,
                xmax, Meval, abstol, reltol / 2)
            / dens;
    }
    if (!is_constant_all<T_t0>::value) {
      ops_partials.edge3_.partials_[i] = -deriv_t_7;
    }
    if (!is_constant_all<T_w>::value) {
      ops_partials.edge4_.partials_[i]
          = internal::wiener7_integrand(
                internal::wiener7_impl<internal::FunType::GradW>,
                labstol_wiener5, lerror_bound + log_dens, params, dim, xmin,
                xmax, Meval, abstol, reltol / 2)
            / dens;
    }
    if (!is_constant_all<T_v>::value) {
      ops_partials.edge5_.partials_[i]
          = internal::wiener7_integrand(
                internal::wiener7_impl<internal::FunType::GradV>,
                labstol_wiener5, lerror_bound + log_dens, params, dim, xmin,
                xmax, Meval, abstol, reltol / 2)
            / dens;
    }
    if (!is_constant_all<T_sv>::value) {
      ops_partials.edge6_.partials_[i]
          = internal::wiener7_integrand(
                internal::wiener7_impl<internal::FunType::GradSV>,
                labstol_wiener5, lerror_bound + log_dens, params, dim, xmin,
                xmax, Meval, abstol, reltol / 2)
            / dens;
    }
    if (!is_constant_all<T_sw>::value) {
      if (sw_val == 0) {
        ops_partials.edge7_.partials_[i] = 0;
      } else {
        double lower = w_val - sw_val / 2;
        double upper = w_val + sw_val / 2;

        const auto& params_sw = std::make_tuple(
            y_val, a_val, v_val, w_val, t0_val, sv_val, 0, st0_val,
            labstol_wiener5 - LOG_TWO, (0 > lower) ? 0 : lower,
            (1 < upper) ? 1 : upper, sw_val);
        if (st0_val == 0) {
          deriv = internal::estimate_with_err_check<8>(
              internal::int_dswddiff, lerror_bound - LOG_TWO,
              std::tuple_cat(std::make_tuple(t0_val, 0), params_sw));
        } else {
          deriv = internal::wiener7_integrand(
              internal::int_dswddiff, labstol_wiener5, lerror_bound, params_sw,
              1, xmin, xmax, Meval, abstol, reltol / 2);
        }
        ops_partials.edge7_.partials_[i] = deriv / dens - 1 / sw_val;
      }
    }
    if (!is_constant_all<T_st0>::value) {
      double f;
      if (st0_val == 0) {
        ops_partials.edge8_.partials_[i] = 0;
      } else if (y_val - (t0_val + st0_val) <= 0) {
        ops_partials.edge8_.partials_[i] = -1 / st0_val;
      } else {
        double t0_st0 = t0_val + st0_val;
        if (sw_val == 0) {
          f = internal::estimate_with_err_check<8>(
              internal::wiener7_impl<internal::FunType::Density>,
              lerror_bound + log(st0_val),
              std::tuple_cat(std::make_tuple(t0_st0, w_val), params));
        } else {
          double new_error = labstol_wiener5 - LOG_TWO;

          const auto& params_st = std::make_tuple(
              y_val, a_val, v_val, w_val, t0_st0, sv_val, sw_val, 0, new_error);
          f = internal::wiener7_integrand(
              internal::wiener7_impl<internal::FunType::Density>,
              labstol_wiener5, lerror_bound, params_st, 1, xmin, xmax, Meval,
              abstol, reltol / 2);
        }
        ops_partials.edge8_.partials_[i] = -1 / st0_val + f / st0_val / dens;
      }
    }
    std::vector<double>().swap(xmin);
    std::vector<double>().swap(xmax);
  }
  return ops_partials.build(ld);
}
}  // namespace math
}  // namespace stan
#endif

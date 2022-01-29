#ifndef STAN_MATH_LAPLACE_LAPLACE_LIKELIHOOD_BERNOULLI_LOGIT_HPP
#define STAN_MATH_LAPLACE_LAPLACE_LIKELIHOOD_BERNOULLI_LOGIT_HPP

namespace stan {
namespace math {

struct bernoulli_logit_likelihood {
  template <typename T_theta, typename T_eta>
  inline stan::return_type_t<T_theta, T_eta> operator()(
      const Eigen::Matrix<T_theta, -1, 1>& theta,
      const Eigen::Matrix<T_eta, -1, 1>& eta, const Eigen::VectorXd& y,
      const std::vector<int>& delta_int, std::ostream* pstream) const {
    return sum(theta.cwiseProduct(y)
               - to_vector(delta_int).cwiseProduct(log(add(1.0, exp(theta)))));
  }
};

/**
 * A structure to compute the log density, first, second,
 * and third-order derivatives for a Bernoulli logistic likelihood
 * whith multiple groups.
 * This structure can be passed to the the laplace_marginal function.
 * Uses sufficient statistics for the data.
 */
struct diff_bernoulli_logit {
  /* The number of samples in each group. */
  Eigen::VectorXd n_samples_;
  /* The sum of counts in each group. */
  Eigen::VectorXd sums_;
  template <typename Vec1, typename Vec2>
  diff_bernoulli_logit(Vec1&& n_samples, Vec2&& sums)
      : n_samples_(std::forward<Vec1>(n_samples)),
        sums_(std::forward<Vec2>(sums)) {}

  /**
   * Return the log density.
   * @tparam T type of the log poisson parameter.
   * @param[in] theta log poisson parameters for each group.
   * @return the log density.
   */
  template <typename T1, typename T2>
  inline T1 log_likelihood(
      const Eigen::Matrix<T1, Eigen::Dynamic, 1>& theta,
      const Eigen::Matrix<T2, Eigen::Dynamic, 1>& eta_dummy) const {
    return sum(theta.cwiseProduct(sums_)
               - n_samples_.cwiseProduct(
                   log(add(Eigen::VectorXd::Ones(theta.size()), exp(theta)))));
  }

  /**
   * Returns the gradient of the log density, and the hessian.
   * Since the latter is diagonal, it is stored inside a vector.
   * The two objects are computed together, because we always use
   * both when solving the Newton iteration of the Laplace
   * approximation, and to avoid redundant computation.
   * @tparam T type of the Bernoulli logistic parameter.
   * @param[in] theta Bernoulli logistic parameters for each group.
   * @param[in, out] gradient
   * @param[in, out] hessian diagonal, so stored in a vector.
   */
  template <typename T1, typename T2>
  inline void diff(const Eigen::Matrix<T1, Eigen::Dynamic, 1>& theta,
                   const Eigen::Matrix<T2, Eigen::Dynamic, 1>& /* eta */,
                   Eigen::Matrix<T1, Eigen::Dynamic, 1>& gradient,
                   Eigen::SparseMatrix<double>& hessian,
                   int block_size_dummy = 0) const {
    Eigen::Matrix<T1, Eigen::Dynamic, 1> exp_theta = exp(theta);
    const Eigen::Index theta_size = theta.size();
    Eigen::VectorXd one = rep_vector(1, theta_size);
    gradient = sums_ - n_samples_.cwiseProduct(inv_logit(theta));
    Eigen::Matrix<T1, Eigen::Dynamic, 1> hessian_diagonal
        = -n_samples_.cwiseProduct(
            elt_divide(exp_theta, square(add(1.0, exp_theta))));
    hessian.resize(theta_size, theta_size);
    hessian.reserve(Eigen::VectorXi::Constant(theta_size, 1));
    for (Eigen::Index i = 0; i < theta_size; i++)
      hessian.insert(i, i) = hessian_diagonal(i);
  }

  /**
   * Returns the third derivative tensor. Because it is (cubic) diagonal,
   * the object is stored in a vector.
   * @tparam T type of the log poisson parameter.
   * @param[in] theta log poisson parameters for each group.
   * @param[in] eta_dummy additional likelihood parameters (used for other lk)
   * @return A vector containing the non-zero elements of the third
   *         derivative tensor.
   */
  template <typename T1, typename T2>
  inline Eigen::Matrix<T1, Eigen::Dynamic, 1> third_diff(
      const Eigen::Matrix<T1, Eigen::Dynamic, 1>& theta,
      const Eigen::Matrix<T2, Eigen::Dynamic, 1>& /* eta */) const {
    Eigen::VectorXd exp_theta = exp(theta);
    Eigen::VectorXd nominator = exp_theta.cwiseProduct(
        exp_theta - Eigen::VectorXd::Ones(theta.size()));
    Eigen::VectorXd one_plus_exp_theta = add(1.0, exp_theta);
    Eigen::VectorXd denominator
        = square(one_plus_exp_theta).cwiseProduct(one_plus_exp_theta);

    return n_samples_.cwiseProduct(elt_divide(nominator, denominator));
  }
};

}  // namespace math
}  // namespace stan

#endif

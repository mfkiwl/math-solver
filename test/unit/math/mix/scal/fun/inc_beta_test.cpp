#include <test/unit/math/test_ad.hpp>
#include <limits>

TEST(mathMixScalFun, inc_beta) {
  auto f = [](const auto& x1, const auto& x2, const auto& x3) {
    return stan::math::inc_beta(x1, x2, x3);
  };

  // replicate pre-existing test cases
  stan::test::expect_ad(f, 0.6, 0.3, 0.5);
  stan::test::expect_ad(f, 0.9, 0.9, 0.9);
  stan::test::expect_ad(f, 1.0, 1.0, 0.4);

  // integer instantiations
  stan::test::expect_ad(f, 3, 2, 0.2);
  stan::test::expect_ad(f, 3, 2.0, 0.2);
  stan::test::expect_ad(f, 3.0, 2, 0.2);
  stan::test::expect_ad(f, 3.0, 2.0, 0.2);

  // test all nan instantiations
  double nan = std::numeric_limits<double>::quiet_NaN();
  // stan::test::expect_ad(f, 0.6, 0.3, nan);
  stan::test::expect_ad(f, 0.6, nan, 0.5);
  stan::test::expect_ad(f, 0.6, nan, nan);
  stan::test::expect_ad(f, nan, 0.3, 0.5);
  stan::test::expect_ad(f, nan, 0.3, nan);
  stan::test::expect_ad(f, nan, nan, 0.5);
  stan::test::expect_ad(f, nan, nan, nan);
}

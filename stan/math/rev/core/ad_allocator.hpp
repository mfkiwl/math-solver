#ifndef STAN_MATH_REV_CORE_AD_ALLOCATOR_HPP
#define STAN_MATH_REV_CORE_AD_ALLOCATOR_HPP

#include <stan/math/rev/core/chainablestack.hpp>

namespace stan {
namespace math {

/**
 * std library compatible allocator that uses AD stack.
 * @tparam T type of scalar
 */
template <typename T>
struct AD_allocator {
  using value_type = T;

  /**
   * Allocates space for `n` items of type `T`.
   *
   * @param n number of items to allocate space for
   * @return pointer to allocated space
   */
  T* allocate(std::size_t n) {
    return ChainableStack::instance_->memalloc_.alloc_array<T>(n);
  }

  /**
   * No-op. Memory is dealocated by caling `recover_memory()`.
   */
  void deallocate(T* /*p*/, std::size_t /*n*/) noexcept {}
};

}  // namespace math
}  // namespace stan

#endif
#pragma once
#include <limits>
#include <rpmalloc.h>
#include <new>
namespace Allocators {

// allocator interface for rpmalloc
template <class T> struct RPmallocator {
  typedef T value_type;

  RPmallocator() = default;

  template <class U> constexpr RPmallocator(const RPmallocator<U> &) noexcept {}

  [[nodiscard]] T *allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
      throw std::bad_array_new_length();

    if (auto p = static_cast<T *>(rpmalloc(n * sizeof(T)))) {
      return p;
    }

    throw std::bad_alloc();
  }

  void deallocate(T *p, std::size_t n) noexcept { rpfree(p); }
};

template <class T, class U>
bool operator==(const RPmallocator<T> &, const RPmallocator<U> &) {
  return true;
}

template <class T, class U>
bool operator!=(const RPmallocator<T> &, const RPmallocator<U> &) {
  return false;
}

} // namespace Allocators

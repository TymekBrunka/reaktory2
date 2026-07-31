#pragma once
#include <cstddef>
#include <cstdint>
#include <stdfloat>
#include <vector>
#include <stdatomic.h>
namespace Allocators {

class Arena {
  atomic_bool is_locked = false;
  _Float16 growth_factor = 2;
  atomic_uint_fast32_t page_size = 4096;
  atomic_uint_fast32_t offset = 0;
  unsigned char *memory = nullptr;
  Arena *next = nullptr;
  // std::vector<void *> larger_blobs;

public:
  Arena() = default;
  ~Arena();
  Arena(uint32_t page_size = 4096, _Float16 growth_factor = 1);
  Arena(const Arena &other) = delete;
  Arena &operator=(const Arena &other) = delete;
  Arena(Arena &&other) noexcept = delete;
  Arena &operator=(Arena &&other) noexcept = delete;

  void *alloc(uint32_t size);
  void attempt_free(void *ptr, uint32_t size);
};

template <class T> struct ArenaAllocator {
  Arena *arena = nullptr;

  typedef T value_type;

  ArenaAllocator() = default;

  template <class U>
  constexpr ArenaAllocator(const ArenaAllocator<U> &other) noexcept {
    arena = std::move(other.arena);
  }

  [[nodiscard]] T *allocate(std::size_t n) {
    void *ptr = arena->alloc(n * sizeof(T));
    if (ptr)
      return *(T *)ptr;

    throw std::bad_alloc();
  }

  void deallocate(T *p, std::size_t n) noexcept {
    arena->attempt_free(p, n * sizeof(T));
  }
};

template <class T, class U>
bool operator==(const ArenaAllocator<T> &a, const ArenaAllocator<U> &b) {
  return a.arena == b.arena;
}

template <class T, class U>
bool operator!=(const ArenaAllocator<T> &a, const ArenaAllocator<U> &b) {
  return a.arena != b.arena;
}

} // namespace Allocators

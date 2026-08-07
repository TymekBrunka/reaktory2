#pragma once
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdfloat>
#include <vector>
namespace Allocators {

// thread-local arena
class Arena {
  _Float16 growth_factor = 1;
  uint32_t page_size = 4096;
  uint32_t offset = 0;
  unsigned char *memory = nullptr;
  Arena *next = nullptr;
  std::vector<void *> larger_blobs;

public:
  Arena() = default;
  ~Arena();
  Arena(uint32_t page_size = 4096, _Float16 growth_factor = 1);
  Arena(const Arena &other) = delete;
  Arena &operator=(const Arena &other) = delete;
  Arena(Arena &&other) noexcept;
  Arena &operator=(Arena &&other) noexcept;

  void *alloc(uint32_t size);
  void attempt_free(void *ptr, uint32_t size);

  inline Arena *const get_next() { return next; }

  inline uint32_t const get_page_size() { return page_size; }

  inline uint32_t const get_page_memory_usage() { return offset; }

  inline uint32_t const get_page_free_size() { return page_size - offset; }

  inline uint32_t const get_usable_size() {
    uint32_t max = page_size - offset;
    for (Arena *arena = this; arena != nullptr; arena = arena->next)
      if (arena->get_page_free_size() > max)
        max = arena->get_page_free_size();
    return max;
  }

  inline size_t const get_memory_usage() {
    size_t sum = 0;
    for (Arena *arena = this; arena != nullptr; arena = arena->next)
      sum += arena->get_page_memory_usage();
    return sum;
  }

  inline size_t const get_free_size() {
    size_t sum = 0;
    for (Arena *arena = this; arena != nullptr; arena = arena->next)
      sum += arena->get_free_size();
    return sum;
  }

  inline bool const is_in_page_bounds(void *ptr) {
    return ptr >= memory && ptr < (memory + page_size);
  }

  inline bool const is_in_page_bounds(void *ptr, uint32_t size) {
    return ptr >= memory && (unsigned char *)ptr + size < (memory + page_size);
  }

  inline bool const is_in_bounds(void *ptr) {
    for (Arena *arena = this; arena != nullptr; arena = arena->next)
      if (arena->is_in_page_bounds(ptr))
        return true;
    return false;
  }

  inline bool const is_in_bounds(void *ptr, uint32_t size) {
    for (Arena *arena = this; arena != nullptr; arena = arena->next)
      if (arena->is_in_page_bounds(ptr, size))
        return true;
    return false;
  }
};

// allocator interface for thread-local arena
template <class T> struct ArenaAllocator {
  Arena *arena = nullptr;

  typedef T value_type;

  ArenaAllocator() = default;

  template <class U>
  constexpr ArenaAllocator(const ArenaAllocator<U> &other) noexcept {
    arena = std::move(other.arena);
  }

  [[nodiscard]] T *allocate(std::size_t n) {
    if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
      throw std::bad_array_new_length();

    void *ptr = arena->alloc(n * sizeof(T));
    if (ptr)
      return static_cast<T *>(ptr);

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

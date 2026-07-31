#include <Arena.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdatomic.h>
namespace Allocators {

Arena::Arena(uint32_t page_size, _Float16 growth_factor) {
  this->page_size = page_size;
  this->growth_factor = growth_factor;
  offset = 0;
  memory = (unsigned char *)calloc(1, 4096);
  next = nullptr;
}

Arena::~Arena() {
  if (memory)
    free(memory);
  if (next)
    delete next;
}

// Arena::Arena(Arena &&other) noexcept {
//   growth_factor = other.growth_factor;
//   page_size = other.page_size;
//   offset = other.offset;
//   memory = other.memory;
//   next = other.next;
//   larger_blobs = std::move(larger_blobs);
//   other.memory = nullptr;
//   other.next = nullptr;
// }
//
// Arena &Arena::operator=(Arena &&other) noexcept {
//   if (this != &other) {
//     growth_factor = other.growth_factor;
//     page_size = other.page_size;
//     offset = other.offset;
//     memory = other.memory;
//     next = other.next;
//     larger_blobs = std::move(larger_blobs);
//     other.memory = nullptr;
//     other.next = nullptr;
//   }
//   return *this;
// }

void *Arena::alloc(uint32_t size) {
  Arena *arena = this;
  uint_fast32_t offset_ = 0;
  uint_fast32_t old_offset = 0;
  do {
    if (atomic_load_explicit(&arena->is_locked, memory_order_acq_rel)) {
      atomic_wait_explicit(&arena->is_locked, true,
                           memory_order_acq_rel); // put thread to sleep until
                                                  // new page is allocated
    }
    // if (arena->get_page_free_size() <= size) {
    //   arena->offset += size;
    //   return memory + arena->offset - size;
    // }
    offset_ = atomic_load_explicit(&arena->offset, memory_order_acq_rel);
    old_offset = offset_;
    while (offset_ == old_offset + size) {
      if (offset_ + size >
          arena->page_size) // if there is not enough space, check other pages
        break;
      if (atomic_compare_exchange_weak(&arena->offset, &offset_, offset_ + size,
                                       memory_order_acq_rel)) {
        break;
      }
      old_offset = offset_;
    }
  } while (arena->next != nullptr);

  if (offset_ != old_offset + size) {
    // needs to allocate new page
  }
}

void Arena::attempt_free(void *ptr, uint32_t size) {
  for (Arena *arena = this; arena != nullptr; arena = arena->next) {
    if (ptr >= arena->memory &&
        (unsigned char *)ptr + size == (arena->memory + offset)) {
      memset(ptr, 0, size);
      arena->offset -= size;
    }
  }
}

} // namespace Allocators
